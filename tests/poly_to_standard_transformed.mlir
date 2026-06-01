module {
  func.func @test_lower_add(%arg0: tensor<10xi32>, %arg1: tensor<10xi32>) -> tensor<10xi32> {
    %0 = arith.addi %arg0, %arg1 : tensor<10xi32>
    return %0 : tensor<10xi32>
  }
  func.func @test_lower_sub(%arg0: tensor<10xi32>, %arg1: tensor<10xi32>) -> tensor<10xi32> {
    %0 = arith.subi %arg0, %arg1 : tensor<10xi32>
    return %0 : tensor<10xi32>
  }
  func.func @test_lower_to_tensor(%arg0: tensor<10xi32>) -> tensor<10xi32> {
    return %arg0 : tensor<10xi32>
  }
  func.func @test_lower_from_tensor(%arg0: tensor<10xi32>) -> tensor<10xi32> {
    return %arg0 : tensor<10xi32>
  }
  func.func @test_lower_from_tensor_extend(%arg0: tensor<10xi32>) -> tensor<20xi32> {
    %c0_i32 = arith.constant 0 : i32
    %padded = tensor.pad %arg0 low[0] high[10] {
    ^bb0(%arg1: index):
      tensor.yield %c0_i32 : i32
    } : tensor<10xi32> to tensor<20xi32>
    return %padded : tensor<20xi32>
  }
  func.func @test_lower_add_and_fold() {
    %cst = arith.constant dense<[2, 3, 4]> : tensor<3xi32>
    %c0_i32 = arith.constant 0 : i32
    %padded = tensor.pad %cst low[0] high[7] {
    ^bb0(%arg0: index):
      tensor.yield %c0_i32 : i32
    } : tensor<3xi32> to tensor<10xi32>
    %cst_0 = arith.constant dense<[3, 4, 5]> : tensor<3xi32>
    %c0_i32_1 = arith.constant 0 : i32
    %padded_2 = tensor.pad %cst_0 low[0] high[7] {
    ^bb0(%arg0: index):
      tensor.yield %c0_i32_1 : i32
    } : tensor<3xi32> to tensor<10xi32>
    %cst_3 = arith.constant dense<[5, 7, 9]> : tensor<3xi32>
    %c0_i32_4 = arith.constant 0 : i32
    %padded_5 = tensor.pad %cst_3 low[0] high[7] {
    ^bb0(%arg0: index):
      tensor.yield %c0_i32_4 : i32
    } : tensor<3xi32> to tensor<10xi32>
    return
  }
  func.func @test_lower_mul(%arg0: tensor<10xi32>, %arg1: tensor<10xi32>) -> tensor<10xi32> {
    %cst = arith.constant dense<0> : tensor<10xi32>
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index
    %c1 = arith.constant 1 : index
    %0 = scf.for %arg2 = %c0 to %c10 step %c1 iter_args(%arg3 = %cst) -> (tensor<10xi32>) {
      %1 = scf.for %arg4 = %c0 to %c10 step %c1 iter_args(%arg5 = %arg3) -> (tensor<10xi32>) {
        %2 = arith.addi %arg2, %arg4 : index
        %3 = arith.remui %2, %c10 : index
        %extracted = tensor.extract %arg1[%arg4] : tensor<10xi32>
        %extracted_0 = tensor.extract %arg0[%arg2] : tensor<10xi32>
        %4 = arith.muli %extracted_0, %extracted : i32
        %extracted_1 = tensor.extract %arg5[%3] : tensor<10xi32>
        %5 = arith.addi %4, %extracted_1 : i32
        %inserted = tensor.insert %5 into %arg5[%3] : tensor<10xi32>
        scf.yield %inserted : tensor<10xi32>
      }
      scf.yield %1 : tensor<10xi32>
    }
    return %0 : tensor<10xi32>
  }
  func.func @test_lower_eval(%arg0: tensor<10xi32>, %arg1: i32) -> i32 {
    %c1 = arith.constant 1 : index
    %c10 = arith.constant 10 : index
    %c11 = arith.constant 11 : index
    %c0_i32 = arith.constant 0 : i32
    %0 = scf.for %arg2 = %c1 to %c11 step %c1 iter_args(%arg3 = %c0_i32) -> (i32) {
      %1 = arith.subi %c10, %arg2 : index
      %2 = arith.muli %arg1, %arg3 : i32
      %extracted = tensor.extract %arg0[%1] : tensor<10xi32>
      %3 = arith.addi %2, %extracted : i32
      scf.yield %3 : i32
    }
    return %0 : i32
  }
  func.func @test_lower_many(%arg0: tensor<10xi32>, %arg1: i32) -> i32 {
    %cst = arith.constant dense<[2, 3, 4]> : tensor<3xi32>
    %c0_i32 = arith.constant 0 : i32
    %padded = tensor.pad %cst low[0] high[7] {
    ^bb0(%arg2: index):
      tensor.yield %c0_i32 : i32
    } : tensor<3xi32> to tensor<10xi32>
    %0 = arith.addi %padded, %arg0 : tensor<10xi32>
    %cst_0 = arith.constant dense<0> : tensor<10xi32>
    %c0 = arith.constant 0 : index
    %c10 = arith.constant 10 : index
    %c1 = arith.constant 1 : index
    %1 = scf.for %arg2 = %c0 to %c10 step %c1 iter_args(%arg3 = %cst_0) -> (tensor<10xi32>) {
      %4 = scf.for %arg4 = %c0 to %c10 step %c1 iter_args(%arg5 = %arg3) -> (tensor<10xi32>) {
        %5 = arith.addi %arg2, %arg4 : index
        %6 = arith.remui %5, %c10 : index
        %extracted = tensor.extract %0[%arg4] : tensor<10xi32>
        %extracted_4 = tensor.extract %0[%arg2] : tensor<10xi32>
        %7 = arith.muli %extracted_4, %extracted : i32
        %extracted_5 = tensor.extract %arg5[%6] : tensor<10xi32>
        %8 = arith.addi %7, %extracted_5 : i32
        %inserted = tensor.insert %8 into %arg5[%6] : tensor<10xi32>
        scf.yield %inserted : tensor<10xi32>
      }
      scf.yield %4 : tensor<10xi32>
    }
    %2 = arith.subi %1, %arg0 : tensor<10xi32>
    %c1_1 = arith.constant 1 : index
    %c10_2 = arith.constant 10 : index
    %c11 = arith.constant 11 : index
    %c0_i32_3 = arith.constant 0 : i32
    %3 = scf.for %arg2 = %c1_1 to %c11 step %c1_1 iter_args(%arg3 = %c0_i32_3) -> (i32) {
      %4 = arith.subi %c10_2, %arg2 : index
      %5 = arith.muli %arg1, %arg3 : i32
      %extracted = tensor.extract %2[%4] : tensor<10xi32>
      %6 = arith.addi %5, %extracted : i32
      scf.yield %6 : i32
    }
    return %3 : i32
  }
}

