# Collect all MLIR dialect & conversion libraries registered globally by MLIR's
# add_mlir_dialect_library / add_mlir_conversion_library macros.
# These include the core dialects (Arith, Func, Tensor, Complex, etc.) that
# our code depends on transitively.
get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)

# ---- tutorial-opt: the main pass-driver binary, like mlir-opt ----
set (LIBS
    ${dialect_libs}
    ${conversion_libs}
    AffineFullUnroll          # our Affine full-unroll pass
    MLIRNoisy                 # the Noisy dialect library
    MLIROptLib                # MLIR's MlirOptMain entry point
    MLIRPass                  # MLIR pass infrastructure
    MLIRPoly                  # the Poly dialect library
    MulToAdd                  # our MulToAdd pass
    NoisyPasses               # passes for the Noisy dialect
    PolyToStandard            # Poly → Standard lowering
    ortools::ortools          # Google OR-Tools (used by some passes)
)

# Build the tutorial-opt executable from a single source file.
# PARTIAL_SOURCES_INTENDED is required because this directory has multiple
# targets (tutorial-opt + tutorial-lsp-server) that each use different sources.
add_llvm_executable(tutorial-opt PARTIAL_SOURCES_INTENDED tutorial-opt.cpp)

# Propagate LLVM compile flags (warnings, optimization level, etc.) to this target.
llvm_update_compile_flags(tutorial-opt)
# Link all the dialect, conversion, and pass libraries listed above.
target_link_libraries(tutorial-opt PRIVATE ${LIBS})

# Verify that all linked libraries are actually used (no dead references).
mlir_check_all_link_libraries(tutorial-opt)

# ---- tutorial-lsp-server: LSP language server for .mlir files ----
# Provides hover, completion, go-to-definition (SSA values/symbols),
# and find-references for the Poly and Noisy dialects.
set(LSP_LIBS
    ${dialect_libs}            # all MLIR core dialects so the server can parse them
    MLIRLspServerLib           # MLIR's LSP server library (MlirLspServerMain)
    MLIRPoly                    # Poly dialect (operations, types, parsing)
    MLIRNoisy                   # Noisy dialect (operations, types, parsing)
)

add_llvm_executable(tutorial-lsp-server PARTIAL_SOURCES_INTENDED tutorial-lsp-server.cpp)

llvm_update_compile_flags(tutorial-lsp-server)
target_link_libraries(tutorial-lsp-server PRIVATE ${LSP_LIBS})

mlir_check_all_link_libraries(tutorial-lsp-server)
