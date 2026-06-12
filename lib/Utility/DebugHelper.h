#ifndef LIB_UTILITY_DEBUGHELPER_H_
#define LIB_UTILITY_DEBUGHELPER_H_

#include "llvm/ADT/APInt.h"
#include "mlir/IR/Value.h"
#include "mlir/IR/Operation.h"

namespace mlir {
class Region; // forward decl — Region.h included in DebugHelper.cpp only

// Debug helpers for interactive use from GDB/LLDB.
//
// Implementations are in DebugHelper.cpp, compiled into the tutorial-opt
// executable directly (see tools/CMakeLists.txt). This keeps symbols
// available in the binary regardless of which library code is being debugged.
//
// Usage from GDB:
//   (gdb) call mlir::tutorial::dumpValue(val)
//   (gdb) call mlir::tutorial::dumpOp(op)
//   (gdb) call mlir::tutorial::dumpRegion(region)
//
// For typed MLIR ops like ConstantOp, use .state to get the Operation*:
//   (gdb) call mlir::tutorial::dumpOp(constOp.state, 0)

namespace tutorial {

void dumpValue(mlir::Value val, const char *label = nullptr);
void dumpOp(mlir::Operation *op, const char *label = nullptr);
void dumpAPInt(const llvm::APInt &val, const char *label = nullptr);
void dumpRegion(mlir::Region &region, const char *label = nullptr);

} // namespace tutorial
} // namespace mlir

#endif // LIB_UTILITY_DEBUGHELPER_H_
