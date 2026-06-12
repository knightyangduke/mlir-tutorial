// RUN: tutorial-opt %s --optimize-poly-const | FileCheck %s

// ---------------------------------------------------------------------------
// MultiplyConstOne: replace y = C * x  with  y = x  when all coefficients are 1
// ---------------------------------------------------------------------------

// CHECK-LABEL: test_mul_by_one
func.func @test_mul_by_one(%arg0: !poly.poly<10>) -> (!poly.poly<10>, !poly.poly<10>) {
  // Constant on RHS.
  %c = poly.constant dense<1> : tensor<10xi32> : !poly.poly<10>
  %rhs = poly.mul %arg0, %c : !poly.poly<10>
  // Constant on LHS — still commutative.
  %lhs = poly.mul %c, %arg0 : !poly.poly<10>
  // CHECK-NOT: poly.mul
  // CHECK: return %arg0, %arg0
  return %rhs, %lhs : !poly.poly<10>, !poly.poly<10>
}

// CHECK-LABEL: test_mul_not_applied
func.func @test_mul_not_applied(%arg0: !poly.poly<10>) -> (!poly.poly<10>, !poly.poly<10>) {
  // Mixed coefficients [1, 2] — not all 1, pattern should NOT apply.
  %c1 = poly.constant dense<[1, 2]> : tensor<2xi32> : !poly.poly<10>
  %mixed = poly.mul %arg0, %c1 : !poly.poly<10>
  // All coefficients = 2 — not identity, pattern should NOT apply.
  %c2 = poly.constant dense<2> : tensor<10xi32> : !poly.poly<10>
  %all2 = poly.mul %arg0, %c2 : !poly.poly<10>
  // CHECK: poly.mul
  // CHECK: poly.mul
  return %mixed, %all2 : !poly.poly<10>, !poly.poly<10>
}

// ---------------------------------------------------------------------------
// AddConstZero: replace y = C + x  with  y = x  when all coefficients are 0
// ---------------------------------------------------------------------------

// CHECK-LABEL: test_add_zero
func.func @test_add_zero(%arg0: !poly.poly<10>) -> (!poly.poly<10>, !poly.poly<10>) {
  // Zero polynomial on RHS.
  %c = poly.constant dense<0> : tensor<10xi32> : !poly.poly<10>
  %rhs = poly.add %arg0, %c : !poly.poly<10>
  // Zero polynomial on LHS — still commutative.
  %lhs = poly.add %c, %arg0 : !poly.poly<10>
  // CHECK-NOT: poly.add
  // CHECK: return %arg0, %arg0
  return %rhs, %lhs : !poly.poly<10>, !poly.poly<10>
}

// CHECK-LABEL: test_add_not_applied
func.func @test_add_not_applied(%arg0: !poly.poly<10>) -> (!poly.poly<10>, !poly.poly<10>) {
  // Mixed coefficients [0, 1] — not all 0, pattern should NOT apply.
  %c1 = poly.constant dense<[0, 1]> : tensor<2xi32> : !poly.poly<10>
  %mixed = poly.add %arg0, %c1 : !poly.poly<10>
  // All coefficients = 1 — not zero, pattern should NOT apply.
  %c2 = poly.constant dense<1> : tensor<10xi32> : !poly.poly<10>
  %all1 = poly.add %c2, %arg0 : !poly.poly<10>
  // CHECK: poly.add
  // CHECK: poly.add
  return %mixed, %all1 : !poly.poly<10>, !poly.poly<10>
}

// ---------------------------------------------------------------------------
// No constant operand — neither pattern should apply
// ---------------------------------------------------------------------------

// CHECK-LABEL: test_no_const
func.func @test_no_const(%arg0: !poly.poly<10>, %arg1: !poly.poly<10>) -> (!poly.poly<10>, !poly.poly<10>) {
  %mul = poly.mul %arg0, %arg1 : !poly.poly<10>
  %add = poly.add %arg0, %arg1 : !poly.poly<10>
  // CHECK: poly.mul
  // CHECK: poly.add
  return %mul, %add : !poly.poly<10>, !poly.poly<10>
}

// ---------------------------------------------------------------------------
// Combined: both patterns applied in one function
// ---------------------------------------------------------------------------

// CHECK-LABEL: test_combined
func.func @test_combined(%arg0: !poly.poly<10>) -> !poly.poly<10> {
  %c1 = poly.constant dense<1> : tensor<10xi32> : !poly.poly<10>
  %c0 = poly.constant dense<0> : tensor<10xi32> : !poly.poly<10>
  %0  = poly.mul %arg0, %c1 : !poly.poly<10>    // arg0 * 1  →  arg0
  %1  = poly.add %0, %c0 : !poly.poly<10>        // arg0 + 0  →  arg0
  // CHECK-NOT: poly.mul
  // CHECK-NOT: poly.add
  // CHECK: return %arg0
  return %1 : !poly.poly<10>
}
