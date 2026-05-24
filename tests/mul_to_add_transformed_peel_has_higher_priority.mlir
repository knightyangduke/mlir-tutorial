module {
  func.func @just_power_of_two(%arg0: i32) -> i32 {
    %0 = arith.addi %arg0, %arg0 : i32
    %1 = arith.addi %0, %arg0 : i32
    %2 = arith.addi %1, %arg0 : i32
    %3 = arith.addi %2, %arg0 : i32
    %4 = arith.addi %3, %arg0 : i32
    %5 = arith.addi %4, %arg0 : i32
    %6 = arith.addi %5, %arg0 : i32
    return %6 : i32
  }
  func.func @power_of_two_plus_one(%arg0: i32) -> i32 {
    %0 = arith.addi %arg0, %arg0 : i32
    %1 = arith.addi %0, %arg0 : i32
    %2 = arith.addi %1, %arg0 : i32
    %3 = arith.addi %2, %arg0 : i32
    %4 = arith.addi %3, %arg0 : i32
    %5 = arith.addi %4, %arg0 : i32
    %6 = arith.addi %5, %arg0 : i32
    %7 = arith.addi %6, %arg0 : i32
    return %7 : i32
  }
}

