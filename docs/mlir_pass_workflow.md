# MLIR Pass Creation Workflow

This document describes the end-to-end workflow for creating and adding a new MLIR pass in this project, using `OptimizePolyConst` as a concrete example.

## Overview

Creating an MLIR pass involves four layers:

```
  Passes.td       — TableGen declaration
       ↓
  mlir-tblgen     — generates Passes.h.inc
       ↓
  Passes.h.inc    — macro-guarded declarations & definitions
       ↓
  YourPass.h      — include .inc with GEN_PASS_DECL_*
  YourPass.cpp    — include .inc with GEN_PASS_DEF_*, implement patterns & pass
```

## Step-by-Step

### 1. Define the Pass in TableGen

**File:** `lib/Transform/<YourDir>/Passes.td`

```tablegen
#ifndef LIB_TRANSFORM_<YOURDIR>_PASSES_TD_
#define LIB_TRANSFORM_<YOURDIR>_PASSES_TD_

include "mlir/Pass/PassBase.td"

def OptimizePolyConst : Pass<"optimize-poly-const"> {
  let summary = "Optimize poly constant operations";
  let description = [{
    Optimize poly constant operations.
  }];
}

#endif
```

- `def OptimizePolyConst` — the C++ **class name** (TableGen uppercases it for macros)
- `Pass<"optimize-poly-const">` — the **CLI argument** (used with `--pass-name` on command line)

### 2. Add TableGen Build Rule

**File:** `lib/Transform/<YourDir>/CMakeLists.txt`

```cmake
# Tell LLVM's tablegen infrastructure which .td file to process.
set(LLVM_TARGET_DEFINITIONS Passes.td)
# Invoke mlir-tblgen with the pass-decls backend to generate Passes.h.inc.
# The -name <Prefix> flag sets the namespace for generated macros.
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Poly)
# Bundle the tablegen output into a custom target for dependency tracking.
add_public_tablegen_target(MLIR<YourDir>TransformPasses)
```

**Generated macro names** follow the pattern:

| .td `def` name | Macro |
|----------------|-------|
| `OptimizePolyConst` | `GEN_PASS_DECL_OPTIMIZEPOLYCONST` |
| `OptimizePolyConst` | `GEN_PASS_DEF_OPTIMIZEPOLYCONST` |
| `OptimizePolyConst` | `GEN_PASS_REGISTRATION` |

The naming is: `GEN_PASS_` + `DECL`/`DEF`/`REGISTRATION` + `_` + **uppercase of the def name**.

### 3. Generate the .inc File Manually (Optional)

```bash
mlir-tblgen -gen-pass-decls -name Poly \
  -I /path/to/llvm-project/mlir/include \
  -I /path/to/your/project \
  lib/Transform/Poly/Passes.td
```

This prints the generated file to stdout. Redirect to a file for inspection:

```bash
mlir-tblgen ... > Passes.h.inc
```

### 4. Create the Pass Header

**File:** `lib/Transform/<YourDir>/YourPass.h`

```cpp
#ifndef LIB_TRANSFORM_<YOURDIR>_YOURPASS_H_
#define LIB_TRANSFORM_<YOURDIR>_YOURPASS_H_

#include "mlir/Pass/Pass.h"

namespace mlir {
namespace tutorial {

#define GEN_PASS_DECL_OPTIMIZEPOLYCONST
#include "lib/Transform/<YourDir>/Passes.h.inc"

}  // namespace tutorial
}  // namespace mlir

#endif
```

`GEN_PASS_DECL_OPTIMIZEPOLYCONST` expands to:

```cpp
std::unique_ptr<::mlir::Pass> createOptimizePolyConst();
```

### 5. Create the Registration Header

**File:** `lib/Transform/<YourDir>/Passes.h`

```cpp
#ifndef LIB_TRANSFORM_<YOURDIR>_PASSES_H_
#define LIB_TRANSFORM_<YOURDIR>_PASSES_H_

#include "lib/Transform/<YourDir>/YourPass.h"

namespace mlir {
namespace tutorial {
namespace <yourdir> {    // optional — consistent with project convention

#define GEN_PASS_REGISTRATION
#include "lib/Transform/<YourDir>/Passes.h.inc"

}  // namespace <yourdir>
}  // namespace tutorial
}  // namespace mlir

#endif
```

