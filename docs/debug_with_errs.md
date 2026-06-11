# Debugging MLIR Passes with `llvm::errs()`

## Why `llvm::errs()`?

`llvm::errs()` is the LLVM/MLIR equivalent of `std::cerr` — it writes to **stderr**. Unlike `std::cout`, it's **unbuffered** by default, so output appears immediately without needing `flush()` (though calling `flush()` doesn't hurt).

## Common Pitfall: Output Doesn't Show Up When Piping

```bash
echo '...' | ./tutorial-opt --pass-name 2>&1   # ❌ stderr goes into the pipe, not your terminal
```

When you pipe input into `tutorial-opt`, the shell sets up a pipe between `echo`'s stdout and `tutorial-opt`'s stdin. The `2>&1` redirect **does not** route stderr to your terminal in this case — it redirects stderr to wherever fd 1 (stdout) is going, which is the pipe. The debug output is silently lost.

**Fix:** write the input to a file first, then run without piping stdin:

```bash
echo '...' > /tmp/test.mlir
./tutorial-opt /tmp/test.mlir --pass-name 2>&1     # stderr → terminal
./tutorial-opt /tmp/test.mlir --pass-name 2>/tmp/debug.txt  # stderr → file
```

For the full set of stdout/stderr redirection patterns (saving IR vs. debug output separately, `tee`, etc.), see [shell_io_redirect.md](shell_io_redirect.md).

## Putting Debug Prints in Patterns

```cpp
LogicalResult matchAndRewrite(MyOp op,
                              PatternRewriter &rewriter) const override {
  llvm::errs() << "MyPattern: attempting\n";

  // Print SSA values
  Value lhs = op.getOperand(0);
  llvm::errs() << "  LHS: " << lhs << "\n";

  // Print integers
  llvm::errs() << "  value = " << someInt << "\n";

  // Print APInt coefficients
  auto coeffs = constOp.getCoefficients().getValues<APInt>();
  llvm::errs() << "Coefficients: ";
  for (APInt v : coeffs) llvm::errs() << v.getSExtValue() << " ";
  llvm::errs() << "\n";

  // Not strictly needed (errs() is unbuffered), but good practice:
  llvm::errs().flush();
}
```

## Quick Reference

| Goal | Command |
|------|---------|
| See both stdout and stderr in terminal | `./tutorial-opt input.mlir --pass` |
| Save stderr to file | `./tutorial-opt input.mlir --pass 2>/tmp/debug.log` |
| See only debug (suppress IR output) | `./tutorial-opt input.mlir --pass 1>/dev/null` |
| See only IR (suppress debug) | `./tutorial-opt input.mlir --pass 2>/dev/null` |
| Separate both | `./tutorial-opt input.mlir --pass 1>ir.mlir 2>debug.log` |

## Debugging with VSCode/GDB

Since LLVM in this repo is **not built in debug mode**, the default `op.dump()` / pretty-printers may not work reliably from GDB. Use the helpers in [lib/Utility/DebugHelper.h](../lib/Utility/DebugHelper.h) instead, which are compiled directly into `tutorial-opt` and stay available as call targets.

From the GDB console (or VSCode "Debug Console" while paused at a breakpoint):

```
-exec call mlir::tutorial::dumpOp(op.state, 0)
```

Available helpers:

| Function | Use for |
|---|---|
| `dumpValue(val, label = nullptr)` | `mlir::Value` |
| `dumpOp(op, label = nullptr)` | `mlir::Operation*` |
| `dumpAPInt(val, label = nullptr)` | `llvm::APInt` |
| `dumpRegion(region, label = nullptr)` | `mlir::Region&` |

**Note:** for typed ops (e.g., `ConstantOp`), pass `.state` to get the underlying `Operation*`:

```
-exec call mlir::tutorial::dumpOp(constOp.state, 0)
```

Calling `dumpOp(op.getOperation())` directly will **not** work — `getOperation()` is inlined and unavailable as a call target in GDB.
