#ifndef LIB_DIALECT_POLY_POLYOPS_TD_
#define LIB_DIALECT_POLY_POLYOPS_TD_

include "PolyDialect.td"
include "PolyTypes.td"
include "mlir/IR/BuiltinAttributes.td"
include "mlir/IR/OpBase.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

// Type constraint for poly binop arguments: polys, vectors of polys, or
// tensors of polys.
def PolyOrContainer : TypeOrValueSemanticsContainer<Polynomial, "poly-or-container">;

// Inject verification that all integer-like arguments are 32-bits
def Has32BitArguments : NativeOpTrait<"Has32BitArguments"> {
  let cppNamespace = "::mlir::tutorial::poly";
}

class Poly_BinOp<string mnemonic> : Op<Poly_Dialect, mnemonic, [Pure, ElementwiseMappable, SameOperandsAndResultType]> {
  let arguments = (ins PolyOrContainer:$lhs, PolyOrContainer:$rhs);
  let results = (outs PolyOrContainer:$output);
  // assemblyFormat: prints as `poly.add %lhs, %rhs {optional attrs} : !poly.poly<N>`
  //   $lhs, $rhs      — operands; attr-dict — op attributes (omitted if empty);
  //   qualified(type) — output type with full dialect prefix, e.g. !poly.poly<3072>
  let assemblyFormat = "$lhs `,` $rhs attr-dict `:` qualified(type($output))";
  let hasFolder = 1;
  let hasCanonicalizer = 1;
}

def Poly_AddOp : Poly_BinOp<"add"> {
  let summary = "Addition operation between polynomials.";
  // Example:
  //   %0 = poly.add %p, %q : !poly.poly<3072>
}

def Poly_SubOp : Poly_BinOp<"sub"> {
  let summary = "Subtraction operation between polynomials.";
  // Example:
  //   %0 = poly.sub %p, %q : !poly.poly<3072>
}

def Poly_MulOp : Poly_BinOp<"mul"> {
  let summary = "Multiplication operation between polynomials.";
  // Example:
  //   %0 = poly.mul %p, %q : !poly.poly<3072>
}

def Poly_FromTensorOp : Op<Poly_Dialect, "from_tensor", [Pure]> {
  let summary = "Creates a Polynomial from integer coefficients stored in a tensor.";
  // Example (coefficients [1, 2, 3] → 1 + 2x + 3x^2):
  //   %0 = poly.from_tensor %t : tensor<3xi32> -> !poly.poly<3072>
  let arguments = (ins TensorOf<[AnyInteger]>:$input);
  let results = (outs Polynomial:$output);
  let assemblyFormat = "$input attr-dict `:` type($input) `->` qualified(type($output))";
  let hasFolder = 1;
}

def Poly_ToTensorOp : Op<Poly_Dialect, "to_tensor", [Pure]> {
  let summary = "Converts a polynomial to a tensor of its integer coefficients.";
  // Example:
  //   %t = poly.to_tensor %p : !poly.poly<3072> -> tensor<3072xi32>
  let arguments = (ins Polynomial:$input);
  let results = (outs TensorOf<[AnyInteger]>:$output);
  let assemblyFormat = "$input attr-dict `:` qualified(type($input)) `->` type($output)";
}

def IntOrComplex : AnyTypeOf<[AnyInteger, AnyComplex]>;

def Poly_EvalOp : Op<Poly_Dialect, "eval", [AllTypesMatch<["point", "output"]>, Has32BitArguments]> {
  let summary = "Evaluates a Polynomial at a given input value.";
  // Example:
  //   %r = poly.eval %p, %x : (!poly.poly<3072>, i32) -> i32
  let arguments = (ins Polynomial:$polynomial, IntOrComplex:$point);
  let results = (outs IntOrComplex:$output);
  let assemblyFormat = "$polynomial `,` $point attr-dict `:` `(` qualified(type($polynomial)) `,` type($point) `)` `->` type($output)";
  let hasVerifier = 1;
  let hasCanonicalizer = 1;
}

def Poly_ConstantOp : Op<Poly_Dialect, "constant", [Pure, ConstantLike]> {
  let summary = "Define a constant polynomial via an attribute.";
  // Example (constant 1 + 2x + 3x^2):
  //   %0 = poly.constant dense<[1, 2, 3]> : !poly.poly<3072>
  let arguments = (ins AnyIntElementsAttr:$coefficients);
  let results = (outs Polynomial:$output);
  let assemblyFormat = "$coefficients attr-dict `:` qualified(type($output))";
  let hasFolder = 1;
}


#endif  // LIB_DIALECT_POLY_POLYOPS_TD_
