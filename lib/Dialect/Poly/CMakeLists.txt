# Inlining `add_mlir_dialect(Poly poly)` commands so that
# we can custom name `*.inc` generated files.

# --- TableGen: Op, Type, and Dialect code generation ---
# PolyOps.td is the single entry point; it transitively includes
# PolyDialect.td and PolyTypes.td, so one LLVM_TARGET_DEFINITIONS suffices.
# All mlir_tablegen calls below implicitly use this file as input.
set(LLVM_TARGET_DEFINITIONS PolyOps.td)
mlir_tablegen(PolyOps.h.inc     -gen-op-decls)                             # Op class declarations
mlir_tablegen(PolyOps.cpp.inc   -gen-op-defs)                              # Op method bodies
mlir_tablegen(PolyTypes.h.inc   -gen-typedef-decls -typedefs-dialect=poly) # Type declarations  (-typedefs-dialect filters out MLIR built-in types)
mlir_tablegen(PolyTypes.cpp.inc -gen-typedef-defs  -typedefs-dialect=poly) # Type method bodies
mlir_tablegen(PolyDialect.h.inc   -gen-dialect-decls -dialect=poly)        # Dialect class declaration
mlir_tablegen(PolyDialect.cpp.inc -gen-dialect-defs  -dialect=poly)        # Dialect method bodies
# Bundle the above .inc outputs into one target; also appends to LLVM_COMMON_DEPENDS
# so add_mlir_dialect_library below automatically depends on it.
add_public_tablegen_target(MLIRPolyOpsIncGen)
# mlir-headers is the global MLIR umbrella target meaning "all generated headers
# are ready".  Adding MLIRPolyOpsIncGen as a dependency means mlir-headers
# depends on MLIRPolyOpsIncGen, so MLIRPolyOpsIncGen must be built FIRST.
# Without this, building mlir-opt or other targets that depend on mlir-headers
# could fail because Poly's .inc files wouldn't exist yet.
add_dependencies(mlir-headers MLIRPolyOpsIncGen)

add_mlir_doc(PolyDialect PolyDialect Poly/ -gen-dialect-doc)

# --- TableGen: DRR canonicalization patterns ---
set(LLVM_TARGET_DEFINITIONS PolyPatterns.td)
mlir_tablegen(PolyCanonicalize.cpp.inc -gen-rewriters) # C++ rewrite pattern boilerplate
add_public_tablegen_target(MLIRPolyCanonicalizationIncGen) # also appended to LLVM_COMMON_DEPENDS

# --- C++ library ---
# Compiles PolyDialect.cpp, PolyOps.cpp, and PolyReductionPatterns.cpp
# into the MLIRPoly library.
# Automatically links MLIR core libs and depends on tablegen targets
# above via LLVM_COMMON_DEPENDS (no manual add_dependencies needed).
add_mlir_dialect_library(MLIRPoly
    PolyDialect.cpp
    PolyOps.cpp
    PolyReductionPatterns.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/lib/Dialect/Poly  # allows #include "PolyOps.h" etc.
    ${PROJECT_SOURCE_DIR}/lib/Utility/

    LINK_LIBS PUBLIC
    MLIRReduceLib
    )
