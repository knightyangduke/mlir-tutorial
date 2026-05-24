#ifndef LIB_TRANSFORM_ARITH_MULTOSHIFT_H_
#define LIB_TRANSFORM_ARITH_MULTOSHIFT_H_

#include "mlir/Pass/Pass.h"

namespace mlir {
namespace tutorial {

#define GEN_PASS_DECL_MULTOSHIFT
#include "lib/Transform/Arith/Passes.h.inc"

}  // namespace tutorial
}  // namespace mlir

#endif  // LIB_TRANSFORM_ARITH_MULTOSHIFT_H_
