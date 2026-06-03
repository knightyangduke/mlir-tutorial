module {
  llvm.func @free(!llvm.ptr)
  llvm.func @malloc(i64) -> !llvm.ptr
  llvm.mlir.global private constant @__constant_3xi32(dense<[2, 3, 4]> : tensor<3xi32>) {addr_space = 0 : i32, alignment = 64 : i64} : !llvm.array<3 x i32>
  llvm.mlir.global private constant @__constant_10xi32(dense<0> : tensor<10xi32>) {addr_space = 0 : i32, alignment = 64 : i64} : !llvm.array<10 x i32>
  llvm.func @test_poly_fn(%arg0: i32) -> i32 {
    %0 = llvm.mlir.zero : !llvm.ptr
    %1 = llvm.mlir.constant(0 : i32) : i32
    %2 = llvm.mlir.addressof @__constant_10xi32 : !llvm.ptr
    %3 = llvm.mlir.addressof @__constant_3xi32 : !llvm.ptr
    %4 = llvm.mlir.constant(64 : index) : i64
    %5 = llvm.mlir.constant(3 : index) : i64
    %6 = llvm.mlir.constant(11 : index) : i64
    %7 = llvm.mlir.constant(1 : index) : i64
    %8 = llvm.mlir.constant(10 : index) : i64
    %9 = llvm.mlir.constant(0 : index) : i64
    %10 = llvm.getelementptr %2[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<10 x i32>
    %11 = llvm.getelementptr %3[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<3 x i32>
    %12 = llvm.getelementptr %0[10] : (!llvm.ptr) -> !llvm.ptr, i32
    %13 = llvm.ptrtoint %12 : !llvm.ptr to i64
    %14 = llvm.add %13, %4 : i64
    %15 = llvm.call @malloc(%14) : (i64) -> !llvm.ptr
    %16 = llvm.ptrtoint %15 : !llvm.ptr to i64
    %17 = llvm.sub %4, %7 : i64
    %18 = llvm.add %16, %17 : i64
    %19 = llvm.urem %18, %4 : i64
    %20 = llvm.sub %18, %19 : i64
    %21 = llvm.inttoptr %20 : i64 to !llvm.ptr
    llvm.br ^bb1(%9 : i64)
  ^bb1(%22: i64):  // 2 preds: ^bb0, ^bb2
    %23 = llvm.icmp "slt" %22, %8 : i64
    llvm.cond_br %23, ^bb2, ^bb3
  ^bb2:  // pred: ^bb1
    %24 = llvm.getelementptr inbounds|nuw %21[%22] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    llvm.store %arg0, %24 : i32, !llvm.ptr
    %25 = llvm.add %22, %7 : i64
    llvm.br ^bb1(%25 : i64)
  ^bb3:  // pred: ^bb1
    %26 = llvm.call @malloc(%14) : (i64) -> !llvm.ptr
    %27 = llvm.ptrtoint %26 : !llvm.ptr to i64
    %28 = llvm.add %27, %17 : i64
    %29 = llvm.urem %28, %4 : i64
    %30 = llvm.sub %28, %29 : i64
    %31 = llvm.inttoptr %30 : i64 to !llvm.ptr
    llvm.br ^bb4(%9 : i64)
  ^bb4(%32: i64):  // 2 preds: ^bb3, ^bb5
    %33 = llvm.icmp "slt" %32, %8 : i64
    llvm.cond_br %33, ^bb5, ^bb6
  ^bb5:  // pred: ^bb4
    %34 = llvm.getelementptr inbounds|nuw %31[%32] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    llvm.store %1, %34 : i32, !llvm.ptr
    %35 = llvm.add %32, %7 : i64
    llvm.br ^bb4(%35 : i64)
  ^bb6:  // pred: ^bb4
    %36 = llvm.mul %5, %7 : i64
    %37 = llvm.getelementptr %0[1] : (!llvm.ptr) -> !llvm.ptr, i32
    %38 = llvm.ptrtoint %37 : !llvm.ptr to i64
    %39 = llvm.mul %36, %38 : i64
    "llvm.intr.memcpy"(%31, %11, %39) <{isVolatile = false}> : (!llvm.ptr, !llvm.ptr, i64) -> ()
    llvm.br ^bb7(%9 : i64)
  ^bb7(%40: i64):  // 2 preds: ^bb6, ^bb8
    %41 = llvm.icmp "slt" %40, %8 : i64
    llvm.cond_br %41, ^bb8, ^bb9
  ^bb8:  // pred: ^bb7
    %42 = llvm.getelementptr inbounds|nuw %31[%40] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %43 = llvm.load %42 : !llvm.ptr -> i32
    %44 = llvm.getelementptr inbounds|nuw %21[%40] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %45 = llvm.load %44 : !llvm.ptr -> i32
    %46 = llvm.add %43, %45 : i32
    llvm.store %46, %42 : i32, !llvm.ptr
    %47 = llvm.add %40, %7 : i64
    llvm.br ^bb7(%47 : i64)
  ^bb9:  // pred: ^bb7
    %48 = llvm.call @malloc(%14) : (i64) -> !llvm.ptr
    %49 = llvm.ptrtoint %48 : !llvm.ptr to i64
    %50 = llvm.add %49, %17 : i64
    %51 = llvm.urem %50, %4 : i64
    %52 = llvm.sub %50, %51 : i64
    %53 = llvm.inttoptr %52 : i64 to !llvm.ptr
    %54 = llvm.mul %8, %7 : i64
    %55 = llvm.mul %54, %38 : i64
    "llvm.intr.memcpy"(%53, %10, %55) <{isVolatile = false}> : (!llvm.ptr, !llvm.ptr, i64) -> ()
    llvm.br ^bb10(%9 : i64)
  ^bb10(%56: i64):  // 2 preds: ^bb9, ^bb14
    %57 = llvm.icmp "slt" %56, %8 : i64
    llvm.cond_br %57, ^bb11, ^bb15
  ^bb11:  // pred: ^bb10
    llvm.br ^bb12(%9 : i64)
  ^bb12(%58: i64):  // 2 preds: ^bb11, ^bb13
    %59 = llvm.icmp "slt" %58, %8 : i64
    llvm.cond_br %59, ^bb13, ^bb14
  ^bb13:  // pred: ^bb12
    %60 = llvm.add %56, %58 : i64
    %61 = llvm.urem %60, %8 : i64
    %62 = llvm.getelementptr inbounds|nuw %31[%58] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %63 = llvm.load %62 : !llvm.ptr -> i32
    %64 = llvm.getelementptr inbounds|nuw %31[%56] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %65 = llvm.load %64 : !llvm.ptr -> i32
    %66 = llvm.mul %65, %63 : i32
    %67 = llvm.getelementptr inbounds|nuw %53[%61] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %68 = llvm.load %67 : !llvm.ptr -> i32
    %69 = llvm.add %66, %68 : i32
    llvm.store %69, %67 : i32, !llvm.ptr
    %70 = llvm.add %58, %7 : i64
    llvm.br ^bb12(%70 : i64)
  ^bb14:  // pred: ^bb12
    %71 = llvm.add %56, %7 : i64
    llvm.br ^bb10(%71 : i64)
  ^bb15:  // pred: ^bb10
    llvm.br ^bb16(%9 : i64)
  ^bb16(%72: i64):  // 2 preds: ^bb15, ^bb17
    %73 = llvm.icmp "slt" %72, %8 : i64
    llvm.cond_br %73, ^bb17, ^bb18
  ^bb17:  // pred: ^bb16
    %74 = llvm.getelementptr inbounds|nuw %53[%72] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %75 = llvm.load %74 : !llvm.ptr -> i32
    %76 = llvm.getelementptr inbounds|nuw %21[%72] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %77 = llvm.load %76 : !llvm.ptr -> i32
    %78 = llvm.sub %75, %77 : i32
    llvm.store %78, %74 : i32, !llvm.ptr
    %79 = llvm.add %72, %7 : i64
    llvm.br ^bb16(%79 : i64)
  ^bb18:  // pred: ^bb16
    llvm.br ^bb19(%7, %1 : i64, i32)
  ^bb19(%80: i64, %81: i32):  // 2 preds: ^bb18, ^bb20
    %82 = llvm.icmp "slt" %80, %6 : i64
    llvm.cond_br %82, ^bb20, ^bb21
  ^bb20:  // pred: ^bb19
    %83 = llvm.sub %8, %80 : i64
    %84 = llvm.mul %arg0, %81 : i32
    %85 = llvm.getelementptr inbounds|nuw %53[%83] : (!llvm.ptr, i64) -> !llvm.ptr, i32
    %86 = llvm.load %85 : !llvm.ptr -> i32
    %87 = llvm.add %84, %86 : i32
    %88 = llvm.add %80, %7 : i64
    llvm.br ^bb19(%88, %87 : i64, i32)
  ^bb21:  // pred: ^bb19
    llvm.call @free(%15) : (!llvm.ptr) -> ()
    llvm.call @free(%26) : (!llvm.ptr) -> ()
    llvm.call @free(%48) : (!llvm.ptr) -> ()
    llvm.return %81 : i32
  }
}

