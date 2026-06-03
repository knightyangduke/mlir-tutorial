// RUN: tutorial-opt %s --noisy-reduce-noise-checker 2>&1 | FileCheck %s
// Ensure the checker reports an error when noise exceeds the maximum.

// CHECK: error: 'noisy.mul' op Noise value 50 exceeds the maximum allowable noise of 26.
func.func @test_noise_overflow() {
  %c3_i5 = arith.constant 3 : i5
  %c4_i5 = arith.constant 4 : i5
  %c5_i5 = arith.constant 5 : i5
  %0 = noisy.encode %c3_i5 : i5 -> !noisy.i32
  %1 = noisy.encode %c4_i5 : i5 -> !noisy.i32
  %2 = noisy.encode %c5_i5 : i5 -> !noisy.i32
  %3 = noisy.add %0, %1 : !noisy.i32
  %4 = noisy.mul %3, %2 : !noisy.i32
  %5 = noisy.mul %4, %4 : !noisy.i32
  %6 = noisy.decode %5 : !noisy.i32 -> i5
  return
}
