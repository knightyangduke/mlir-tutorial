# How `scf.for` is Created in MLIR

When writing dialect conversion patterns, you often create `scf.for` loops using a builder with a lambda (body builder). This document explains what happens under the hood.

## The Builder Call

```cpp
b.create<scf::ForOp>(
    lowerBound,      // %c0
    numTermsOp,      // %cN
    step,            // %c1
    ValueRange(polymulResult.getResult()),  // iter_args init values
    [&](OpBuilder &builder, Location loc, Value p0Index,
        ValueRange loopState) {
      // Fill the loop body here
    });
```

## What Happens Inside `ForOp::build()`

Source: `mlir/lib/Dialect/SCF/IR/SCF.cpp` line 316

```cpp
void ForOp::build(OpBuilder &builder, OperationState &result, Value lb,
                  Value ub, Value step, ValueRange initArgs,
                  BodyBuilderFn bodyBuilder) {
  OpBuilder::InsertionGuard guard(builder);

  // 1. Record operands: lb, ub, step, and init args
  result.addOperands({lb, ub, step});
  result.addOperands(initArgs);

  // 2. Each init arg produces a result of the same type
  for (Value v : initArgs)
    result.addTypes(v.getType());

  // 3. Create a region with an entry block
  Region *bodyRegion = result.addRegion();
  Block *bodyBlock = builder.createBlock(bodyRegion);

  // 4. Add block arguments:
  //    - First arg: the induction variable (%i), type = index (lb type)
  bodyBlock->addArgument(t, result.location);
  //    - Remaining args: one per init arg, for loop-carried values (%prod)
  for (Value v : initArgs)
    bodyBlock->addArgument(v.getType(), v.getLoc());

  // 5. Call the lambda (body builder), passing block args
  if (bodyBuilder) {
    OpBuilder::InsertionGuard guard(builder);
    builder.setInsertionPointToStart(bodyBlock);
    bodyBuilder(builder, result.location,
                bodyBlock->getArgument(0),       // → p0Index (%i)
                bodyBlock->getArguments().drop_front());  // → loopState (%prod)
  }
}
```

## The Generated IR

```
%outer = scf.for %i = %c0 to %cN step %c1 iter_args(%prod = %cst) -> tensor<Nxi32> {
  ...
  scf.yield %new_val : tensor<Nxi32>
}
```

## Key Insight

| Concept | What it maps to |
|---|---|
| `p0Index` | `%i` — the induction variable, a **block argument** of the `scf.for`'s entry block |
| `loopState` | `%prod` (and any additional iter_args) — also **block arguments** of the entry block |
| Lambda | The **body builder** — it fills the block's body with operations |

The `scf::ForOp::build()` method:
1. Creates the `scf.for` operation in the IR
2. Creates the entry block inside its region
3. **Defines** the block arguments (`%i`, `%prod`) — these are the "variables"
4. Calls your lambda, **passing** those block arguments so you can use them

The variables are **not** defined by any preceding op — they are block arguments, analogous to function parameters. The `scf.for` operation itself is their defining owner. That's why you only see use sites in the conversion pattern, never a def site.
