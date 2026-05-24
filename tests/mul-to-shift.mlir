
func.func @just_power_of_two(%arg: i32) -> i32 {
  %0 = arith.constant 8 : i32
  %1 = arith.muli %arg, %0 : i32
  func.return %1 : i32
}

func.func @power_of_two_plus_one(%arg: i32) -> i32 {
  %0 = arith.constant 9 : i32
  %1 = arith.muli %arg, %0 : i32
  func.return %1 : i32
}
