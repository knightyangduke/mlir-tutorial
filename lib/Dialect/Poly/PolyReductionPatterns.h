#ifndef LIB_DIALECT_POLY_POLYREDUCTIONPATTERNS_H_
#define LIB_DIALECT_POLY_POLYREDUCTIONPATTERNS_H_

#include "mlir/IR/PatternMatch.h"

namespace mlir {
class RewritePatternSet;
} // namespace mlir

namespace mlir {
namespace tutorial {
namespace poly {

/// Populate the given RewritePatternSet with reduction patterns for the Poly
/// dialect. These patterns simplify polynomial operations (e.g., multiply by
/// constant 1, add constant 0) to help mlir-reduce shrink the input IR.
void populatePolyReductionPatterns(RewritePatternSet &patterns);

} // namespace poly
} // namespace tutorial
} // namespace mlir

#endif // LIB_DIALECT_POLY_POLYREDUCTIONPATTERNS_H_
