# Collect all registered MLIR dialect and conversion libraries.
# These are populated by add_mlir_dialect_library() and add_mlir_conversion_library()
# throughout the project.
get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)

# --- tutorial-opt: the main MLIR test driver ---
# Links together all dialect, conversion, and transform libraries so that
# tutorial-opt can parse, optimize, and lower any dialect used in this project.
set (LIBS
    ${dialect_libs}              # All MLIR dialects (including Poly, Noisy, etc.)
    ${conversion_libs}           # All MLIR conversion passes
    AffineFullUnroll             # Affine full-unrolling transform pass
    MLIRNoisy                    # Noisy dialect library
    MLIROptLib                   # Core MLIR-opt support library
    MLIRPass                     # MLIR pass infrastructure
    MLIRPoly                     # Poly dialect library
    MulToAdd                     # Arith Mul→Add transform pass
    NoisyPasses                  # Noisy transform passes (ReduceNoiseChecker, etc.)
    PolyToStandard               # Poly → Standard conversion pass
    PolyTransform                # Poly transform passes (MultiplyConstOne, AddConstZero, etc.)
    ortools::ortools             # OR-Tools (used by Noisy dialect)
)

# Build the tutorial-opt executable from tutorial-opt.cpp.
# PARTIAL_SOURCES_INTENDED tells LLVM's build system that only a subset of
# source files are listed here; it prevents automatic source scanning.
add_llvm_executable(tutorial-opt PARTIAL_SOURCES_INTENDED
    tutorial-opt.cpp
    ${PROJECT_SOURCE_DIR}/lib/Utility/DebugHelper.cpp
)

# Apply LLVM compile flags (warnings, optimization level, etc.).
llvm_update_compile_flags(tutorial-opt)
# Disable --gc-sections so debug helper symbols survive in the binary for GDB.
target_link_options(tutorial-opt PRIVATE -Wl,--no-gc-sections)
# Link all the libraries listed above.
# PRIVATE means these libraries are linked only into tutorial-opt itself,
# and are NOT propagated to anything that depends on tutorial-opt
# (which is fine since tutorial-opt is a final executable, not a library).
target_link_libraries(tutorial-opt PRIVATE ${LIBS})

# Verify that all linked libraries actually exist.
mlir_check_all_link_libraries(tutorial-opt)

# --- tutorial-lsp-server: the MLIR language server ---
# Provides IDE features (code completion, go-to-definition, etc.) for MLIR files.
set(LSP_LIBS
    ${dialect_libs}
    MLIRLspServerLib           # MLIR's LSP server framework
    MLIRPoly                   # Poly dialect (for .mlir files using poly dialect)
    MLIRNoisy                  # Noisy dialect (for .mlir files using noisy dialect)
)

add_llvm_executable(tutorial-lsp-server PARTIAL_SOURCES_INTENDED tutorial-lsp-server.cpp)

llvm_update_compile_flags(tutorial-lsp-server)
# PRIVATE: LSP libraries are linked only into this executable, not propagated.
target_link_libraries(tutorial-lsp-server PRIVATE ${LSP_LIBS})

mlir_check_all_link_libraries(tutorial-lsp-server)
