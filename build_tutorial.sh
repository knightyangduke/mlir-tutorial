#Note this will download and build or-tools, which is very slow
#just run the final build command if you have already built or-tools
#cmake --build build-ninja --target MLIRAffineFullUnrollPasses MLIRMulToAddPasses MLIRNoisyPasses mlir-headers mlir-doc tutorial-opt check-mlir-tutorial
BUILD_SYSTEM="Ninja"
BUILD_DIR=./build-`echo ${BUILD_SYSTEM}| tr '[:upper:]' '[:lower:]'`

rm -rf $BUILD_DIR
mkdir $BUILD_DIR
pushd $BUILD_DIR

LLVM_BUILD_DIR=/home/yeyang/llvm-project/build-release
cmake -G $BUILD_SYSTEM .. \
    -DLLVM_DIR="$LLVM_BUILD_DIR/lib/cmake/llvm" \
    -DMLIR_DIR="$LLVM_BUILD_DIR/lib/cmake/mlir" \
    -DBUILD_DEPS="ON" \
    -DBUILD_SHARED_LIBS="OFF" \
    -DCMAKE_BUILD_TYPE=Debug

popd

cmake --build $BUILD_DIR --target MLIRAffineFullUnrollPasses
cmake --build $BUILD_DIR --target MLIRMulToAddPasses
cmake --build $BUILD_DIR --target MLIRNoisyPasses
cmake --build $BUILD_DIR --target mlir-headers
cmake --build $BUILD_DIR --target mlir-doc
cmake --build $BUILD_DIR --target tutorial-opt
cmake --build $BUILD_DIR --target check-mlir-tutorial