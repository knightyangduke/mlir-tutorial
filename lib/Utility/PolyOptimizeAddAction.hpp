#pragma once

#include "mlir/IR/Action.h"
#include "mlir/IR/Unit.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

namespace mlir {
namespace tracing {

/// Action fired each time AddConstZero successfully rewrites a poly.add.
/// Tag: "poly-optimize-add"  —  pass --my-debug-tag=poly-optimize-add to intercept.
class PolyOptmizeAdd : public ActionImpl<PolyOptmizeAdd> {
public:
  using Base = ActionImpl<PolyOptmizeAdd>;
  /// irUnits should contain the enclosing func.func Operation* so the handler
  /// can print the whole function before/after the rewrite.
  PolyOptmizeAdd(ArrayRef<IRUnit> irUnits)
      : Base(irUnits) {}
  static constexpr StringLiteral tag = "poly-optimize-add";
  static constexpr StringLiteral desc =
      "Fired when AddConstZero eliminates a poly.add with a zero constant";
  void print(llvm::raw_ostream &os) const override {
    os << "Action \"" << tag << "\" - " << desc;
  }
};

} // namespace tracing
} // namespace mlir