#!/usr/bin/env python3
"""
Verify that MLIR debug helper symbols are present in the tutorial-opt binary.

Usage:
  python3 scripts/verify_debug.py [path/to/tutorial-opt]
"""

import subprocess, sys, os

BINARY = sys.argv[1] if len(sys.argv) > 1 else "build-ninja/tools/tutorial-opt"

SYMBOLS = {
    "dumpValue":  {"mangled": "_ZN4mlir8tutorial9dumpValueENS_5ValueEPKc", "desc": "Value"},
    "dumpOp":     {"mangled": "_ZN4mlir8tutorial6dumpOpEPNS_9OperationEPKc", "desc": "Operation*"},
    "dumpAPInt":  {"mangled": "_ZN4mlir8tutorial9dumpAPIntERKN4llvm5APIntEPKc", "desc": "APInt"},
    "dumpRegion": {"mangled": "_ZN4mlir8tutorial10dumpRegionERNS_6RegionEPKc", "desc": "Region"},
}

def check_binary(path):
    """Check if the binary exists and has symbols."""
    if not os.path.isfile(path):
        print(f"FAIL: {path} not found. Did you build?")
        return False
    out = subprocess.check_output(["nm", path], text=True, stderr=subprocess.DEVNULL)
    ok = True
    for name, info in SYMBOLS.items():
        if info["mangled"] in out:
            print(f"  OK   {name} ({info['desc']})")
        else:
            print(f"  MISS {name} ({info['desc']}) — not linked!")
            ok = False
    return ok

def check_no_gc_sections():
    """Check that --no-gc-sections is in the cmake config."""
    cmake = "build-ninja/CMakeCache.txt"
    if not os.path.isfile(cmake):
        return
    with open(cmake) as f:
        for line in f:
            if "CMAKE_EXE_LINKER_FLAGS" in line and "no-gc-sections" in line:
                print(f"  OK   --no-gc-sections enabled in CMake")
                return
    print(f"  WARN --no-gc-sections not found in CMakeCache")

print(f"=== MLIR debug helper verification ===")
print(f"Binary: {BINARY}\n")

ok = check_binary(BINARY)
print()
check_no_gc_sections()
print()

if ok:
    print("All 4 debug helpers are present.")
    print("In GDB (at a breakpoint):")
    print("  mlir-dump-all                          — verify symbols")
    print("  mlir-dump-op constOp.getOperation()    — dump an op")
    print("  mlir-dump-val lhs                      — dump a value")
else:
    print("FAIL: Some symbols missing. Rebuild with:")
    print("  cd build-ninja && ninja tutorial-opt")

sys.exit(0 if ok else 1)
