// Tests for the Action-based debug interception in PolyConstOptimize.
//
// Each test verifies two things:
//   1. The handler fires and prints the correct intercept header (stderr)
//   2. The IR is printed before and after the rewrite (stderr)
//   3. The pass still produces correct output (stdout)
//
// The handler output goes to stderr; we redirect 2>&1 to check it with
// FileCheck. A separate RUN line checks the pass output on stdout.

// ---------------------------------------------------------------------------
// poly-optimize-mul: fired once for a single poly.mul rewrite
// ---------------------------------------------------------------------------

// RUN: tutorial-opt %s --optimize-poly-const --my-debug-tag=poly-optimize-mul 2>&1 | FileCheck %s --check-prefix=MUL-ACTION
// RUN: tutorial-opt %s --optimize-poly-const 2>&1 | FileCheck %s --check-prefix=MUL-OUT

// MUL-ACTION: [INTERCEPTED] Action "poly-optimize-mul"
// MUL-ACTION: Trigger Count: 1
// MUL-ACTION: --- IR Before ---
// MUL-ACTION: poly.mul
// MUL-ACTION: --- IR After ---
// MUL-ACTION: return %arg0

// MUL-OUT-NOT: poly.mul
// MUL-OUT: return %arg0

// ---------------------------------------------------------------------------
// poly-optimize-add: fired once for a single poly.add rewrite
// ---------------------------------------------------------------------------

// RUN: tutorial-opt %s --optimize-poly-const --my-debug-tag=poly-optimize-add 2>&1 | FileCheck %s --check-prefix=ADD-ACTION
// RUN: tutorial-opt %s --optimize-poly-const 2>&1 | FileCheck %s --check-prefix=ADD-OUT

// ADD-ACTION: [INTERCEPTED] Action "poly-optimize-add"
// ADD-ACTION: Trigger Count: 1
// ADD-ACTION: --- IR Before ---
// ADD-ACTION: poly.add
// ADD-ACTION: --- IR After ---
// ADD-ACTION: return %arg0

// ADD-OUT-NOT: poly.add
// ADD-OUT: return %arg0

// ---------------------------------------------------------------------------
// tag isolation: passing poly-optimize-mul must NOT intercept poly.add rewrites
// ---------------------------------------------------------------------------

// RUN: tutorial-opt %s --optimize-poly-const --my-debug-tag=poly-optimize-mul 2>&1 | FileCheck %s --check-prefix=ISOLATE
// ISOLATE-NOT: [INTERCEPTED] Action "poly-optimize-add"

// ---------------------------------------------------------------------------
// no tag: handler must be silent (no [INTERCEPTED] lines at all)
// ---------------------------------------------------------------------------

// RUN: tutorial-opt %s --optimize-poly-const 2>&1 | FileCheck %s --check-prefix=SILENT
// SILENT-NOT: [INTERCEPTED]

// ---------------------------------------------------------------------------
// trigger count: both patterns applied → each action fires exactly once
// ---------------------------------------------------------------------------

// RUN: tutorial-opt %s --optimize-poly-const --my-debug-tag=poly-optimize-mul 2>&1 | FileCheck %s --check-prefix=COUNT-MUL
// RUN: tutorial-opt %s --optimize-poly-const --my-debug-tag=poly-optimize-add 2>&1 | FileCheck %s --check-prefix=COUNT-ADD
// COUNT-MUL: Trigger Count: 1
// COUNT-MUL-NOT: Trigger Count: 2
// COUNT-ADD: Trigger Count: 1
// COUNT-ADD-NOT: Trigger Count: 2

// ---------------------------------------------------------------------------
// Input IR: one poly.mul (c=1) and one poly.add (c=0) — both patterns apply.
// ---------------------------------------------------------------------------

func.func @test_action(%arg0: !poly.poly<10>) -> !poly.poly<10> {
  %c1 = poly.constant dense<1> : tensor<10xi32> : !poly.poly<10>
  %c0 = poly.constant dense<0> : tensor<10xi32> : !poly.poly<10>
  %0 = poly.mul %arg0, %c1 : !poly.poly<10>   // MultiplyConstOne fires → PolyOptmizeMul action
  %1 = poly.add %0, %c0 : !poly.poly<10>       // AddConstZero fires     → PolyOptmizeAdd action
  return %1 : !poly.poly<10>
}
