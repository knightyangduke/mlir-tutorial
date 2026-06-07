static ::llvm::LogicalResult IsPowerOfTwoPDLFn(::mlir::PatternRewriter &rewriter, ::mlir::Attribute attr) {
  int64_t value = cast<::mlir::IntegerAttr>(attr).getValue().getSExtValue();
  return success((value & (value - 1)) == 0);
}

namespace {

struct PowerOfTwoExpandRhs : ::mlir::PDLPatternModule {
  template <typename... ConfigsT>
  PowerOfTwoExpandRhs(::mlir::MLIRContext *context, ConfigsT &&...configs)
    : ::mlir::PDLPatternModule(::mlir::parseSourceString<::mlir::ModuleOp>(
R"mlir(pdl.pattern @PowerOfTwoExpandRhs : benefit(2) {
  %0 = operands loc("lib/Transform/Arith/MulToAdd.pdll":16:29)
  %1 = attribute loc("lib/Transform/Arith/MulToAdd.pdll":16:57)
  %2 = types loc("lib/Transform/Arith/MulToAdd.pdll":16:29)
  %3 = operation "arith.constant"(%0 : !pdl.range<value>)  {"value" = %1} -> (%2 : !pdl.range<type>) loc("lib/Transform/Arith/MulToAdd.pdll":16:29)
  %4 = result 0 of %3 loc("lib/Transform/Arith/MulToAdd.pdll":16:29)
  %5 = operand loc("lib/Transform/Arith/MulToAdd.pdll":16:71)
  %6 = types loc("lib/Transform/Arith/MulToAdd.pdll":16:14)
  %7 = operation "arith.muli"(%4, %5 : !pdl.value, !pdl.value)  -> (%6 : !pdl.range<type>) loc("lib/Transform/Arith/MulToAdd.pdll":16:14)
  apply_native_constraint "IsPowerOfTwo"(%1 : !pdl.attribute) loc("lib/Transform/Arith/MulToAdd.pdll":17:3)
  %8 = apply_native_constraint "Halve"(%1 : !pdl.attribute) : !pdl.attribute loc("lib/Transform/Arith/MulToAdd.pdll":18:22)
  rewrite %7 {
    %9 = operation "arith.constant"  {"value" = %8} loc("lib/Transform/Arith/MulToAdd.pdll":21:20)
    %10 = result 0 of %9 loc("lib/Transform/Arith/MulToAdd.pdll":22:33)
    %11 = operation "arith.muli"(%10, %5 : !pdl.value, !pdl.value)  loc("lib/Transform/Arith/MulToAdd.pdll":22:18)
    %12 = result 0 of %11 loc("lib/Transform/Arith/MulToAdd.pdll":23:33)
    %13 = result 0 of %11 loc("lib/Transform/Arith/MulToAdd.pdll":23:41)
    %14 = operation "arith.addi"(%12, %13 : !pdl.value, !pdl.value)  loc("lib/Transform/Arith/MulToAdd.pdll":23:18)
    replace %7 with %14 loc("lib/Transform/Arith/MulToAdd.pdll":24:5)
  } loc("lib/Transform/Arith/MulToAdd.pdll":20:3)
} loc("lib/Transform/Arith/MulToAdd.pdll":14:1)
    )mlir", context), std::forward<ConfigsT>(configs)...) {
    registerConstraintFunction("IsPowerOfTwo", IsPowerOfTwoPDLFn);
  }
};


