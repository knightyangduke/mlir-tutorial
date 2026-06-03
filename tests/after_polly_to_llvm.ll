; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

@__constant_3xi32 = private constant [3 x i32] [i32 2, i32 3, i32 4], align 64
@__constant_10xi32 = private constant [10 x i32] zeroinitializer, align 64

declare void @free(ptr)

declare ptr @malloc(i64)

define i32 @test_poly_fn(i32 %0) {
  %2 = call ptr @malloc(i64 104)
  %3 = ptrtoint ptr %2 to i64
  %4 = add i64 %3, 63
  %5 = urem i64 %4, 64
  %6 = sub i64 %4, %5
  %7 = inttoptr i64 %6 to ptr
  br label %8

8:                                                ; preds = %11, %1
  %9 = phi i64 [ %13, %11 ], [ 0, %1 ]
  %10 = icmp slt i64 %9, 10
  br i1 %10, label %11, label %14

11:                                               ; preds = %8
  %12 = getelementptr inbounds nuw i32, ptr %7, i64 %9
  store i32 %0, ptr %12, align 4
  %13 = add i64 %9, 1
  br label %8

14:                                               ; preds = %8
  %15 = call ptr @malloc(i64 104)
  %16 = ptrtoint ptr %15 to i64
  %17 = add i64 %16, 63
  %18 = urem i64 %17, 64
  %19 = sub i64 %17, %18
  %20 = inttoptr i64 %19 to ptr
  br label %21

21:                                               ; preds = %24, %14
  %22 = phi i64 [ %26, %24 ], [ 0, %14 ]
  %23 = icmp slt i64 %22, 10
  br i1 %23, label %24, label %27

24:                                               ; preds = %21
  %25 = getelementptr inbounds nuw i32, ptr %20, i64 %22
  store i32 0, ptr %25, align 4
  %26 = add i64 %22, 1
  br label %21

27:                                               ; preds = %21
  call void @llvm.memcpy.p0.p0.i64(ptr %20, ptr @__constant_3xi32, i64 12, i1 false)
  br label %28

28:                                               ; preds = %31, %27
  %29 = phi i64 [ %37, %31 ], [ 0, %27 ]
  %30 = icmp slt i64 %29, 10
  br i1 %30, label %31, label %38

31:                                               ; preds = %28
  %32 = getelementptr inbounds nuw i32, ptr %20, i64 %29
  %33 = load i32, ptr %32, align 4
  %34 = getelementptr inbounds nuw i32, ptr %7, i64 %29
  %35 = load i32, ptr %34, align 4
  %36 = add i32 %33, %35
  store i32 %36, ptr %32, align 4
  %37 = add i64 %29, 1
  br label %28

38:                                               ; preds = %28
  %39 = call ptr @malloc(i64 104)
  %40 = ptrtoint ptr %39 to i64
  %41 = add i64 %40, 63
  %42 = urem i64 %41, 64
  %43 = sub i64 %41, %42
  %44 = inttoptr i64 %43 to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %44, ptr @__constant_10xi32, i64 40, i1 false)
  br label %45

45:                                               ; preds = %64, %38
  %46 = phi i64 [ %65, %64 ], [ 0, %38 ]
  %47 = icmp slt i64 %46, 10
  br i1 %47, label %48, label %66

48:                                               ; preds = %45
  br label %49

49:                                               ; preds = %52, %48
  %50 = phi i64 [ %63, %52 ], [ 0, %48 ]
  %51 = icmp slt i64 %50, 10
  br i1 %51, label %52, label %64

52:                                               ; preds = %49
  %53 = add i64 %46, %50
  %54 = urem i64 %53, 10
  %55 = getelementptr inbounds nuw i32, ptr %20, i64 %50
  %56 = load i32, ptr %55, align 4
  %57 = getelementptr inbounds nuw i32, ptr %20, i64 %46
  %58 = load i32, ptr %57, align 4
  %59 = mul i32 %58, %56
  %60 = getelementptr inbounds nuw i32, ptr %44, i64 %54
  %61 = load i32, ptr %60, align 4
  %62 = add i32 %59, %61
  store i32 %62, ptr %60, align 4
  %63 = add i64 %50, 1
  br label %49

64:                                               ; preds = %49
  %65 = add i64 %46, 1
  br label %45

66:                                               ; preds = %45
  br label %67

67:                                               ; preds = %70, %66
  %68 = phi i64 [ %76, %70 ], [ 0, %66 ]
  %69 = icmp slt i64 %68, 10
  br i1 %69, label %70, label %77

70:                                               ; preds = %67
  %71 = getelementptr inbounds nuw i32, ptr %44, i64 %68
  %72 = load i32, ptr %71, align 4
  %73 = getelementptr inbounds nuw i32, ptr %7, i64 %68
  %74 = load i32, ptr %73, align 4
  %75 = sub i32 %72, %74
  store i32 %75, ptr %71, align 4
  %76 = add i64 %68, 1
  br label %67

77:                                               ; preds = %67
  br label %78

78:                                               ; preds = %82, %77
  %79 = phi i64 [ %88, %82 ], [ 1, %77 ]
  %80 = phi i32 [ %87, %82 ], [ 0, %77 ]
  %81 = icmp slt i64 %79, 11
  br i1 %81, label %82, label %89

82:                                               ; preds = %78
  %83 = sub i64 10, %79
  %84 = mul i32 %0, %80
  %85 = getelementptr inbounds nuw i32, ptr %44, i64 %83
  %86 = load i32, ptr %85, align 4
  %87 = add i32 %84, %86
  %88 = add i64 %79, 1
  br label %78

89:                                               ; preds = %78
  call void @free(ptr %2)
  call void @free(ptr %15)
  call void @free(ptr %39)
  ret i32 %80
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #0

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
