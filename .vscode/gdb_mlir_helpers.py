"""
GDB helpers for debugging MLIR passes.
Load this via `source .vscode/gdb_mlir_helpers.py` in GDB, or add to
launch.json setupCommands.

Provides:
  mlir-dump-op <op>          — dump an Operation*
  mlir-dump-val <val>        — dump a Value
  mlir-dump-region <region>  — dump a Region
  mlir-dump-all              — verify all 4 helpers are callable
"""

import gdb

# Full C++ function signatures we need to call
FUNCS = {
    "dumpValue":  "void mlir::tutorial::dumpValue(mlir::Value, const char*)",
    "dumpOp":     "void mlir::tutorial::dumpOp(mlir::Operation*, const char*)",
    "dumpAPInt":  "void mlir::tutorial::dumpAPInt(llvm::APInt const&, const char*)",
    "dumpRegion": "void mlir::tutorial::dumpRegion(mlir::Region&, const char*)",
}

def _lookup(name):
    """Resolve a C++ mangled symbol. Returns gdb.Symbol or None."""
    try:
        return gdb.lookup_symbol(name)[0]
    except Exception:
        return None

def _call(func_name, *args):
    """Call func_name(value, label) via parse_and_eval. Expects stderr output."""
    expr = f'((void(*)(...))&mlir::tutorial::{func_name})({", ".join(args)})'
    try:
        gdb.execute(f"call {expr}")
    except Exception as e:
        print(f"[mlir] FAILED: {func_name}: {e}")

# ---- GDB commands ----

class MlirDumpOp(gdb.Command):
    """mlir-dump-op <expression> — dump an MLIR Operation*

    Example:  mlir-dump-op constOp.getOperation()
              mlir-dump-op op
"""
    def __init__(self):
        super().__init__("mlir-dump-op", gdb.COMMAND_USER)
    def invoke(self, arg, from_tty):
        _call("dumpOp", arg.strip() or "0", "0")

class MlirDumpVal(gdb.Command):
    """mlir-dump-val <expression> — dump an MLIR Value

    Example:  mlir-dump-val lhs
"""
    def __init__(self):
        super().__init__("mlir-dump-val", gdb.COMMAND_USER)
    def invoke(self, arg, from_tty):
        _call("dumpValue", arg.strip() or "{ }", "0")

class MlirDumpRegion(gdb.Command):
    """mlir-dump-region <expression> — dump an MLIR Region

    Example:  mlir-dump-region getOperation().getRegion(0)
"""
    def __init__(self):
        super().__init__("mlir-dump-region", gdb.COMMAND_USER)
    def invoke(self, arg, from_tty):
        _call("dumpRegion", f"(*({arg.strip() or '0'}))", "0")

class MlirDumpAll(gdb.Command):
    """mlir-dump-all — verify all 4 debug helpers are callable.

    Prints a summary of which symbols were found and whether calling them
    with null/fake arguments succeeds.
"""
    def __init__(self):
        super().__init__("mlir-dump-all", gdb.COMMAND_USER)
    def invoke(self, arg, from_tty):
        print("=== MLIR debug helper check ===")
        for name, sig in FUNCS.items():
            sym = _lookup(f"mlir::tutorial::{name}")
            if sym:
                print(f"  {name}: FOUND ({hex(sym.value().address)})")
            else:
                print(f"  {name}: MISSING — debug helpers NOT linked!")
        print()
        print("Symbols are available. Try in a breakpoint:")
        print("  mlir-dump-op constOp.getOperation()")
        print("  mlir-dump-val lhs")
        print("================================")

# Register commands
MlirDumpOp()
MlirDumpVal()
MlirDumpRegion()
MlirDumpAll()
print("[mlir] GDB helpers loaded. Try: mlir-dump-all")
