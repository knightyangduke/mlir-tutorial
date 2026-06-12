//===- tutorial-reduce.cpp - Poly-aware mlir-reduce driver ----------------===//
//
// A custom mlir-reduce binary that loads all built-in dialects plus the poly
// dialect.  Because PolyDialect::initialize() registers the
// PolyReductionPatternInterface, mlir-reduce automatically discovers and
// uses the poly reduction patterns (MultiplyConstOne, AddConstZero) when
// simplifying IR during reduction.
//
// Usage:
//   tutorial-reduce input.mlir -reduction-tree='traversal-mode=0 test=script.sh'
//===----------------------------------------------------------------------===//

#include "lib/Dialect/Poly/PolyDialect.h"
#include "lib/Transform/Poly/Passes.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Tools/mlir-reduce/MlirReduceMain.h"

using namespace mlir;

int main(int argc, char **argv) {
  DialectRegistry registry;

  // Register all built-in MLIR dialects (func, arith, etc.).
  registerAllDialects(registry);

  // Register the poly dialect.  Its initialize() calls
  //   addInterface<PolyReductionPatternInterface>()
  // which is how mlir-reduce discovers the reduction patterns.
  registry.insert<tutorial::poly::PolyDialect>();

  // Register poly transform passes so --opt-reduction-pass can use them.
  mlir::tutorial::poly::registerPolyPasses();

  MLIRContext context(registry);
  return failed(mlirReduceMain(argc, argv, context));
}
