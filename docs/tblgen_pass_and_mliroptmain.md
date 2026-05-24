# How MLIR TableGen Passes Work — from `.td` to `MlirOptMain`

## Overview

Writing an MLIR pass involves three layers:

```
Passes.td          ← you write this (pass metadata)
    ↓ mlir-tblgen
Passes.h.inc       ← auto-generated (CRTP base class, registration, docs)
    ↓ #include
AffineFullUnroll.cpp  ← you write this (actual pass logic)
    ↓
tutorial-opt       ← driver that ties everything together via MlirOptMain
```

---

## Layer 1: `Passes.td` — pass metadata

```tablegen
def AffineFullUnroll : Pass<"affine-full-unroll"> {
  let summary = "Fully unroll all affine loops";
  let description = [{ Fully unroll all affine loops. }];
  let dependentDialects = ["mlir::affine::AffineDialect"];
}
```

Key fields:
- **First arg `"affine-full-unroll"`** — the CLI flag (`--affine-full-unroll`)
- **No second arg** — pass is not anchored to a specific op type (accepts any op;
  see Layer 4 for what this means at runtime)
- **`dependentDialects`** — dialects automatically loaded into the `MLIRContext`
  before this pass runs; generated into `getDependentDialects()` in the base class
- **`summary`/`description`** — appear in `--help` output and generated `.md` docs

---

## Layer 2: `CMakeLists.txt` — invoking `mlir-tblgen`

```cmake
set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls -name Affine)
add_public_tablegen_target(MLIRAffineFullUnrollPasses)
```

- `mlir_tablegen(...)` runs `mlir-tblgen -gen-pass-decls -name Affine` on `Passes.td`
  and writes the output to `build-ninja/lib/Transform/Affine/Passes.h.inc`.
- `-name Affine` sets the group prefix: generates `registerAffinePasses()`,
  `AffineFullUnrollBase`, etc. Changing it to `-name Foo` would produce
  `registerFooPasses()`, `FooFullUnrollBase`.
- `add_public_tablegen_target(MLIRAffineFullUnrollPasses)` creates a CMake target
  that the library's `DEPENDS` clause waits on, ensuring the `.inc` is generated
  before any `.cpp` that `#include`s it is compiled.

---

## Layer 3: `Passes.h.inc` — what `mlir-tblgen` generates

The generated file is **not a standalone translation unit**. It is included
multiple times with different `#define`s active, each time exposing a different
section:

### Section A — declaration (`GEN_PASS_DECL_AFFINEFULLUNROLL`)

Activated in `AffineFullUnroll.h`:
```cpp
#define GEN_PASS_DECL_AFFINEFULLUNROLL
#include "lib/Transform/Affine/Passes.h.inc"
```
Emits:
```cpp
std::unique_ptr<::mlir::Pass> createAffineFullUnroll();
```

### Section B — definition / CRTP base (`GEN_PASS_DEF_AFFINEFULLUNROLL`)

Activated in `AffineFullUnroll.cpp`:
```cpp
#define GEN_PASS_DEF_AFFINEFULLUNROLL
#include "lib/Transform/Affine/Passes.h.inc"
```
Emits (inside `namespace impl`):
```cpp
template <typename DerivedT>
class AffineFullUnrollBase : public ::mlir::OperationPass<> {
  static constexpr StringLiteral getArgumentName() { return "affine-full-unroll"; }
  StringRef getDescription() const override { return "Fully unroll all affine loops"; }
  void getDependentDialects(DialectRegistry &r) const override {
    r.insert<mlir::affine::AffineDialect>();
  }
  // ... clone, classof, TypeID, etc.
  friend std::unique_ptr<Pass> createAffineFullUnroll() {
    return std::make_unique<DerivedT>();   // ← instantiates your concrete struct
  }
};
```

### Section C — registration (`GEN_PASS_REGISTRATION`)

