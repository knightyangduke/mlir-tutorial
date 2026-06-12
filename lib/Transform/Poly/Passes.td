#ifndef LIB_TRANSFORM_POLY_PASSES_TD_
#define LIB_TRANSFORM_POLY_PASSES_TD_

include "mlir/Pass/PassBase.td"

def OptimizePolyConst : Pass<"optimize-poly-const"> {
  let summary = "Optimize poly constant operations";
  let description = [{
    Optimize poly constant operations.
  }];
}



#endif  // LIB_TRANSFORM_POLY_PASSES_TD_