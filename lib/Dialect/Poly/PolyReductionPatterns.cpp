//===----------------------------------------------------------------------===//
// Poly reduction patterns
//
// These patterns simplify polynomial operations to help mlir-reduce produce
// smaller, simpler IR while preserving interestingness.
//===----------------------------------------------------------------------===//

#include "lib/Dialect/Poly/PolyReductionPatterns.h"
#include "lib/Dialect/Poly/PolyOps.h"
#include "lib/Utility/PolyOptimizeAddAction.hpp"
#include "lib/Utility/PolyOptmizeMulAction.hpp"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/MLIRContext.h"

namespace mlir {
namespace tutorial {
namespace poly {

//===----------------------------------------------------------------------===//
// MultiplyConstOne
//
// Replace  y = C * x   with   y = x   when every coefficient of C is 1.
// Since multiplying by the multiplicative identity (all-1 polynomial) is
// a no-op, we can simply forward the non-constant operand.
//===----------------------------------------------------------------------===//
struct MultiplyConstOne : public OpRewritePattern<MulOp> {
  MultiplyConstOne(mlir::MLIRContext *context)
      : OpRewritePattern<MulOp>(context, /*benefit=*/2) {}

  LogicalResult matchAndRewrite(MulOp op,
                                PatternRewriter &rewriter) const override {
    Value lhs = op.getOperand(0);
    Value rhs = op.getOperand(1);

    // Try to get a ConstantOp from either operand.
    auto constOp = lhs.getDefiningOp<ConstantOp>();
    Value nonConst = rhs;
    if (!constOp) {
      constOp = rhs.getDefiningOp<ConstantOp>();
      nonConst = lhs;
    }
    if (!constOp) {
      return failure();
    }

    // Check that every coefficient equals 1.
    auto coeffs = constOp.getCoefficients().getValues<APInt>();
    bool allOne = llvm::all_of(coeffs, [](APInt v) { return v.isOne(); });
    if (!allOne) {
      return failure();
    }

    // Wrap the rewrite in a PolyOptmizeMul action so the handler can
    // intercept it when --my-debug-tag=poly-optimize-mul is passed.
    mlir::Operation *parentFunc = op->getParentOfType<mlir::func::FuncOp>();
    mlir::IRUnit irUnit = parentFunc ? parentFunc : static_cast<mlir::Operation *>(op);
    op->getContext()->executeAction<mlir::tracing::PolyOptmizeMul>(
        [&]() { rewriter.replaceOp(op, nonConst); }, {irUnit});
    return success();
  }
};

//===----------------------------------------------------------------------===//
// AddConstZero
//
// Replace  y = C + x   with   y = x   when every coefficient of C is 0.
// Adding the additive identity (zero polynomial) is a no-op.
//===----------------------------------------------------------------------===//
struct AddConstZero : public OpRewritePattern<AddOp> {
  AddConstZero(mlir::MLIRContext *context)
      : OpRewritePattern<AddOp>(context, /*benefit=*/2) {}

  LogicalResult matchAndRewrite(AddOp op,
                                PatternRewriter &rewriter) const override {
    Value lhs = op.getOperand(0);
    Value rhs = op.getOperand(1);

    // Try to get a ConstantOp from either operand.
    auto constOp = lhs.getDefiningOp<ConstantOp>();
    Value nonConst = rhs;
    if (!constOp) {
      constOp = rhs.getDefiningOp<ConstantOp>();
      nonConst = lhs;
    }
    if (!constOp) {
      return failure();
    }

    // Check that every coefficient equals 0.
    auto coeffs = constOp.getCoefficients().getValues<APInt>();
    bool allZero = llvm::all_of(coeffs, [](APInt v) { return v.isZero(); });
    if (!allZero) {
      return failure();
    }

    // Wrap the rewrite in a PolyOptmizeAdd action so the handler can
    // intercept it when --my-debug-tag=poly-optimize-add is passed.
    mlir::Operation *parentFunc = op->getParentOfType<mlir::func::FuncOp>();
    mlir::IRUnit irUnit = parentFunc ? parentFunc : static_cast<mlir::Operation *>(op);
    op->getContext()->executeAction<mlir::tracing::PolyOptmizeAdd>(
        [&]() { rewriter.replaceOp(op, nonConst); }, {irUnit});
    return success();
  }
};

//===----------------------------------------------------------------------===//
// populatePolyReductionPatterns
//===----------------------------------------------------------------------===//
void populatePolyReductionPatterns(RewritePatternSet &patterns) {
  patterns.add<MultiplyConstOne, AddConstZero>(patterns.getContext());
}

} // namespace poly
} // namespace tutorial
} // namespace mlir
