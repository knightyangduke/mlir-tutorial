//===----------------------------------------------------------------------===//
// OptimizePolyConst pass
//
// Applies poly constant folding patterns (multiply by 1, add 0) to every
// operation in the region. The patterns themselves live in the shared
// populatePolyReductionPatterns() function in the Poly dialect library,
// so they are also available to mlir-reduce via the
// DialectReductionPatternInterface.
//===----------------------------------------------------------------------===//

#include "lib/Dialect/Poly/PolyReductionPatterns.h"
#include "lib/Utility/DebugHelper.h"
#include "mlir/IR/PatternMatch.h"

#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace tutorial {

// --- Pass definition ---
#define GEN_PASS_DEF_OPTIMIZEPOLYCONST
#include "lib/Transform/Poly/Passes.h.inc"

struct OptimizePolyConst
    : impl::OptimizePolyConstBase<OptimizePolyConst> {
  using OptimizePolyConstBase::OptimizePolyConstBase;

  void runOnOperation() {
    llvm::errs() << "OptimizePolyConst pass running\n";
    llvm::errs().flush();
    mlir::RewritePatternSet patterns(&getContext());
    poly::populatePolyReductionPatterns(patterns);
    (void)applyPatternsGreedily(getOperation(), std::move(patterns));
  }
};

} // namespace tutorial
} // namespace mlir
