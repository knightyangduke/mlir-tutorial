#ifndef LIB_TRANSFORM_POLY_POLYCONSTOPTIMIZER_H_
#define LIB_TRANSFORM_POLY_POLYCONSTOPTIMIZER_H_

#include "mlir/Pass/Pass.h"

namespace mlir {
namespace tutorial {

#define GEN_PASS_DECL_OPTIMIZEPOLYCONST
#include "lib/Transform/Poly/Passes.h.inc"

}  // namespace tutorial
}  // namespace mlir

#endif  // LIB_TRANSFORM_POLY_POLYCONSTOPTIMIZER_H_