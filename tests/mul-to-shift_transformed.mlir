module {
  func.func @just_power_of_two(%arg0: i32) -> i32 {
    %c3_i32 = arith.constant 3 : i32
    %0 = arith.shli %arg0, %c3_i32 : i32
    return %0 : i32
  }
  func.func @power_of_two_plus_one(%arg0: i32) -> i32 {
    %c3_i32 = arith.constant 3 : i32
    %0 = arith.shli %arg0, %c3_i32 : i32
    %1 = arith.addi %0, %arg0 : i32
    return %1 : i32
  }
}

