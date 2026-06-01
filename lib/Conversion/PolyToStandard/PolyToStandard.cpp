#include "lib/Conversion/PolyToStandard/PolyToStandard.h"

#include "lib/Dialect/Poly/PolyOps.h"
#include "lib/Dialect/Poly/PolyTypes.h"
#include "llvm/ADT/SmallVector.h"          // from @llvm-project
#include "mlir/Dialect/SCF/IR/SCF.h"                    // from @llvm-project
#include "mlir/Dialect/Func/IR/FuncOps.h"  // from @llvm-project
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"  // from @llvm-project
#include "mlir/Dialect/Tensor/IR/Tensor.h"  // from @llvm-project
#include "mlir/IR/ImplicitLocOpBuilder.h"  // from @llvm-project
#include "mlir/Transforms/DialectConversion.h"  // from @llvm-project

namespace mlir {
namespace tutorial {
namespace poly {

#define GEN_PASS_DEF_POLYTOSTANDARD
#include "lib/Conversion/PolyToStandard/PolyToStandard.h.inc"

class PolyToStandardTypeConverter : public TypeConverter {
 public:
  PolyToStandardTypeConverter(MLIRContext *ctx) {
    addConversion([](Type type) { return type; });
    addConversion([ctx](PolynomialType type) -> Type {
      int degreeBound = type.getDegreeBound();
      IntegerType elementTy =
          IntegerType::get(ctx, 32, IntegerType::SignednessSemantics::Signless);
      return RankedTensorType::get({degreeBound}, elementTy);
    });
    // Handle tensor<Kx!poly.poly<D>> → tensor<KxDxi32> by flattening.
    // This is needed because tensors-of-tensors are invalid in MLIR.
    //
    // Example: tensor<2x!poly.poly<10>> → tensor<2x10xi32>
    //   K=2 (number of elements), D=10 (degree bound) appended to shape.
    addConversion([ctx](RankedTensorType type) -> Type {
      auto polyType = dyn_cast<PolynomialType>(type.getElementType());
      if (!polyType) return type;
      int degreeBound = polyType.getDegreeBound();
      IntegerType elementTy =
          IntegerType::get(ctx, 32, IntegerType::SignednessSemantics::Signless);
      SmallVector<int64_t> shape = llvm::to_vector(type.getShape());
      shape.push_back(degreeBound);
      return RankedTensorType::get(shape, elementTy);
    });
        // Convert from a tensor type to a poly type: use from_tensor
    addSourceMaterialization([](OpBuilder &builder, Type type,
                                ValueRange inputs, Location loc) -> Value {
      return builder.create<poly::FromTensorOp>(loc, type, inputs[0]);
    });

    // Convert from a poly type to a tensor type: use to_tensor
    addTargetMaterialization([](OpBuilder &builder, Type type,
                                ValueRange inputs, Location loc) -> Value {
      return builder.create<poly::ToTensorOp>(loc, type, inputs[0]);
    });
  }
};

struct ConvertAdd : public OpConversionPattern<AddOp> {
  ConvertAdd(mlir::MLIRContext *context)
      : OpConversionPattern<AddOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      AddOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    arith::AddIOp addOp = rewriter.create<arith::AddIOp>(
        op.getLoc(), adaptor.getLhs(), adaptor.getRhs());
    rewriter.replaceOp(op.getOperation(), addOp);
    return success();
  }
};

struct ConvertSub : public OpConversionPattern<SubOp> {
  ConvertSub(mlir::MLIRContext *context)
      : OpConversionPattern<SubOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      SubOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    arith::SubIOp subOp = rewriter.create<arith::SubIOp>(
        op.getLoc(), adaptor.getLhs(), adaptor.getRhs());
    rewriter.replaceOp(op.getOperation(), subOp);
    return success();
  }
};

struct ConvertMul : public OpConversionPattern<MulOp> {
  ConvertMul(mlir::MLIRContext *context)
      : OpConversionPattern<MulOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  
  // Lowers %r = poly.mul %p0, %p1 : !poly.poly<N>
  // where %p0 and %p1 are already converted to tensor<Nxi32>, into:
  //
  //   // Initialize result tensor to all zeros.
  //   %cst = arith.constant dense<0> : tensor<Nxi32>
  //   %c0 = arith.constant 0 : index
  //   %cN = arith.constant N : index
  //   %c1 = arith.constant 1 : index
  //
  //   // Outer loop: for i = 0 .. N-1
  //   %outer = scf.for %i = %c0 to %cN step %c1 iter_args(%prod = %cst)
  //                                                       -> tensor<Nxi32> {
  //     // Inner loop: for j = 0 .. N-1
  //     %inner = scf.for %j = %c0 to %cN step %c1 iter_args(%accum = %prod)
  //                                                         -> tensor<Nxi32> {
  //       // dest = (i + j) mod N
  //       %sum = arith.addi %i, %j : index
  //       %dest = arith.remui %sum, %cN : index
  //
  //       // Mul = p0[i] * p1[j]
  //       %coef_i = tensor.extract %p0[%i] : tensor<Nxi32>
  //       %coef_j = tensor.extract %p1[%j] : tensor<Nxi32>
  //       %mul = arith.muli %coef_i, %coef_j : i32
  //
  //       // Add to accumulator at dest index.
  //       %old = tensor.extract %accum[%dest] : tensor<Nxi32>
  //       %sum2 = arith.addi %mul, %old : i32
  //       %new_accum = tensor.insert %sum2 into %accum[%dest] : tensor<Nxi32>
  //       scf.yield %new_accum : tensor<Nxi32>
  //     }
  //     scf.yield %inner : tensor<Nxi32>
  //   }
  //   %r = %outer : tensor<Nxi32>

