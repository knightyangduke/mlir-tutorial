#include "lib/Dialect/Poly/PolyDialect.h"

#include "lib/Dialect/Poly/PolyOps.h"
#include "lib/Dialect/Poly/PolyTypes.h"
#include "mlir/IR/Builders.h"
#include "llvm/ADT/TypeSwitch.h"

#include "lib/Dialect/Poly/PolyDialect.cpp.inc"
#define GET_TYPEDEF_CLASSES
#include "lib/Dialect/Poly/PolyTypes.cpp.inc"
#define GET_OP_CLASSES
#include "lib/Dialect/Poly/PolyOps.cpp.inc"

namespace mlir {
namespace tutorial {
namespace poly {

void PolyDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "lib/Dialect/Poly/PolyTypes.cpp.inc"
      >();
  addOperations<
#define GET_OP_LIST
#include "lib/Dialect/Poly/PolyOps.cpp.inc"
      >();
}

// Required by the dialect interface so that MLIR's constant folding and
// canonicalization infrastructure can turn a folded Attribute back into an
// SSA value.  Without this, any pass that folds a poly operation to a
// constant attribute (e.g. constant propagation) would be unable to
// re-materialize that attribute as an operation and would silently drop it.
Operation *PolyDialect::materializeConstant(OpBuilder &builder, Attribute value,
                                            Type type, Location loc) {
  auto coeffs = dyn_cast<DenseIntElementsAttr>(value);
  if (!coeffs)
    return nullptr;
  return builder.create<ConstantOp>(loc, type, coeffs);
}

} // namespace poly
} // namespace tutorial
} // namespace mlir
