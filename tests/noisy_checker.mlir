// RUN: tutorial-opt %s --noisy-reduce-noise-checker | FileCheck %s
// Ensure the checker passes cleanly when noise stays within bounds.

// CHECK-LABEL: test_noise_within_bounds
// CHECK:      return
func.func @test_noise_within_bounds() {
  %c3_i5 = arith.constant 3 : i5
  %c4_i5 = arith.constant 4 : i5
  %0 = noisy.encode %c3_i5 : i5 -> !noisy.i32
  %1 = noisy.encode %c4_i5 : i5 -> !noisy.i32
  %2 = noisy.add %0, %1 : !noisy.i32
  %3 = noisy.mul %2, %2 : !noisy.i32
  %4 = noisy.reduce_noise %3 : !noisy.i32
  %5 = noisy.decode %4 : !noisy.i32 -> i5
  return
}