  LogicalResult matchAndRewrite(
      MulOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto polymulTensorType = cast<RankedTensorType>(adaptor.getLhs().getType());
    auto numTerms = polymulTensorType.getShape()[0];
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    // --> %cst = arith.constant dense<0> : tensor<Nxi32>
    auto polymulResult = b.create<arith::ConstantOp>(
        polymulTensorType, DenseElementsAttr::get(polymulTensorType, 0));

    // --> %c0 = arith.constant 0 : index
    auto lowerBound =
        b.create<arith::ConstantOp>(b.getIndexType(), b.getIndexAttr(0));
    // --> %cN = arith.constant N : index
    auto numTermsOp =
        b.create<arith::ConstantOp>(b.getIndexType(), b.getIndexAttr(numTerms));
    // --> %c1 = arith.constant 1 : index
    auto step =
        b.create<arith::ConstantOp>(b.getIndexType(), b.getIndexAttr(1));

    auto p0 = adaptor.getLhs();
    auto p1 = adaptor.getRhs();

    // for i = 0, ..., N-1
    //   for j = 0, ..., N-1
    //     product[i+j (mod N)] += p0[i] * p1[j]

    // --> %outer = scf.for %i = %c0 to %cN step %c1 iter_args(%prod = %cst)
    //                                                       -> tensor<Nxi32>
    auto outerLoop = b.create<scf::ForOp>(
        lowerBound, numTermsOp, step, ValueRange(polymulResult.getResult()),
        [&](OpBuilder &builder, Location loc, Value p0Index,
            ValueRange loopState) {
          ImplicitLocOpBuilder b(op.getLoc(), builder);
          // -->   %inner = scf.for %j = %c0 to %cN step %c1
          //           iter_args(%accum = %prod) -> tensor<Nxi32>
          auto innerLoop = b.create<scf::ForOp>(
              lowerBound, numTermsOp, step, loopState,
              [&](OpBuilder &builder, Location loc, Value p1Index,
                  ValueRange loopState) {
                ImplicitLocOpBuilder b(op.getLoc(), builder);
                auto accumTensor = loopState.front();
                // -->   %sum = arith.addi %i, %j : index
                // -->   %dest = arith.remui %sum, %cN : index
                auto destIndex = b.create<arith::RemUIOp>(
                    b.create<arith::AddIOp>(p0Index, p1Index), numTermsOp);
                // -->   %coef_i = tensor.extract %p0[%i] : tensor<Nxi32>
                // -->   %coef_j = tensor.extract %p1[%j] : tensor<Nxi32>
                // -->   %mul = arith.muli %coef_i, %coef_j : i32
                auto mulOp = b.create<arith::MulIOp>(
                    b.create<tensor::ExtractOp>(p0, ValueRange(p0Index)),
                    b.create<tensor::ExtractOp>(p1, ValueRange(p1Index)));
                // -->   %old = tensor.extract %accum[%dest] : tensor<Nxi32>
                // -->   %sum2 = arith.addi %mul, %old : i32
                auto result = b.create<arith::AddIOp>(
                    mulOp, b.create<tensor::ExtractOp>(accumTensor,
                                                       destIndex.getResult()));
                // -->   %new_accum = tensor.insert %sum2 into %accum[%dest]
                //                                                       : tensor<Nxi32>
                auto stored = b.create<tensor::InsertOp>(result, accumTensor,
                                                         destIndex.getResult());
                // -->   scf.yield %new_accum : tensor<Nxi32>
                b.create<scf::YieldOp>(stored.getResult());
              });

          // -->   scf.yield %inner : tensor<Nxi32>
          b.create<scf::YieldOp>(innerLoop.getResults());
        });

    // The result is %outer.getResult(0), the final accumulated tensor.
    rewriter.replaceOp(op, outerLoop.getResult(0));
    return success();
  }
};

struct ConvertEval : public OpConversionPattern<EvalOp> {
  ConvertEval(mlir::MLIRContext *context)
      : OpConversionPattern<EvalOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      EvalOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto polyTensorType =
        cast<RankedTensorType>(adaptor.getPolynomial().getType());
    auto numTerms = polyTensorType.getShape()[0];
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    auto lowerBound =
        b.create<arith::ConstantOp>(b.getIndexType(), b.getIndexAttr(1));
    auto numTermsOp = b.create<arith::ConstantOp>(b.getIndexType(),
                                                  b.getIndexAttr(numTerms));
    auto upperBound = b.create<arith::ConstantOp>(b.getIndexType(),
                                                  b.getIndexAttr(numTerms + 1));
    auto step = lowerBound;

