// RUN: { tutorial-opt %s --noisy-reduce-noise-checker 2>&1 || true; } | FileCheck %s
// The `{ ... || true; }` wrapper is needed because:
//   1. The pass reports an error (noise overflow) → tutorial-opt exits 1
//   2. lit's test runner uses `set -o pipefail`, so the pipeline exit code
//      equals the first non-zero exit in the chain
//   3. We wrap only tutorial-opt (not FileCheck) so that:
//        - tutorial-opt's exit 1 → swallowed by || true
//        - FileCheck's exit code → still propagates (if pattern fails, test fails)
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