`GEN_PASS_REGISTRATION` generates registration functions like `registerOptimizePolyConst()` and `register<Prefix>Passes()`.

### 6. Implement the Pass

**File:** `lib/Transform/<YourDir>/YourPass.cpp`

#### 6a. Include headers and define the pass macro

```cpp
#include "lib/Dialect/<YourDialect>/<YourDialect>Ops.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace tutorial {

#define GEN_PASS_DEF_OPTIMIZEPOLYCONST
#include "lib/Transform/<YourDir>/Passes.h.inc"
```

`GEN_PASS_DEF_OPTIMIZEPOLYCONST` expands to:
- A forward declaration: `impl::createOptimizePolyConst()`
- A CRTP base class: `impl::OptimizePolyConstBase<DerivedT>`
- The public function: `createOptimizePolyConst()`

#### 6b. Define rewrite patterns

```cpp
using poly::ConstantOp;
using poly::MulOp;

struct MultiplyConstOne : public OpRewritePattern<MulOp> {
  MultiplyConstOne(mlir::MLIRContext *context)
      : OpRewritePattern<MulOp>(context, /*benefit=*/2) {}

  LogicalResult matchAndRewrite(MulOp op,
                                PatternRewriter &rewriter) const override {
    // ... matching & rewriting logic ...
    return success();
  }
};
```

#### 6c. Define the pass class (CRTP)

```cpp
struct OptimizePolyConst
    : impl::OptimizePolyConstBase<OptimizePolyConst> {
  using OptimizePolyConstBase::OptimizePolyConstBase;

  void runOnOperation() {
    mlir::RewritePatternSet patterns(&getContext());
    patterns.add<MultiplyConstOne>(&getContext());
    patterns.add<AddConstZero>(&getContext());
    (void)applyPatternsGreedily(getOperation(), std::move(patterns));
  }
};

}  // namespace tutorial
}  // namespace mlir
```

**Why CRTP is required**: The generated `OptimizePolyConstBase<DerivedT>` has a `friend` function:

```cpp
friend std::unique_ptr<::mlir::Pass> createOptimizePolyConst() {
    return std::make_unique<DerivedT>();
}
```

This provides the definition of `impl::createOptimizePolyConst()`, but only when `DerivedT` is instantiated. Without the `struct OptimizePolyConst : impl::OptimizePolyConstBase<OptimizePolyConst>` line, you get an **undefined reference** linker error.

### 7. Add Library Build Rule

**File:** `lib/Transform/<YourDir>/CMakeLists.txt`

```cmake
add_mlir_library(<YourDir>Transform
    YourPass.cpp

    ${PROJECT_SOURCE_DIR}/lib/Transform/<YourDir>/
    ADDITIONAL_HEADER_DIRS

    DEPENDS
    MLIR<YourDialect>            # dialect library
    MLIR<YourDir>TransformPasses # tablegen target

    LINK_LIBS PUBLIC
    MLIR<YourDialect>            # dialect ops/types
)
```

### 8. Register the Subdirectory

**File:** `lib/Transform/CMakeLists.txt`

```cmake
add_subdirectory(<YourDir>)
```

### 9. Link into tutorial-opt

**File:** `tools/CMakeLists.txt`

```cmake
set (LIBS
    ...
    <YourDir>Transform     # add to the list
)
```

### 10. Register Passes in tutorial-opt

**File:** `tools/tutorial-opt.cpp`

```cpp
#include "lib/Transform/<YourDir>/Passes.h"

// In main():
mlir::tutorial::<yourdir>::register<YourDir>Passes();
```

## Namespace Convention

The registration function's namespace depends on how `GEN_PASS_REGISTRATION` is wrapped in `Passes.h`:

```cpp
// If wrapped in an extra namespace:
namespace mlir::tutorial::poly {
  #define GEN_PASS_REGISTRATION
  #include ...
}
// → call: mlir::tutorial::poly::registerPolyPasses();

// If only in tutorial:
namespace mlir::tutorial {
  #define GEN_PASS_REGISTRATION
  #include ...
}
// → call: mlir::tutorial::registerPolyPasses();
```

Check your `Passes.h` to determine the correct namespace.

## Generated File Structure

