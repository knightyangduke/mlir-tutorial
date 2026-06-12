// RUN: tutorial-reduce %s -reduction-tree='traversal-mode=0 test=%S/poly_reduce_interesting.py' 2>/dev/null | FileCheck %s
// RUN: tutorial-reduce %s --opt-reduction-pass='opt-pass=optimize-poly-const test=%S/poly_reduce_interesting.py' 2>/dev/null | FileCheck %s
//
// `2>/dev/null` suppresses stderr from intermediate reduction attempts that
// produce invalid IR (e.g., "null operand found"). These are expected — they
// are variants that the interestingness script correctly rejected.  Without
// the redirect, stderr noise would leak into FileCheck and break matching.
//
// line breakdown:
//   RUN 1: --reduction-tree
//     Uses operation elimination + dialect reduction patterns (MultiplyConstOne,
//     AddConstZero) + built-in passes.  Patterns are discovered automatically
//     via PolyReductionPatternInterface registered on the poly dialect.
//
//   RUN 2: --opt-reduction-pass
//     Runs ONLY the --optimize-poly-const pass to reduce.  No operation
//     elimination, no other dialects' patterns.  This exercises the pass
//     registration path (registerPolyPasses() from Passes.h).
//
//   tutorial-reduce    - our custom mlir-reduce binary (loads poly dialect + passes)
//   %s                 - lit substitution: path to THIS file
//   -reduction-tree    - use the reduction-tree algorithm (vs. opt-reduction-pass)
//   traversal-mode=0   - single-path traversal: try one reduction variant per step
//   test=%S/...py      - interestingness script; %S = directory of this test file
//                        mlir-reduce calls this script on each variant to decide
//                        if it's "interesting" (exit 1) or should be discarded (exit 0)
//   | FileCheck %s     - pipe the final reduced output to FileCheck, which reads
//                        CHECK lines from this same file
//
// To run this test manually from the project root:
//   ./build-ninja/tools/tutorial-reduce tests/poly_reduce.mlir \
//     -reduction-tree='traversal-mode=0 test=tests/poly_reduce_interesting.py'

// poly_reduce.mlir - Test that mlir-reduce uses PolyReductionPatternInterface
// to simplify identity poly operations during reduction.
//
// The reduction patterns (MultiplyConstOne, AddConstZero) registered on the
// poly dialect should fire during mlir-reduce, eliminating identity operations:
//   poly.mul %x, <all-1 constant>  →  %x
//   poly.add %x, <all-0 constant>  →  %x
//
// The interestingness script simply checks that tutorial-opt can still parse
// the reduced file.  This is a minimal sanity check — a real interestingness
// script would check for a specific error message or diagnostic.

// MultiplyConstOne should eliminate the mul, AddConstZero the add.
// CHECK-LABEL: func.func @test_reduce
// CHECK-NOT: poly.mul
// CHECK-NOT: poly.add
// CHECK: return %arg0
func.func @test_reduce(%arg0: !poly.poly<10>) -> !poly.poly<10> {
  // These identity constants should be eliminated by reduction patterns.
  %c1 = poly.constant dense<1> : tensor<10xi32> : !poly.poly<10>
  %c0 = poly.constant dense<0> : tensor<10xi32> : !poly.poly<10>

  // poly.mul by all-1 → MultiplyConstOne should fire
  %0 = poly.mul %arg0, %c1 : !poly.poly<10>

  // poly.add by all-0 → AddConstZero should fire
  %1 = poly.add %0, %c0 : !poly.poly<10>

  return %1 : !poly.poly<10>
}
