# Additional Notes on MLIR

## Pitfall: `getValues<int64_t>()` Returns Empty Range

When a `DenseIntElementsAttr` has element type `i32`, calling `getValues<int64_t>()` may return an **empty range** due to a type mismatch. Always use `getValues<APInt>()` instead:

```cpp
// ❌ Wrong — empty range for i32 attributes:
auto coeffs = constOp.getCoefficients().getValues<int64_t>();

// ✅ Correct — works for any integer bit width:
auto coeffs = constOp.getCoefficients().getValues<APInt>();
for (APInt v : coeffs) {
  int64_t val = v.getSExtValue();  // sign-extend to int64_t
}
```

## Note: `DenseElementsAttr` Iterators Are Reusable

The iterator range returned by `getValues<APInt>()` can be iterated **multiple times** safely — it wraps a pointer to the underlying data and is not consumed after one pass.

However, if you use the same range in both a debug loop and a subsequent `llvm::all_of`, the code is clearer with two separate range variables:

```cpp
auto coeffs = constOp.getCoefficients().getValues<APInt>();

// First iteration — debug print
for (APInt v : coeffs) llvm::errs() << v.getSExtValue() << " ";

// Second iteration — still works fine with the same range
// bool allOne = llvm::all_of(coeffs, [](APInt v) { return v.isOne(); });
```