struct PowerOfTwoExpandLhs : ::mlir::PDLPatternModule {
  template <typename... ConfigsT>
  PowerOfTwoExpandLhs(::mlir::MLIRContext *context, ConfigsT &&...configs)
    : ::mlir::PDLPatternModule(::mlir::parseSourceString<::mlir::ModuleOp>(
R"mlir(pdl.pattern @PowerOfTwoExpandLhs : benefit(2) {
  %0 = operand loc("lib/Transform/Arith/MulToAdd.pdll":31:29)
  %1 = operands loc("lib/Transform/Arith/MulToAdd.pdll":31:41)
  %2 = attribute loc("lib/Transform/Arith/MulToAdd.pdll":31:69)
  %3 = types loc("lib/Transform/Arith/MulToAdd.pdll":31:41)
  %4 = operation "arith.constant"(%1 : !pdl.range<value>)  {"value" = %2} -> (%3 : !pdl.range<type>) loc("lib/Transform/Arith/MulToAdd.pdll":31:41)
  %5 = result 0 of %4 loc("lib/Transform/Arith/MulToAdd.pdll":31:41)
  %6 = types loc("lib/Transform/Arith/MulToAdd.pdll":31:14)
  %7 = operation "arith.muli"(%0, %5 : !pdl.value, !pdl.value)  -> (%6 : !pdl.range<type>) loc("lib/Transform/Arith/MulToAdd.pdll":31:14)
  apply_native_constraint "IsPowerOfTwo"(%2 : !pdl.attribute) loc("lib/Transform/Arith/MulToAdd.pdll":32:3)
  %8 = apply_native_constraint "Halve"(%2 : !pdl.attribute) : !pdl.attribute loc("lib/Transform/Arith/MulToAdd.pdll":33:22)
  rewrite %7 {
    %9 = operation "arith.constant"  {"value" = %8} loc("lib/Transform/Arith/MulToAdd.pdll":36:20)
    %10 = result 0 of %9 loc("lib/Transform/Arith/MulToAdd.pdll":37:38)
    %11 = operation "arith.muli"(%0, %10 : !pdl.value, !pdl.value)  loc("lib/Transform/Arith/MulToAdd.pdll":37:18)
    %12 = result 0 of %11 loc("lib/Transform/Arith/MulToAdd.pdll":38:33)
    %13 = result 0 of %11 loc("lib/Transform/Arith/MulToAdd.pdll":38:41)
    %14 = operation "arith.addi"(%12, %13 : !pdl.value, !pdl.value)  loc("lib/Transform/Arith/MulToAdd.pdll":38:18)
    replace %7 with %14 loc("lib/Transform/Arith/MulToAdd.pdll":39:5)
  } loc("lib/Transform/Arith/MulToAdd.pdll":35:3)
} loc("lib/Transform/Arith/MulToAdd.pdll":30:1)
    )mlir", context), std::forward<ConfigsT>(configs)...) {
    registerConstraintFunction("IsPowerOfTwo", IsPowerOfTwoPDLFn);
  }
};


struct PeelFromMulRhs : ::mlir::PDLPatternModule {
  template <typename... ConfigsT>
  PeelFromMulRhs(::mlir::MLIRContext *context, ConfigsT &&...configs)
    : ::mlir::PDLPatternModule(::mlir::parseSourceString<::mlir::ModuleOp>(
R"mlir(pdl.pattern @PeelFromMulRhs : benefit(1) {
  %0 = operand loc("lib/Transform/Arith/MulToAdd.pdll":45:29)
  %1 = operands loc("lib/Transform/Arith/MulToAdd.pdll":45:41)
  %2 = attribute loc("lib/Transform/Arith/MulToAdd.pdll":45:69)
  %3 = types loc("lib/Transform/Arith/MulToAdd.pdll":45:41)
  %4 = operation "arith.constant"(%1 : !pdl.range<value>)  {"value" = %2} -> (%3 : !pdl.range<type>) loc("lib/Transform/Arith/MulToAdd.pdll":45:41)
  %5 = result 0 of %4 loc("lib/Transform/Arith/MulToAdd.pdll":45:41)
  %6 = types loc("lib/Transform/Arith/MulToAdd.pdll":45:14)
  %7 = operation "arith.muli"(%0, %5 : !pdl.value, !pdl.value)  -> (%6 : !pdl.range<type>) loc("lib/Transform/Arith/MulToAdd.pdll":45:14)
  %8 = apply_native_constraint "MinusOne"(%2 : !pdl.attribute) : !pdl.attribute loc("lib/Transform/Arith/MulToAdd.pdll":50:24)
  rewrite %7 {
    %9 = operation "arith.constant"  {"value" = %8} loc("lib/Transform/Arith/MulToAdd.pdll":53:20)
    %10 = result 0 of %9 loc("lib/Transform/Arith/MulToAdd.pdll":54:38)
    %11 = operation "arith.muli"(%0, %10 : !pdl.value, !pdl.value)  loc("lib/Transform/Arith/MulToAdd.pdll":54:18)
    %12 = result 0 of %11 loc("lib/Transform/Arith/MulToAdd.pdll":55:33)
    %13 = operation "arith.addi"(%12, %0 : !pdl.value, !pdl.value)  loc("lib/Transform/Arith/MulToAdd.pdll":55:18)
    replace %7 with %13 loc("lib/Transform/Arith/MulToAdd.pdll":56:5)
  } loc("lib/Transform/Arith/MulToAdd.pdll":52:3)
} loc("lib/Transform/Arith/MulToAdd.pdll":44:1)
    )mlir", context), std::forward<ConfigsT>(configs)...) {
  }
};


