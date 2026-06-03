#include "lib/Transform/Noisy/ReduceNoiseChecker.h"

#include "lib/Dialect/Noisy/NoisyDialect.h"
#include "lib/Dialect/Noisy/NoisyOps.h"
#include "mlir/Analysis/DataFlow/DeadCodeAnalysis.h"
#include "mlir/Analysis/DataFlow/IntegerRangeAnalysis.h"
#include "mlir/Analysis/DataFlowFramework.h"
#include "mlir/IR/Visitors.h"
#include "mlir/Pass/Pass.h"
#include "llvm/Support/raw_ostream.h"

namespace mlir {
namespace tutorial {
namespace noisy {

#define GEN_PASS_DEF_REDUCENOISECHECKER
#include "lib/Transform/Noisy/Passes.h.inc"

struct ReduceNoiseChecker
    : impl::ReduceNoiseCheckerBase<ReduceNoiseChecker> {
  using ReduceNoiseCheckerBase::ReduceNoiseCheckerBase;

  void runOnOperation() {
    Operation *module = getOperation();

    // Run the integer range analysis to compute noise bounds for each op.
    DataFlowSolver solver;
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::IntegerRangeAnalysis>();
    if (failed(solver.initializeAndRun(module))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    WalkResult result = module->walk([&](Operation *op) {
      if (!llvm::isa<noisy::AddOp, noisy::SubOp, noisy::MulOp,
                     noisy::ReduceNoiseOp>(*op)) {
        return WalkResult::advance();
      }

      const dataflow::IntegerValueRangeLattice *opRange =
          solver.lookupState<dataflow::IntegerValueRangeLattice>(
              op->getResult(0));
      if (!opRange || opRange->getValue().isUninitialized()) {
        op->emitOpError()
            << "Found op without a set integer range; did the analysis fail?";
        return WalkResult::interrupt();
      }

      ConstantIntRanges range = opRange->getValue().getValue();
      uint64_t umin = range.umin().getZExtValue();
      uint64_t umax = range.umax().getZExtValue();
      llvm::errs() << "[ReduceNoiseChecker] " << op->getName()
                   << " noise range: [" << umin << ", " << umax << "]"
                   << " (MAX_NOISE=" << MAX_NOISE << ")\n";
      if (umax > MAX_NOISE) {
        op->emitOpError()
            << "Noise value " << umax
            << " exceeds the maximum allowable noise of " << MAX_NOISE
            << ".\n";
        return WalkResult::interrupt();
      }

      llvm::errs() << "[ReduceNoiseChecker] " << op->getName()
                   << " OK (umax=" << umax << " <= " << MAX_NOISE << ")\n";
      return WalkResult::advance();
    });

    if (result.wasInterrupted()) {
      signalPassFailure();
    }
  }
};

} // namespace noisy
} // namespace tutorial
} // namespace mlir
