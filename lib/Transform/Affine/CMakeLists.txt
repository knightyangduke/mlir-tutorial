# Define a new MLIR library target named "AffineFullUnroll".
# This macro is provided by the MLIR CMake infrastructure and handles
# registration, installation, and linking boilerplate automatically.
add_mlir_library(AffineFullUnroll
    # Source file implementing the full loop unrolling pass using
    # the explicit walk-based approach.
    AffineFullUnroll.cpp
    # Source file implementing the same pass using the pattern-rewrite
    # (GreedyPatternRewriteDriver) approach.
    AffineFullUnrollPatternRewrite.cpp

    # Tell CMake where to look for additional (non-generated) header files
    # that belong to this library. Using PROJECT_SOURCE_DIR makes the path
    # absolute and independent of the build directory.
    ${PROJECT_SOURCE_DIR}/lib/Transform/Affine/
    ADDITIONAL_HEADER_DIRS

    # List build targets that must be completed before this library is built.
    # Here we wait for the tablegen target so that Passes.h.inc is generated
    # before the C++ sources that #include it are compiled.
    DEPENDS
    MLIRAffineFullUnrollPasses

    # Libraries to link into this target (with PUBLIC visibility so that
    # consumers of AffineFullUnroll also transitively see these dependencies).
    # Currently empty — add MLIR/LLVM libs here as needed (e.g. MLIRAffineDialect).
    LINK_LIBS PUBLIC
)

# Tell LLVM's tablegen infrastructure which .td (TableGen definition) file
# contains the pass declarations to process next.
set(LLVM_TARGET_DEFINITIONS Passes.td)

# Run mlir-tblgen on Passes.td to emit C++ pass declaration boilerplate into
# Passes.h.inc. The "-name Affine" argument sets the pass pipeline name/prefix
# used in the generated registration and option-parsing code. For example,
# the generated Passes.h.inc contains:
#
#   // A single function that registers every pass in this group:
#   inline void registerAffinePasses() { ... }
#
#   // A CRTP base class for each pass, named after the group:
#   class AffineFullUnrollBase : public ::mlir::OperationPass<> { ... };
#
# If "-name Affine" were changed to "-name Foo", these would become
# registerFooPasses() and FooFullUnrollBase instead.
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Affine)

# Create a CMake target called "MLIRAffineFullUnrollPasses" that other targets
# can depend on (via DEPENDS above) to ensure the .inc file is generated first.
add_public_tablegen_target(MLIRAffineFullUnrollPasses)

# Generate Markdown documentation for the passes declared in Passes.td and
# write it to AffinePasses.md in the current doc output directory ("./").
add_mlir_doc(Passes AffinePasses ./ -gen-pass-doc)
