#map = affine_map<(d0) -> (d0 + 1)>
#map1 = affine_map<(d0) -> (d0 + 2)>
#map2 = affine_map<(d0) -> (d0 + 3)>
module {
  func.func @test_single_nested_loop(%arg0: memref<4xi32>) -> i32 {
    %c0 = arith.constant 0 : index
    %c0_i32 = arith.constant 0 : i32
    %0 = affine.load %arg0[%c0] : memref<4xi32>
    %1 = arith.addi %c0_i32, %0 : i32
    %2 = affine.apply #map(%c0)
    %3 = affine.load %arg0[%2] : memref<4xi32>
    %4 = arith.addi %1, %3 : i32
    %5 = affine.apply #map1(%c0)
    %6 = affine.load %arg0[%5] : memref<4xi32>
    %7 = arith.addi %4, %6 : i32
    %8 = affine.apply #map2(%c0)
    %9 = affine.load %arg0[%8] : memref<4xi32>
    %10 = arith.addi %7, %9 : i32
    return %10 : i32
  }
  func.func @test_doubly_nested_loop(%arg0: memref<4x3xi32>) -> i32 {
    %c0 = arith.constant 0 : index
    %c0_0 = arith.constant 0 : index
    %c0_i32 = arith.constant 0 : i32
    %c0_i32_1 = arith.constant 0 : i32
    %0 = affine.load %arg0[%c0, %c0_0] : memref<4x3xi32>
    %1 = arith.addi %c0_i32_1, %0 : i32
    %2 = affine.apply #map(%c0_0)
    %3 = affine.load %arg0[%c0, %2] : memref<4x3xi32>
    %4 = arith.addi %1, %3 : i32
    %5 = affine.apply #map1(%c0_0)
    %6 = affine.load %arg0[%c0, %5] : memref<4x3xi32>
    %7 = arith.addi %4, %6 : i32
    %8 = arith.addi %c0_i32, %7 : i32
    %9 = affine.apply #map(%c0)
    %c0_i32_2 = arith.constant 0 : i32
    %10 = affine.load %arg0[%9, %c0_0] : memref<4x3xi32>
    %11 = arith.addi %c0_i32_2, %10 : i32
    %12 = affine.apply #map(%c0_0)
    %13 = affine.load %arg0[%9, %12] : memref<4x3xi32>
    %14 = arith.addi %11, %13 : i32
    %15 = affine.apply #map1(%c0_0)
    %16 = affine.load %arg0[%9, %15] : memref<4x3xi32>
    %17 = arith.addi %14, %16 : i32
    %18 = arith.addi %8, %17 : i32
    %19 = affine.apply #map1(%c0)
    %c0_i32_3 = arith.constant 0 : i32
    %20 = affine.load %arg0[%19, %c0_0] : memref<4x3xi32>
    %21 = arith.addi %c0_i32_3, %20 : i32
    %22 = affine.apply #map(%c0_0)
    %23 = affine.load %arg0[%19, %22] : memref<4x3xi32>
    %24 = arith.addi %21, %23 : i32
    %25 = affine.apply #map1(%c0_0)
    %26 = affine.load %arg0[%19, %25] : memref<4x3xi32>
    %27 = arith.addi %24, %26 : i32
    %28 = arith.addi %18, %27 : i32
    %29 = affine.apply #map2(%c0)
    %c0_i32_4 = arith.constant 0 : i32
    %30 = affine.load %arg0[%29, %c0_0] : memref<4x3xi32>
    %31 = arith.addi %c0_i32_4, %30 : i32
    %32 = affine.apply #map(%c0_0)
    %33 = affine.load %arg0[%29, %32] : memref<4x3xi32>
    %34 = arith.addi %31, %33 : i32
    %35 = affine.apply #map1(%c0_0)
    %36 = affine.load %arg0[%29, %35] : memref<4x3xi32>
    %37 = arith.addi %34, %36 : i32
    %38 = arith.addi %28, %37 : i32
    return %38 : i32
  }
}