```
Passes.h.inc
├── #ifdef GEN_PASS_DECL
│   ├── #define GEN_PASS_DECL_OPTIMIZEPOLYCONST
│   └── #undef GEN_PASS_DECL
├── #ifdef GEN_PASS_DECL_OPTIMIZEPOLYCONST
│   └── std::unique_ptr<Pass> createOptimizePolyConst();
├── #ifdef GEN_PASS_DEF_OPTIMIZEPOLYCONST
│   ├── namespace impl { createOptimizePolyConst(); }
│   ├── template<DerivedT> class OptimizePolyConstBase { ... }
│   └── createOptimizePolyConst() { return impl::createOptimizePolyConst(); }
├── #ifdef GEN_PASS_REGISTRATION
│   └── registerOptimizePolyConst(), registerPolyPasses()
└── #ifdef GEN_PASS_CLASSES (deprecated)
    └── template<DerivedT> class OptimizePolyConstBase { ... }
```
that provides all pass boilerplate: `getArgument`, `getName`, `clonePass`,
`getDependentDialects`, etc. The only method you **must** implement in your
concrete struct is:

```cpp
void runOnOperation() override;
```

The `using XxxBase::XxxBase;` line in the struct inherits the constructors,
so no constructor needs to be written either.

---

## Why the normal VS Code / clangd approach doesn't work

The standard IDE tricks for finding missing overrides are:

| Approach | How it normally works |
|---|---|
| Hover on the struct name | clangd shows *"struct Foo is abstract, pure virtual 'runOnOperation' not overridden"* |
| `Ctrl+.` code action on struct | clangd offers *"Implement all pure virtual methods"* and generates stubs |
| `F12` on base class | Jump to base class definition, search for `= 0` |
| Problems panel (`Ctrl+Shift+M`) | Lists all errors in the workspace |

**None of these work here**, because of the MLIR tablegen CRTP pattern:

- The concrete struct (e.g. `AffineFullUnroll`) is defined in your `.cpp`.
- The pass instantiation (`std::make_unique<DerivedT>()`) lives inside the
  **generated** `Passes.h.inc` file.
- When `runOnOperation` is missing, the compiler error (`allocation_of_abstract_type`)
  fires **inside `Passes.h.inc`**, not in your `.cpp`.
- clangd in LSP mode reports diagnostics in the file where they occur — so the
  squiggle appears in `Passes.h.inc`, not in your `.cpp`.
- Result: no squiggle, no code action, no hover error in the `.cpp` editor.

---

## Option 1 — Open the generated `.inc` file

Open `build-ninja/lib/Transform/Affine/Passes.h.inc` in VS Code.
The squiggle appears on `std::make_unique<DerivedT>()` (the error site),
with a secondary annotation on `createAffineFullUnroll()` (the instantiation point).

Downside: the file is auto-generated and not pleasant to read.

---

## Option 2 — VS Code task: "syntax-check current file"

Run **`Ctrl+Shift+P` → Tasks: Run Task → "syntax-check current file"**
while the `.cpp` is the active editor.

This uses `.vscode/syntax_check.py`, which:
1. Reads `build-ninja/compile_commands.json` to get the exact compiler flags for the file.
2. Replaces `-c` with `-fsyntax-only` (no object file produced — check only).
3. Strips `-o <output>` (irrelevant without `-c`).
4. Runs the compiler, producing the full diagnostic chain:

```
error: allocating an object of abstract class type 'mlir::tutorial::AffineFullUnroll'
note: unimplemented pure virtual method 'runOnOperation' in 'AffineFullUnroll'
```

The `$gcc` problem matcher in `tasks.json` also populates the Problems panel.

---

## Option 3 — `clangd --check` from terminal

```bash
CLANGD=~/.vscode-server/data/User/globalStorage/llvm-vs-code-extensions.vscode-clangd/install/19.1.2/clangd_19.1.2/bin/clangd
$CLANGD --check=lib/Transform/Affine/AffineFullUnroll.cpp 2>&1 | grep -v "^I\["
```

clangd detects the error correctly, but its `--check` mode only logs the
top-level diagnostic — it omits the `note:` lines that name the missing method.
**Option 2 gives a clearer message.**

---

## Summary

| Option | Tells you the missing method by name? | Notes |
|---|---|---|
| Hover / code action in `.cpp` | No | Error is in included `.inc`, not `.cpp` |
| Open `Passes.h.inc` in editor | Yes (squiggle) | Generated file, noisy |
| Task: syntax-check current file | **Yes** (full note) | Recommended |
| `clangd --check` in terminal | Partial (no note lines) | |
