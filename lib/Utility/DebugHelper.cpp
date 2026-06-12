#include "lib/Utility/DebugHelper.h"
#include "mlir/IR/Block.h"

namespace mlir {
namespace tutorial {

__attribute__((used, noinline)) void dumpValue(mlir::Value val, const char *label) {
  if (label) llvm::errs() << label << ": ";
  val.dump();
  llvm::errs().flush();
}

__attribute__((used, noinline)) void dumpOp(mlir::Operation *op, const char *label) {
  if (label) llvm::errs() << label << ":\n";
  op->dump();
  llvm::errs().flush();
}

__attribute__((used, noinline)) void dumpAPInt(const llvm::APInt &val, const char *label) {
  if (label) llvm::errs() << label << ": ";
  llvm::errs() << val.getSExtValue() << "\n";
  llvm::errs().flush();
}

__attribute__((used, noinline)) void dumpRegion(mlir::Region &region, const char *label) {
  if (label) llvm::errs() << label << ":\n";
  for (auto &block : region) {
    llvm::errs() << "block:\n";
    for (auto &op : block)
      dumpOp(&op);
  }
  llvm::errs().flush();
}

} // namespace tutorial
} // namespace mlir