Activated in `Passes.h`:
```cpp
#define GEN_PASS_REGISTRATION
#include "lib/Transform/Affine/Passes.h.inc"
```
Emits:
```cpp
inline void registerAffineFullUnroll() {
  ::mlir::registerPass([]() -> std::unique_ptr<::mlir::Pass> {
    return createAffineFullUnroll();
  });
}
inline void registerAffinePasses() {   // ← registers all passes in the group
  registerAffineFullUnroll();
  registerAffineFullUnrollPatternRewrite();
}
```

### Section D — deprecated (`GEN_PASS_CLASSES`)

Also emitted by `mlir-tblgen` for backwards compatibility with pre-2022 code that
used a single `#define GEN_PASS_CLASSES` to get all base classes at once (not in
`namespace impl`). Ignore it — this project uses Sections B and C.

---

## Layer 4: Your concrete pass struct

```cpp
// AffineFullUnroll.cpp
#define GEN_PASS_DEF_AFFINEFULLUNROLL
#include "lib/Transform/Affine/Passes.h.inc"   // pulls in AffineFullUnrollBase

struct AffineFullUnroll : impl::AffineFullUnrollBase<AffineFullUnroll> {
  using AffineFullUnrollBase::AffineFullUnrollBase;  // inherit constructors

  void runOnOperation() {          // ← the ONE method you must implement
    getOperation()->walk([&](AffineForOp op) {
      if (failed(loopUnrollFull(op))) {
        op.emitError("unrolling failed");
        signalPassFailure();
      }
    });
  }
};
```

The base class provides everything else via CRTP:
`getArgument()`, `getName()`, `clonePass()`, `getDependentDialects()`, `classof()`, etc.

---

## Layer 5: `tutorial-opt.cpp` and `MlirOptMain`

```cpp
int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  // ...
  mlir::tutorial::registerAffinePasses();   // stores factory lambdas in global map
  // ...
  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Tutorial Pass Driver", registry));
}
```

`MlirOptMain` does the following:
1. **Parses `argv`** using LLVM's `cl::` library for flags like `--affine-full-unroll`
2. **Parses the input `.mlir` file** into `OwningOpRef<ModuleOp>` — the root is
   always a `ModuleOp` (hardcoded in `MlirOptMain`)
3. **Builds a `PassManager`** anchored to that `ModuleOp`
4. **Looks up `"affine-full-unroll"`** in the global registry → calls the factory
   lambda → `createAffineFullUnroll()` → `new AffineFullUnroll{}`
5. **Runs the pass** on the `ModuleOp`
6. **Prints the resulting IR** to stdout

`asMainReturnCode` is just:
```cpp
inline int asMainReturnCode(LogicalResult r) {
  return r.succeeded() ? EXIT_SUCCESS : EXIT_FAILURE;
}
```

---

## What `getOperation()` returns

Because `Passes.td` specifies no op type (`Pass<"affine-full-unroll">` with no
second argument), the pass is an "any op" pass — it accepts whatever op the pass
manager is anchored to. `MlirOptMain` always anchors the top-level pass manager to
a `ModuleOp`, so `getOperation()` returns the `ModuleOp` for the whole file.

That is why `runOnOperation()` uses `walk()` — it starts from the module and
recursively descends through all functions to find every `AffineForOp`.

To anchor a pass to a specific op type, specify it in `Passes.td`:
```tablegen
def MyFuncPass : Pass<"my-pass", "func::FuncOp"> { ... }
//                                ^^^^^^^^^^^^^^ anchored — getOperation() returns FuncOp
```

---

## End-to-end data flow summary

```
tutorial-opt --affine-full-unroll input.mlir
    │
    ├─ main() calls registerAffinePasses()
    │     └─ registerPass(lambda)  →  stored in global map under "affine-full-unroll"
    │
    └─ MlirOptMain(argc, argv, ..., registry)
          ├─ parse input.mlir → ModuleOp
          ├─ parse "--affine-full-unroll" → look up in registry → call lambda
          │       └─ createAffineFullUnroll() → new AffineFullUnroll{}
          ├─ PassManager::run(ModuleOp)
          │       └─ AffineFullUnroll::runOnOperation()
          │               getOperation()  →  ModuleOp
          │               walk(AffineForOp)  →  loopUnrollFull(op)
          └─ print transformed IR
```