    auto poly = adaptor.getPolynomial();//polycoeff
    auto point = adaptor.getPoint();//point

    // Horner's method:
    // https://en.wikipedia.org/wiki/Horner%27s_method
    // accum = 0
    // for i = 1, 2, ..., N
    //   accum = point * accum + polycoeff[N - i]
    //
    // Lowers %r = poly.eval %poly, %point : (!poly.poly<N>, i32) -> i32
    // where %poly is already converted to tensor<Nxi32>, into:
    //
    //   // Loop bounds.
    //   %c1 = arith.constant 1 : index
    //   %cN = arith.constant N : index
    //   %cNp1 = arith.constant 1 : index
    //
    //   // Initialize accumulator to 0.
    //   %init = arith.constant 0 : i32
    //
    //   %result = scf.for %i = %c1 to %cNp1 step %c1 iter_args(%accum = %init) -> i32 {
    //     // coeff index = N - i
    //     %idx = arith.subi %cN, %i : index
    //     %coeff = tensor.extract %poly[%idx] : tensor<Nxi32>
    //
    //     // accum = point * accum + coeff
    //     %mul = arith.muli %point, %accum : i32
    //     %sum = arith.addi %mul, %coeff : i32
    //     scf.yield %sum : i32
    //   }
    //   %r = %result : i32

    auto accum =
        b.create<arith::ConstantOp>(b.getI32Type(), b.getI32IntegerAttr(0));
    auto loop = b.create<scf::ForOp>(
        lowerBound, upperBound, step, accum.getResult(),
        [&](OpBuilder &builder, Location loc, Value loopIndex,
            ValueRange loopState) {
          ImplicitLocOpBuilder b(op.getLoc(), builder);
          auto accum = loopState.front();
          auto coeffIndex = b.create<arith::SubIOp>(numTermsOp, loopIndex);
          auto mulOp = b.create<arith::MulIOp>(point, accum);
          auto result = b.create<arith::AddIOp>(
              mulOp, b.create<tensor::ExtractOp>(poly, coeffIndex.getResult()));
          b.create<scf::YieldOp>(result.getResult());
        });

