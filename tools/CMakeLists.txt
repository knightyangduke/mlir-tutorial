get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)

set (LIBS
    ${dialect_libs}
    ${conversion_libs}
    AffineFullUnroll
    MLIRNoisy
    MLIROptLib
    MLIRPass
    MLIRPoly
    MulToAdd
    NoisyPasses
    PolyToStandard
    ortools::ortools
)

add_llvm_executable(tutorial-opt PARTIAL_SOURCES_INTENDED tutorial-opt.cpp)

llvm_update_compile_flags(tutorial-opt)
target_link_libraries(tutorial-opt PRIVATE ${LIBS})

mlir_check_all_link_libraries(tutorial-opt)

# --- LSP Server ---
set(LSP_LIBS
    ${dialect_libs}
    MLIRLspServerLib
    MLIRPoly
    MLIRNoisy
)

add_llvm_executable(tutorial-lsp-server PARTIAL_SOURCES_INTENDED tutorial-lsp-server.cpp)

llvm_update_compile_flags(tutorial-lsp-server)
target_link_libraries(tutorial-lsp-server PRIVATE ${LSP_LIBS})

mlir_check_all_link_libraries(tutorial-lsp-server)
