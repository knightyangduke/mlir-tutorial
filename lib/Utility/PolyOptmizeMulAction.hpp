#pragma once

#include "mlir/IR/Action.h"
#include "mlir/IR/Unit.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

namespace mlir {
namespace tracing {

/// Action fired each time MultiplyConstOne successfully rewrites a poly.mul.
/// Tag: "poly-optimize-mul"  —  pass --my-debug-tag=poly-optimize-mul to intercept.
class PolyOptmizeMul : public ActionImpl<PolyOptmizeMul> {
public:
  using Base = ActionImpl<PolyOptmizeMul>;
  /// irUnits should contain the enclosing func.func Operation* so the handler
  /// can print the whole function before/after the rewrite.
  PolyOptmizeMul(ArrayRef<IRUnit> irUnits)
      : Base(irUnits) {}
  static constexpr StringLiteral tag = "poly-optimize-mul";
  static constexpr StringLiteral desc =
      "Fired when MultiplyConstOne eliminates a poly.mul with an all-one constant";
  void print(llvm::raw_ostream &os) const override {
    os << "Action \"" << tag << "\" - " << desc;
  }
};

} // namespace tracing
} // namespace mlir