    rewriter.replaceOp(op, loop.getResult(0));
    return success();
  }
};

struct ConvertFromTensor : public OpConversionPattern<FromTensorOp> {
  ConvertFromTensor(mlir::MLIRContext *context)
      : OpConversionPattern<FromTensorOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  // Lowers %r = poly.from_tensor %input : tensor<Mxi32> -> !poly.poly<N>
  // where M <= N (M is the number of provided coefficients, N is the degree
  // bound). The result type !poly.poly<N> converts to tensor<Nxi32>.
  //
  // If M == N (no padding needed):
  //   %r = %input : tensor<Nxi32>
  //
  // If M < N (zero-pad to degree N):
  //   %c0 = arith.constant 0 : i32
  //   %r = tensor.pad %input low = [0] high = [N-M] : tensor<Mxi32> to tensor<Nxi32>
  //     tensor.yield %c0 : i32
  //   }

  LogicalResult matchAndRewrite(
      FromTensorOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    auto resultTensorTy = cast<RankedTensorType>(
        typeConverter->convertType(op->getResultTypes()[0]));
    auto resultShape = resultTensorTy.getShape()[0];
    auto resultEltTy = resultTensorTy.getElementType();

    auto inputTensorTy = op.getInput().getType();
    auto inputShape = inputTensorTy.getShape()[0];

    // Zero pad the tensor if the coefficients' size is less than the polynomial
    // degree.
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);
    auto coeffValue = adaptor.getInput();
    if (inputShape < resultShape) {
      // --> %c0 = arith.constant 0 : i32
      // --> %padded = tensor.pad %input low = [0] high = [N-M]
      //         : tensor<Mxi32> to tensor<Nxi32>
      SmallVector<OpFoldResult, 1> low, high;
      low.push_back(rewriter.getIndexAttr(0));
      high.push_back(rewriter.getIndexAttr(resultShape - inputShape));
      coeffValue = b.create<tensor::PadOp>(
          resultTensorTy, coeffValue, low, high,
          b.create<arith::ConstantOp>(rewriter.getIntegerAttr(resultEltTy, 0)),
          /*nofold=*/false);
    }

    // When no padding: %r = %input (type already matches)
    // When padded:     %r = %padded : tensor<Nxi32>
    rewriter.replaceOp(op, coeffValue);
    return success();
  }
};

struct ConvertToTensor : public OpConversionPattern<ToTensorOp> {
  ConvertToTensor(mlir::MLIRContext *context)
      : OpConversionPattern<ToTensorOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      ToTensorOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOp(op, adaptor.getInput());
    return success();
  }
};

