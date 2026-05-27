#include "lib/Dialect/Poly/PolyOps.h"

#include "mlir/Dialect/CommonFolders.h"
#include "mlir/Dialect/Complex/IR/Complex.h"
#include "mlir/IR/PatternMatch.h"

// Required after PatternMatch.h
#include "lib/Dialect/Poly/PolyCanonicalize.cpp.inc"

namespace mlir {
namespace tutorial {
namespace poly {


// OpFoldResult is a utility type in MLIR that represents the result of constant folding.
// It can hold an Attribute (the folded constant), a Value, or be null to indicate folding failed.
// This allows the canonicalization infrastructure to propagate constants through the IR.
//
// The FoldAdaptor provides a type-safe way to access the operands and attributes of the operation.
// It bundles together the operation's inputs, making folding code easier to write and maintain.
OpFoldResult ConstantOp::fold(ConstantOp::FoldAdaptor adaptor) {
  return adaptor.getCoefficients();
}


OpFoldResult AddOp::fold(AddOp::FoldAdaptor adaptor) {
  return constFoldBinaryOp<IntegerAttr, APInt, void>(
      adaptor.getOperands(), [&](APInt a, APInt b) { return a + b; });
}


OpFoldResult SubOp::fold(SubOp::FoldAdaptor adaptor) {
  return constFoldBinaryOp<IntegerAttr, APInt, void>(
      adaptor.getOperands(), [&](APInt a, APInt b) { return a - b; });
}

OpFoldResult MulOp::fold(MulOp::FoldAdaptor adaptor) {
  auto lhs = dyn_cast_or_null<DenseIntElementsAttr>(adaptor.getOperands()[0]);
  auto rhs = dyn_cast_or_null<DenseIntElementsAttr>(adaptor.getOperands()[1]);

  if (!lhs || !rhs) return nullptr;

  auto degree = llvm::cast<PolynomialType>(getResult().getType()).getDegreeBound();
  auto maxIndex = lhs.size() + rhs.size() - 1;

  SmallVector<APInt, 8> result;
  result.reserve(maxIndex);
  for (int i = 0; i < maxIndex; ++i) {
    result.push_back(APInt((*lhs.begin()).getBitWidth(), 0));
  }

  // Schoolbook polynomial multiplication: for each pair of coefficients
  // lhs[i] and rhs[j], their product contributes to the coefficient of x^(i+j)
  // in the result.  The index is taken modulo `degree` because the polynomial
  // ring is defined as Z[x] / (x^N - 1), so x^N wraps back to x^0.
  int i = 0;
  for (auto lhsIt = lhs.value_begin<APInt>(); lhsIt != lhs.value_end<APInt>();
       ++lhsIt) {
    int j = 0;
    for (auto rhsIt = rhs.value_begin<APInt>(); rhsIt != rhs.value_end<APInt>();
         ++rhsIt) {
      // Accumulate lhs[i] * rhs[j] into result[(i+j) mod N].
      result[(i + j) % degree] += *rhsIt * (*lhsIt);
      ++j;
    }
    ++i;
  }

  return DenseIntElementsAttr::get(
      RankedTensorType::get(static_cast<int64_t>(result.size()),
                            IntegerType::get(getContext(), 32)),
      result);
}

OpFoldResult FromTensorOp::fold(FromTensorOp::FoldAdaptor adaptor) {
  // Returns null if the cast failed, which corresponds to a failed fold.
  return dyn_cast_or_null<DenseIntElementsAttr>(adaptor.getInput());
}

LogicalResult EvalOp::verify() {
  auto pointTy = getPoint().getType();
  bool isSignlessInteger = pointTy.isSignlessInteger(32);
  auto complexPt = dyn_cast<ComplexType>(pointTy);
  return isSignlessInteger || complexPt ? success()
                                        : emitOpError(
                                              "argument point must be a 32-bit "
                                              "integer, or a complex number");
}

void AddOp::getCanonicalizationPatterns(::mlir::RewritePatternSet &results,
                                        ::mlir::MLIRContext *context) {}

void SubOp::getCanonicalizationPatterns(::mlir::RewritePatternSet &results,
                                        ::mlir::MLIRContext *context) {
  results.add<DifferenceOfSquares>(context);
}

void MulOp::getCanonicalizationPatterns(::mlir::RewritePatternSet &results,
                                        ::mlir::MLIRContext *context) {}

void EvalOp::getCanonicalizationPatterns(::mlir::RewritePatternSet &results,
                                         ::mlir::MLIRContext *context) {
  results.add<LiftConjThroughEval>(context);
}

}  // namespace poly
}  // namespace tutorial
}  // namespace mlir