struct PeelFromMulLhs : ::mlir::PDLPatternModule {
  template <typename... ConfigsT>
  PeelFromMulLhs(::mlir::MLIRContext *context, ConfigsT &&...configs)
    : ::mlir::PDLPatternModule(::mlir::parseSourceString<::mlir::ModuleOp>(
R"mlir(pdl.pattern @PeelFromMulLhs : benefit(1) {
  %0 = operands loc("lib/Transform/Arith/MulToAdd.pdll":62:29)
  %1 = attribute loc("lib/Transform/Arith/MulToAdd.pdll":62:57)
  %2 = types loc("lib/Transform/Arith/MulToAdd.pdll":62:29)
  %3 = operation "arith.constant"(%0 : !pdl.range<value>)  {"value" = %1} -> (%2 : !pdl.range<type>) loc("lib/Transform/Arith/MulToAdd.pdll":62:29)
  %4 = result 0 of %3 loc("lib/Transform/Arith/MulToAdd.pdll":62:29)
  %5 = operand loc("lib/Transform/Arith/MulToAdd.pdll":62:71)
  %6 = types loc("lib/Transform/Arith/MulToAdd.pdll":62:14)
  %7 = operation "arith.muli"(%4, %5 : !pdl.value, !pdl.value)  -> (%6 : !pdl.range<type>) loc("lib/Transform/Arith/MulToAdd.pdll":62:14)
  %8 = apply_native_constraint "MinusOne"(%1 : !pdl.attribute) : !pdl.attribute loc("lib/Transform/Arith/MulToAdd.pdll":63:24)
  rewrite %7 {
    %9 = operation "arith.constant"  {"value" = %8} loc("lib/Transform/Arith/MulToAdd.pdll":66:20)
    %10 = result 0 of %9 loc("lib/Transform/Arith/MulToAdd.pdll":67:33)
    %11 = operation "arith.muli"(%10, %5 : !pdl.value, !pdl.value)  loc("lib/Transform/Arith/MulToAdd.pdll":67:18)
    %12 = result 0 of %11 loc("lib/Transform/Arith/MulToAdd.pdll":68:33)
    %13 = operation "arith.addi"(%12, %5 : !pdl.value, !pdl.value)  loc("lib/Transform/Arith/MulToAdd.pdll":68:18)
    replace %7 with %13 loc("lib/Transform/Arith/MulToAdd.pdll":69:5)
  } loc("lib/Transform/Arith/MulToAdd.pdll":65:3)
} loc("lib/Transform/Arith/MulToAdd.pdll":61:1)
    )mlir", context), std::forward<ConfigsT>(configs)...) {
  }
};

} // end namespace

template <typename... ConfigsT>
static void LLVM_ATTRIBUTE_UNUSED populateGeneratedPDLLPatterns(::mlir::RewritePatternSet &patterns, ConfigsT &&...configs) {
  patterns.add<PowerOfTwoExpandRhs>(patterns.getContext(), configs...);
  patterns.add<PowerOfTwoExpandLhs>(patterns.getContext(), configs...);
  patterns.add<PeelFromMulRhs>(patterns.getContext(), configs...);
  patterns.add<PeelFromMulLhs>(patterns.getContext(), configs...);
}
