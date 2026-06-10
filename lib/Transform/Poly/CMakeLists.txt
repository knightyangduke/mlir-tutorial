# Define a new MLIR library target named "PolyTransform".
# This macro is provided by the MLIR CMake infrastructure and handles
# registration, installation, and linking boilerplate automatically.
add_mlir_library(PolyTransform
    # Source file implementing the poly constant folding pass.
    PolyConstOptimize.cpp

    # Tell CMake where to look for additional (non-generated) header files
    # that belong to this library. Using PROJECT_SOURCE_DIR makes the path
    # absolute and independent of the build directory.
    ${PROJECT_SOURCE_DIR}/lib/Transform/Poly/
    ADDITIONAL_HEADER_DIRS

    # List build targets that must be completed before this library is built.
    # Here we wait for:
    #   MLIRPoly              — the Poly dialect library (for PolyOps etc.)
    #   MLIRPolyTransformPasses — tablegen target so Passes.h.inc is generated
    #                             before the C++ sources that #include it compile.
    DEPENDS
    MLIRPoly
    MLIRPolyTransformPasses

    # Libraries to link into this target (with PUBLIC visibility so that
    # consumers of PolyTransform also transitively see these dependencies).
    LINK_LIBS PUBLIC
    MLIRPoly                    # Poly dialect ops/types
)

# Tell LLVM's tablegen infrastructure which .td (TableGen definition) file
# will be processed. This variable is read by mlir_tablegen() below.
set(LLVM_TARGET_DEFINITIONS Passes.td)
# Invoke mlir-tblgen with the pass-decls backend to generate Passes.h.inc.
# The -name Poly flag sets the namespace prefix for the generated macros
# (e.g. GEN_PASS_DECL_OPTIMIZEPOLYCONST, GEN_PASS_DEF_OPTIMIZEPOLYCONST).
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Poly)
# Bundle the tablegen output into a custom target so that DEPENDS above can
# reference it. This target is also automatically added to LLVM_COMMON_DEPENDS,
# meaning any target that depends on MLIR headers will wait for this gen step.
add_public_tablegen_target(MLIRPolyTransformPasses)
