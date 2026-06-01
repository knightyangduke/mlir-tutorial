tests/poly_syntax.mlir:30:21: error: 'arith.muli' op operand #0 must be signless-integer-like, but got 'complex<f64>'
    %complex_eval = poly.eval %4, %z : (!poly.poly<10>, complex<f64>) -> complex<f64>
                    ^
tests/poly_syntax.mlir:30:21: note: see current operation: %40 = "arith.muli"(%17, %arg6) <{overflowFlags = #arith.overflow<none>}> : (complex<f64>, i32) -> complex<f64>
// -----// IR Dump After PolyToStandard Failed (poly-to-standard) //----- //
"builtin.module"() ({
  "func.func"() <{function_type = (tensor<10xi32>) -> tensor<10xi32>, sym_name = "test_type_syntax"}> ({
  ^bb0(%arg14: tensor<10xi32>):
    "func.return"(%arg14) : (tensor<10xi32>) -> ()
  }) : () -> ()
  "func.func"() <{function_type = (tensor<10xi32>, tensor<10xi32>) -> tensor<10xi32>, sym_name = "test_op_syntax"}> ({
  ^bb0(%arg0: tensor<10xi32>, %arg1: tensor<10xi32>):
    %0 = "arith.addi"(%arg0, %arg1) <{overflowFlags = #arith.overflow<none>}> : (tensor<10xi32>, tensor<10xi32>) -> tensor<10xi32>
    %1 = "arith.subi"(%arg0, %arg1) <{overflowFlags = #arith.overflow<none>}> : (tensor<10xi32>, tensor<10xi32>) -> tensor<10xi32>
    %2 = "arith.constant"() <{value = dense<0> : tensor<10xi32>}> : () -> tensor<10xi32>
    %3 = "arith.constant"() <{value = 0 : index}> : () -> index
    %4 = "arith.constant"() <{value = 10 : index}> : () -> index
    %5 = "arith.constant"() <{value = 1 : index}> : () -> index
    %6 = "scf.for"(%3, %4, %5, %2) ({
    ^bb0(%arg10: index, %arg11: tensor<10xi32>):
      %47 = "scf.for"(%3, %4, %5, %arg11) ({
      ^bb0(%arg12: index, %arg13: tensor<10xi32>):
        %48 = "arith.addi"(%arg10, %arg12) <{overflowFlags = #arith.overflow<none>}> : (index, index) -> index
        %49 = "arith.remui"(%48, %4) : (index, index) -> index
        %50 = "tensor.extract"(%arg1, %arg12) : (tensor<10xi32>, index) -> i32
        %51 = "tensor.extract"(%arg0, %arg10) : (tensor<10xi32>, index) -> i32
        %52 = "arith.muli"(%51, %50) <{overflowFlags = #arith.overflow<none>}> : (i32, i32) -> i32
        %53 = "tensor.extract"(%arg13, %49) : (tensor<10xi32>, index) -> i32
        %54 = "arith.addi"(%52, %53) <{overflowFlags = #arith.overflow<none>}> : (i32, i32) -> i32
        %55 = "tensor.insert"(%54, %arg13, %49) : (i32, tensor<10xi32>, index) -> tensor<10xi32>
        "scf.yield"(%55) : (tensor<10xi32>) -> ()
      }) : (index, index, index, tensor<10xi32>) -> tensor<10xi32>
      "scf.yield"(%47) : (tensor<10xi32>) -> ()
    }) : (index, index, index, tensor<10xi32>) -> tensor<10xi32>
    %7 = "arith.constant"() <{value = dense<[1, 2, 3]> : tensor<3xi32>}> : () -> tensor<3xi32>
    %8 = "arith.constant"() <{value = dense<[1, 2, 3]> : tensor<3xi32>}> : () -> tensor<3xi32>
    %9 = "arith.constant"() <{value = 0 : i32}> : () -> i32
    %10 = "tensor.pad"(%8) <{operandSegmentSizes = array<i32: 1, 0, 0>, static_high = array<i64: 7>, static_low = array<i64: 0>}> ({
    ^bb0(%arg9: index):
      "tensor.yield"(%9) : (i32) -> ()
    }) : (tensor<3xi32>) -> tensor<10xi32>
    %11 = "arith.constant"() <{value = 7 : i32}> : () -> i32
    %12 = "arith.constant"() <{value = 1 : index}> : () -> index
    %13 = "arith.constant"() <{value = 10 : index}> : () -> index
    %14 = "arith.constant"() <{value = 11 : index}> : () -> index
    %15 = "arith.constant"() <{value = 0 : i32}> : () -> i32
    %16 = "scf.for"(%12, %14, %12, %15) ({
    ^bb0(%arg7: index, %arg8: i32):
      %43 = "arith.subi"(%13, %arg7) <{overflowFlags = #arith.overflow<none>}> : (index, index) -> index
      %44 = "arith.muli"(%11, %arg8) <{overflowFlags = #arith.overflow<none>}> : (i32, i32) -> i32
      %45 = "tensor.extract"(%10, %43) : (tensor<10xi32>, index) -> i32
      %46 = "arith.addi"(%44, %45) <{overflowFlags = #arith.overflow<none>}> : (i32, i32) -> i32
      "scf.yield"(%46) : (i32) -> ()
    }) : (index, index, index, i32) -> i32
    %17 = "complex.constant"() <{value = [1.000000e+00, 2.000000e+00]}> : () -> complex<f64>
    %18 = "arith.constant"() <{value = 1 : index}> : () -> index
    %19 = "arith.constant"() <{value = 10 : index}> : () -> index
    %20 = "arith.constant"() <{value = 11 : index}> : () -> index
    %21 = "arith.constant"() <{value = 0 : i32}> : () -> i32
    %22 = "scf.for"(%18, %20, %18, %21) ({
    ^bb0(%arg5: index, %arg6: i32):
      %39 = "arith.subi"(%19, %arg5) <{overflowFlags = #arith.overflow<none>}> : (index, index) -> index
      %40 = "arith.muli"(%17, %arg6) <{overflowFlags = #arith.overflow<none>}> : (complex<f64>, i32) -> complex<f64>
      %41 = "tensor.extract"(%10, %39) : (tensor<10xi32>, index) -> i32
      %42 = "arith.addi"(%40, %41) <{overflowFlags = #arith.overflow<none>}> : (complex<f64>, i32) -> complex<f64>
      "scf.yield"(%42) : (complex<f64>) -> ()
    }) : (index, index, index, i32) -> i32
    %23 = "tensor.empty"() : () -> tensor<2x10xi32>
    %24 = "arith.constant"() <{value = 0 : index}> : () -> index
    %25 = "tensor.insert_slice"(%arg0, %23, %24) <{operandSegmentSizes = array<i32: 1, 1, 1, 0, 0>, static_offsets = array<i64: -9223372036854775808, 0>, static_sizes = array<i64: 1, 10>, static_strides = array<i64: 1, 1>}> : (tensor<10xi32>, tensor<2x10xi32>, index) -> tensor<2x10xi32>
    %26 = "arith.constant"() <{value = 1 : index}> : () -> index
    %27 = "tensor.insert_slice"(%arg1, %25, %26) <{operandSegmentSizes = array<i32: 1, 1, 1, 0, 0>, static_offsets = array<i64: -9223372036854775808, 0>, static_sizes = array<i64: 1, 10>, static_strides = array<i64: 1, 1>}> : (tensor<10xi32>, tensor<2x10xi32>, index) -> tensor<2x10xi32>
    %28 = "arith.addi"(%27, %27) <{overflowFlags = #arith.overflow<none>}> : (tensor<2x10xi32>, tensor<2x10xi32>) -> tensor<2x10xi32>
    %29 = "arith.constant"() <{value = dense<[2, 3, 4]> : tensor<3xi32>}> : () -> tensor<3xi32>
    %30 = "arith.constant"() <{value = 0 : i32}> : () -> i32
    %31 = "tensor.pad"(%29) <{operandSegmentSizes = array<i32: 1, 0, 0>, static_high = array<i64: 7>, static_low = array<i64: 0>}> ({
    ^bb0(%arg4: index):
      "tensor.yield"(%30) : (i32) -> ()
    }) : (tensor<3xi32>) -> tensor<10xi32>
    %32 = "arith.constant"() <{value = dense<[2, 3, 4]> : tensor<3xi8>}> : () -> tensor<3xi8>
    %33 = "arith.constant"() <{value = 0 : i32}> : () -> i32
    %34 = "tensor.pad"(%32) <{operandSegmentSizes = array<i32: 1, 0, 0>, static_high = array<i64: 7>, static_low = array<i64: 0>}> ({
    ^bb0(%arg3: index):
      "tensor.yield"(%33) : (i32) -> ()
    }) : (tensor<3xi8>) -> tensor<10xi32>
    %35 = "arith.constant"() <{value = dense<[2, 3, 4]> : tensor<3xi8>}> : () -> tensor<3xi8>
    %36 = "arith.constant"() <{value = 0 : i32}> : () -> i32
    %37 = "tensor.pad"(%35) <{operandSegmentSizes = array<i32: 1, 0, 0>, static_high = array<i64: 7>, static_low = array<i64: 0>}> ({
    ^bb0(%arg2: index):
      "tensor.yield"(%36) : (i32) -> ()
    }) : (tensor<3xi8>) -> tensor<10xi32>
    %38 = "arith.constant"() <{value = dense<4> : tensor<100xi32>}> : () -> tensor<100xi32>
    "func.return"(%10) : (tensor<10xi32>) -> ()
  }) : () -> ()
}) : () -> ()


