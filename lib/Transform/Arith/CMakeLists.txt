add_mlir_pdll_library(MulToAddPdllIncGen
  MulToAdd.pdll
  MulToAddPdll.h.inc
)

add_mlir_library(MulToAdd
    ArithPatterns.cpp
    MulToAdd.cpp
    MulToAddPdll.cpp
    MulToShift.cpp

    ${PROJECT_SOURCE_DIR}/lib/Transform/Arith/
    ${PROJECT_SOURCE_DIR}/lib/Utility/
    ADDITIONAL_HEADER_DIRS

    DEPENDS
    MLIRMulToAddPasses
    MulToAddPdllIncGen

    LINK_LIBS PUBLIC
)

set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Arith)
add_public_tablegen_target(MLIRMulToAddPasses)
add_mlir_doc(Passes ArithPasses ./ -gen-pass-doc)
