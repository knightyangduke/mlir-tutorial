#ifndef LIB_TRANSFORM_POLY_PASSES_H_
#define LIB_TRANSFORM_POLY_PASSES_H_

#include "lib/Transform/Poly/PolyConstOptimizer.h"

namespace mlir {
namespace tutorial {
namespace poly {

#define GEN_PASS_REGISTRATION
#include "lib/Transform/Poly/Passes.h.inc"

}  // namespace poly
}  // namespace tutorial
}  // namespace mlir

#endif  // LIB_TRANSFORM_POLY_PASSES_H_