#include "lib/Transform/Arith/MulToShift.h"
#include "lib/Transform/Arith/ArithPatterns.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
namespace tutorial {

#define GEN_PASS_DEF_MULTOSHIFT
#include "lib/Transform/Arith/Passes.h.inc"

using arith::ConstantOp;
using arith::MulIOp;
using arith::ShLIOp;

// Replace y = C*x with y = x << log2(C), when C is a power of 2.
struct PowerOfTwoToShift : public OpRewritePattern<MulIOp> {
  PowerOfTwoToShift(mlir::MLIRContext *context)
      : OpRewritePattern<MulIOp>(context, /*benefit=*/2) {}

  LogicalResult matchAndRewrite(MulIOp op,
                                PatternRewriter &rewriter) const override {
    Value lhs = op.getOperand(0);

    // canonicalization patterns ensure the constant is on the right, if there
    // is a constant.
    Value rhs = op.getOperand(1);
    auto rhsDefiningOp = rhs.getDefiningOp<arith::ConstantIntOp>();
    if (!rhsDefiningOp) {
      return failure();
    }

    int64_t value = rhsDefiningOp.value();
    bool is_power_of_two = value > 0 && (value & (value - 1)) == 0;

    if (!is_power_of_two) {
      return failure();
    }

    // Compute log2(value) as the shift amount.
    int64_t shiftAmount = 0;
    while ((1LL << shiftAmount) < value) {
      ++shiftAmount;
    }

    ConstantOp newConstant = rewriter.create<ConstantOp>(
        rhsDefiningOp.getLoc(),
        rewriter.getIntegerAttr(rhs.getType(), shiftAmount));
    ShLIOp newShift = rewriter.create<ShLIOp>(op.getLoc(), lhs, newConstant);

    rewriter.replaceOp(op, newShift);
    rewriter.eraseOp(rhsDefiningOp);

    return success();
  }
};

struct MulToShift : impl::MulToShiftBase<MulToShift> {
  using MulToShiftBase::MulToShiftBase;

  void runOnOperation() {
    mlir::RewritePatternSet patterns(&getContext());
    patterns.add<PeelFromMul>(&getContext());
    patterns.add<PowerOfTwoToShift>(&getContext());
    (void)applyPatternsAndFoldGreedily(getOperation(), std::move(patterns));
  }
};

} // namespace tutorial
} // namespace mlir
