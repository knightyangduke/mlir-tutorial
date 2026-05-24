#include "lib/Transform/Arith/ArithPatterns.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"

namespace mlir {
namespace tutorial {

LogicalResult PeelFromMul::matchAndRewrite(arith::MulIOp op,
                                           PatternRewriter &rewriter) const {
  Value lhs = op.getOperand(0);
  Value rhs = op.getOperand(1);
  auto rhsDefiningOp = rhs.getDefiningOp<arith::ConstantIntOp>();
  if (!rhsDefiningOp) {
    return failure();
  }

  int64_t value = rhsDefiningOp.value();

  arith::ConstantOp newConstant = rewriter.create<arith::ConstantOp>(
      rhsDefiningOp.getLoc(),
      rewriter.getIntegerAttr(rhs.getType(), value - 1));
  arith::MulIOp newMul =
      rewriter.create<arith::MulIOp>(op.getLoc(), lhs, newConstant);
  arith::AddIOp newAdd =
      rewriter.create<arith::AddIOp>(op.getLoc(), newMul, lhs);

  rewriter.replaceOp(op, newAdd);
  rewriter.eraseOp(rhsDefiningOp);

  return success();
}

}  // namespace tutorial
}  // namespace mlir
