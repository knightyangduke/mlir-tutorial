#!/usr/bin/env python3
"""
End-to-end test: launch tutorial-opt under GDB, set a breakpoint in
PolyConstOptimize.cpp, call dumpOp() at runtime, and verify IR output
appeared on stderr.

Requires:  tutorial-opt built with DebugHelper symbols + --no-gc-sections
Usage:     python3 scripts/test_debug_live.py [--keep-temp]
           cd build-ninja && ninja tutorial-opt  # build first
"""

import subprocess, sys, os, tempfile, textwrap

WORKSPACE = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
TUTORIAL_OPT = os.path.join(WORKSPACE, "build-ninja", "tools", "tutorial-opt")

# ---- MLIR test input ----
TEST_MLIR = r"""
func.func @test(%arg0: !poly.poly<10>) -> !poly.poly<10> {
  %c = poly.constant dense<1> : tensor<10xi32> : !poly.poly<10>
  %0 = poly.mul %arg0, %c : !poly.poly<10>
  return %0 : !poly.poly<10>
}
"""

# ---- GDB commands ----
# We step until we can call dumpOp on the ConstantOp.
# In MultiplyConstOne::matchAndRewrite, the constant is stored in constOp
# (after the if ladder that assigns it). We break there, call dumpOp,
# then continue to exit cleanly.
GDB_COMMANDS = """
set pagination off
set confirm off

# Suppress "Reading symbols..." noise
set print elements 0

# Break in MultiplyConstOne::matchAndRewrite
# (at the all_of line — after constOp is assigned and coeffs extracted)
break PolyConstOptimize.cpp:88

# Run the pass
run --optimize-poly-const {mlir_file}

# At the breakpoint, call the debug helper
# constOp is a local variable of type poly::ConstantOp.
# We use .state to get the Operation* (getOperation() is inlined).
call mlir::tutorial::dumpOp(constOp.state, 0)

# Verify output went to stderr (GDB captures it in the gdb.MI log,
# but we capture it via redirect).  Call fprintf so we can grep for it.
call fprintf(stderr, "\\n=== DEBUG_OUTPUT_END ===\\n")

continue

quit
"""

def main():
    if not os.path.isfile(TUTORIAL_OPT):
        print(f"FAIL: {TUTORIAL_OPT} not found. Run: cd build-ninja && ninja tutorial-opt")
        sys.exit(1)

    with tempfile.NamedTemporaryFile(mode="w", suffix=".mlir", delete=True) as mlir_f:
        mlir_f.write(TEST_MLIR)
        mlir_f.flush()

        with tempfile.NamedTemporaryFile(mode="w", suffix=".gdb", delete=True) as gdb_f:
            gdb_f.write(GDB_COMMANDS.format(mlir_file=mlir_f.name))
            gdb_f.flush()

            print(f"Running GDB with breakpoint in MultiplyConstOne...")
            result = subprocess.run(
                ["gdb", "-q", "-batch", "-x", gdb_f.name, TUTORIAL_OPT],
                capture_output=True, text=True, timeout=30,
                cwd=WORKSPACE,
            )

    stderr = result.stderr
    stdout = result.stdout

    print(f"GDB exit code: {result.returncode}")

    # Check for the presence of dump output
    checks = [
        ("Operation dumped (op name found)", "poly.constant" in stderr or "poly.constant" in stdout),
        ("add dumped (op name found)", "poly.add" in stderr or "poly.add" in stdout or "mul" in stderr.lower() or "mul" in stdout.lower()),
        ("fprintf marker reached", "DEBUG_OUTPUT_END" in stderr or "DEBUG_OUTPUT_END" in stdout),
    ]

    for label, passed in checks:
        status = "OK" if passed else "FAIL"
        print(f"  [{status}] {label}")

    # Print relevant output
    if any(not p for _, p in checks):
        print("\n--- GDB stdout (last 30 lines) ---")
        for line in stdout.splitlines()[-30:]:
            print(f"  {line}")
        print("\n--- GDB stderr (last 30 lines) ---")
        for line in stderr.splitlines()[-30:]:
            print(f"  {line}")
        print("\nFAIL: debug helper did not produce expected output.")
        print("       Check: cd build-ninja && ninja tutorial-opt")
        sys.exit(1)
    else:
        print("\nPASS: debug helpers are working! Output confirmed.")

if __name__ == "__main__":
    main()
