//===----------------------------------------------------------------------===//
// Poly constant folding patterns
//
// These patterns optimize polynomial operations when one operand is a constant.
// Canonicalization patterns ensure that if either operand is a constant, it
// is placed on the RHS, so we only check the RHS here.
//===----------------------------------------------------------------------===//

#include "lib/Dialect/Poly/PolyOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace tutorial {

// --- Pass definition ---
// GEN_PASS_DEF_OPTIMIZEPOLYCONST is defined by the generated Passes.h.inc;
// it expands to the OptimizePolyConstBase base class and createOptimizePolyConst().
#define GEN_PASS_DEF_OPTIMIZEPOLYCONST
#include "lib/Transform/Poly/Passes.h.inc"

using poly::ConstantOp;
using poly::MulOp;
using poly::AddOp;

//===----------------------------------------------------------------------===//
// MultiplyConstOne
//
// Replace  y = C * x   with   y = x   when every coefficient of C is 1.
// Since multiplying by the multiplicative identity (all-1 polynomial) is
// a no-op, we can simply forward the LHS operand.
//===----------------------------------------------------------------------===//
struct MultiplyConstOne : public OpRewritePattern<MulOp> {
  MultiplyConstOne(mlir::MLIRContext *context)
      : OpRewritePattern<MulOp>(context, /*benefit=*/2) {}

  LogicalResult matchAndRewrite(MulOp op,
                                PatternRewriter &rewriter) const override {
    // Operands of the multiplication (commutative — constant may be on either side).
    Value lhs = op.getOperand(0);
    Value rhs = op.getOperand(1);

    // Try to get a ConstantOp from either operand.
    // Type: mlir::tutorial::poly::ConstantOp (or null)
    auto constOp = lhs.getDefiningOp<ConstantOp>();
    // The non-constant operand is the one we will forward.
    Value nonConst = rhs;
    if (!constOp) {
      constOp = rhs.getDefiningOp<ConstantOp>();
      nonConst = lhs;
    }
    if (!constOp) {
      return failure();
    }

    // Extract the dense integer coefficients from the constant.
    // Type: llvm::iterator_range<DenseElementIterators<int64_t>>
    auto coeffs = constOp.getCoefficients().getValues<int64_t>();

    // Succeed only when every coefficient equals 1 (the multiplicative identity).
    bool allOne = llvm::all_of(coeffs, [](int64_t v) { return v == 1; });
    if (!allOne) {
      return failure();
    }

    // The multiplication is a no-op; replace it with the non-constant operand directly.
    rewriter.replaceOp(op, nonConst);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// AddConstZero
//
// Replace  y = C + x   with   y = x   when every coefficient of C is 0.
// Adding the additive identity (zero polynomial) is a no-op, so we simply
// forward the LHS operand.
//===----------------------------------------------------------------------===//
struct AddConstZero : public OpRewritePattern<AddOp> {
  AddConstZero(mlir::MLIRContext *context)
      : OpRewritePattern<AddOp>(context, /*benefit=*/2) {}

  LogicalResult matchAndRewrite(AddOp op,
                                PatternRewriter &rewriter) const override {
    // Operands of the addition (commutative — constant may be on either side).
    Value lhs = op.getOperand(0);
    Value rhs = op.getOperand(1);

    // Try to get a ConstantOp from either operand.
    // Type: mlir::tutorial::poly::ConstantOp (or null)
    auto constOp = lhs.getDefiningOp<ConstantOp>();
    // The non-constant operand is the one we will forward.
    Value nonConst = rhs;
    if (!constOp) {
      constOp = rhs.getDefiningOp<ConstantOp>();
      nonConst = lhs;
    }
    if (!constOp) {
      return failure();
    }

    // Extract the dense integer coefficients from the constant.
    // Type: llvm::iterator_range<DenseElementIterators<int64_t>>
    auto coeffs = constOp.getCoefficients().getValues<int64_t>();

    // Succeed only when every coefficient equals 0 (the additive identity).
    bool allZero = llvm::all_of(coeffs, [](int64_t v) { return v == 0; });
    if (!allZero) {
      return failure();
    }

    // The addition is a no-op; replace it with the non-constant operand directly.
    rewriter.replaceOp(op, nonConst);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// OptimizePolyConst pass
//
// Applies the constant-folding patterns defined above to every operation
// in the region.
//===----------------------------------------------------------------------===//
struct OptimizePolyConst
    : impl::OptimizePolyConstBase<OptimizePolyConst> {
  using OptimizePolyConstBase::OptimizePolyConstBase;

  void runOnOperation() {
    mlir::RewritePatternSet patterns(&getContext());
    patterns.add<MultiplyConstOne>(&getContext());
    patterns.add<AddConstZero>(&getContext());
    (void)applyPatternsGreedily(getOperation(), std::move(patterns));
  }
};

} // namespace tutorial
} // namespace mlir
