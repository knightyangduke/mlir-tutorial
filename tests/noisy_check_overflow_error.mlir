../tests/noisy_checker_overflow.mlir:14:8: error: 'noisy.mul' op Noise value 50 exceeds the maximum allowable noise of 26.

  %5 = noisy.mul %4, %4 : !noisy.i32
       ^
../tests/noisy_checker_overflow.mlir:14:8: note: see current operation: %8 = "noisy.mul"(%7, %7) : (!noisy.i32, !noisy.i32) -> !noisy.i32
// -----// IR Dump After ReduceNoiseChecker Failed (noisy-reduce-noise-checker) //----- //
module {
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
}