// Lowers %r = tensor.from_elements %a0, %a1 : tensor<Kx!poly.poly<D>>
// to a 2D tensor. Since !poly.poly<D> converts to tensor<Dxi32>, and
// tensors-of-tensors are invalid, we flatten to tensor<KxDxi32>:
//
//   %empty = tensor.empty() : tensor<KxDxi32>
//   %r = tensor.insert_slice %a0 into %empty[0, 0] [1, D] [1, 1]
//       : tensor<Dxi32> into tensor<KxDxi32>
//   %r = tensor.insert_slice %a1 into %r[1, 0] [1, D] [1, 1]
//       : tensor<Dxi32> into tensor<KxDxi32>
// Note: tensor.insert_slice %source into %dest[offset_d0, offset_d1]
//                                            [size_d0, size_d1]
//                                            [stride_d0, stride_d1]
struct ConvertFromElements
    : public OpConversionPattern<tensor::FromElementsOp> {
  ConvertFromElements(mlir::MLIRContext *context)
      : OpConversionPattern<tensor::FromElementsOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      tensor::FromElementsOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    // Only apply if the result type contains poly types (i.e., the type
    // converter actually changes it).
    auto resultTy =
        dyn_cast<RankedTensorType>(typeConverter->convertType(op.getType()));
    if (!resultTy || resultTy == op.getType())
      return failure();

    auto loc = op.getLoc();
    int64_t numElements = resultTy.getDimSize(0); // K
    int64_t innerDim = resultTy.getDimSize(1); // D

    // --> %empty = tensor.empty() : tensor<KxDxi32>
    auto emptyOp = rewriter.create<tensor::EmptyOp>(loc, resultTy, ValueRange{});
    Value result = emptyOp.getResult();

    // Insert each operand as a [1 x D] slice.
    for (int64_t i = 0; i < numElements; ++i) {
      SmallVector<OpFoldResult> offsets;
      offsets.push_back(rewriter.getIndexAttr(i));
      offsets.push_back(rewriter.getIndexAttr(0));
      SmallVector<OpFoldResult> sizes;
      sizes.push_back(rewriter.getIndexAttr(1));
      sizes.push_back(rewriter.getIndexAttr(innerDim));
      SmallVector<OpFoldResult> strides;
      strides.push_back(rewriter.getIndexAttr(1));
      strides.push_back(rewriter.getIndexAttr(1));

      // --> tensor.insert_slice %operand into %result[i, 0] [1, D] [1, 1]
      result = rewriter.create<tensor::InsertSliceOp>(
          loc, adaptor.getOperands()[i], result, offsets, sizes, strides);
    }

    rewriter.replaceOp(op, result);
    return success();
  }
};

struct ConvertConstant : public OpConversionPattern<ConstantOp> {
  ConvertConstant(mlir::MLIRContext *context)
      : OpConversionPattern<ConstantOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      ConstantOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);
    auto constOp = b.create<arith::ConstantOp>(adaptor.getCoefficients());
    auto fromTensorOp =
        b.create<FromTensorOp>(op.getResult().getType(), constOp);
    rewriter.replaceOp(op, fromTensorOp.getResult());
    return success();
  }
};

struct PolyToStandard : impl::PolyToStandardBase<PolyToStandard> {
  using PolyToStandardBase::PolyToStandardBase;

  void runOnOperation() override {
    MLIRContext *context = &getContext();
    auto *module = getOperation();

    PolyToStandardTypeConverter typeConverter(context);

    ConversionTarget target(*context);
    target.addLegalDialect<arith::ArithDialect>();
    target.addIllegalDialect<PolyDialect>();
    // tensor.from_elements with poly element types must be converted.
    target.addDynamicallyLegalOp<tensor::FromElementsOp>(
        [&](tensor::FromElementsOp op) {
          return typeConverter.isLegal(op.getType());
        });

    RewritePatternSet patterns(context);
    patterns.add<ConvertAdd, ConvertConstant, ConvertSub, ConvertEval,
                 ConvertMul, ConvertFromTensor, ConvertToTensor,
                 ConvertFromElements>(typeConverter, context);

    populateFunctionOpInterfaceTypeConversionPattern<func::FuncOp>(
        patterns, typeConverter);
    target.addDynamicallyLegalOp<func::FuncOp>([&](func::FuncOp op) {
      return typeConverter.isSignatureLegal(op.getFunctionType()) &&
             typeConverter.isLegal(&op.getBody());
    });

    populateReturnOpTypeConversionPattern(patterns, typeConverter);
    target.addDynamicallyLegalOp<func::ReturnOp>(
        [&](func::ReturnOp op) { return typeConverter.isLegal(op); });

    populateCallOpTypeConversionPattern(patterns, typeConverter);
    target.addDynamicallyLegalOp<func::CallOp>(
        [&](func::CallOp op) { return typeConverter.isLegal(op); });

    populateBranchOpInterfaceTypeConversionPattern(patterns, typeConverter);
    target.markUnknownOpDynamicallyLegal([&](Operation *op) {
      return isNotBranchOpInterfaceOrReturnLikeOp(op) ||
             isLegalForBranchOpInterfaceTypeConversionPattern(op,
                                                              typeConverter) ||
             isLegalForReturnOpTypeConversionPattern(op, typeConverter);
    });

    if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

}  // namespace poly
}  // namespace tutorial
}  // namespace mlir
