#ifndef LIB_TRANSFORM_ARITH_ARITHPATTERNS_H_
#define LIB_TRANSFORM_ARITH_ARITHPATTERNS_H_

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"

namespace mlir {
namespace tutorial {

// Replace y = 9*x with y = 8*x + x
struct PeelFromMul : public OpRewritePattern<arith::MulIOp> {
  PeelFromMul(mlir::MLIRContext *context)
      : OpRewritePattern<arith::MulIOp>(context, /*benefit=*/1) {}

  LogicalResult matchAndRewrite(arith::MulIOp op,
                                PatternRewriter &rewriter) const override;
};

}  // namespace tutorial
}  // namespace mlir

#endif  // LIB_TRANSFORM_ARITH_ARITHPATTERNS_H_
