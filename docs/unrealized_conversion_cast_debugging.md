# Debugging `unrealized_conversion_cast` in Dialect Conversion

## Problem

When running a dialect conversion pass (e.g., `--poly-to-standard`), you may see:

```
error: failed to legalize unresolved materialization from ('tensor<10xi32>') to ('!poly.poly<10>') that remained live after conversion
  %0 = "builtin.unrealized_conversion_cast"(%arg1) : (tensor<10xi32>) -> !poly.poly<10>
```

## Why this happens

`builtin.unrealized_conversion_cast` is **valid MLIR IR** but it represents a **temporary/incomplete conversion**. The dialect conversion framework (`applyPartialConversion` / `applyFullConversion`) enforces a strict contract:

> **No `unrealized_conversion_cast` ops may remain live when the pass completes.**

If they survive, the pass fails — the conversion was incomplete.

### The mechanism

1. The **type converter** maps old types → new types (e.g., `!poly.poly<10>` → `tensor<10xi32>`).
2. When ops consuming the old types are not converted but their operands **are** converted, the framework inserts `unrealized_conversion_cast` to patch the type mismatch.
3. If the consuming op is never legalized/converted, the cast remains → pass failure.

### The root cause: `ConversionTarget` legalization

Whether a dialect conversion pass errors on leftover casts depends on its **`ConversionTarget`**:

- The dialect conversion framework checks every op against the target's legality rules.
- If `UnrealizedConversionCastOp` is **not** explicitly marked legal, the framework sees it as an illegal op → **error**.
- If the target explicitly calls `addLegalOp<UnrealizedConversionCastOp>()`, the cast is considered legal and survives without error.

**Example — `LLVMConversionTarget`** (used by `convert-func-to-llvm` et al.):

```cpp
LLVMConversionTarget::LLVMConversionTarget(MLIRContext &ctx)
    : ConversionTarget(ctx) {
  this->addLegalDialect<LLVM::LLVMDialect>();
  this->addLegalOp<UnrealizedConversionCastOp>();  // ← casts are LEGAL
}
```

This is intentional: LLVM-lowering passes are part of a **multi-pass pipeline** where casts persist across passes and are cleaned up at the end by `reconcile-unrealized-casts`. In contrast, a self-contained dialect conversion like `--poly-to-standard` uses a target that does **not** legalize the cast, so any leftover triggers a framework error.

### When is it OK to leave unrealized_conversion_cast?

| Scenario | OK? | Why |
|---|---|---|
| Cast inserted mid-pass, resolved by end | ✅ | Normal operation — cast is transient |
| Cast survives end of `applyFullConversion` | ❌ | Target requires *all* ops legalized; `UnrealizedConversionCastOp` not marked legal |
| Cast survives end of `applyPartialConversion` | ❌ | Same check — unless target explicitly calls `addLegalOp<UnrealizedConversionCastOp>()` |
| Pass target explicitly legalizes the cast | ✅ | e.g., `LLVMConversionTarget` — designed for multi-pass lowering |
| Multi-pass lowering with `reconcile-unrealized-casts` | ✅ | Casts persist across passes, then removed by dedicated cleanup pass |

## Common root causes

| Cause | Pattern |
|---|---|
| Missing conversion pattern for a consumer op | An op uses the old dialect's types but has no `OpConversionPattern` |
| Type system impossibility | e.g., `tensor<2x!poly.poly<10>>` → `tensor<2x tensor<10xi32>>` is invalid (MLIR forbids tensors of tensors) |
| Wrong type converter configuration | Type converter doesn't handle all necessary types |
| Missing materialization hooks | If using multi-pass lowering, you need `addSourceMaterialization` / `addTargetMaterialization` |

## Debugging commands

```bash
# Dump IR at the point of failure (best option)
tutorial-opt --poly-to-standard --mlir-print-ir-after-failure input.mlir

# Dump IR before and after every pass
tutorial-opt --poly-to-standard --mlir-print-ir-before-all --mlir-print-ir-after-all input.mlir

# Dump IR and save to a file
tutorial-opt --poly-to-standard --mlir-print-ir-after-failure input.mlir 2>&1 | tee dump.mlir
```

The `--mlir-print-ir-after-failure` flag was designed specifically for this — it shows the full state of the IR at the exact moment the pass failed, including all live `unrealized_conversion_cast` ops and their users.
