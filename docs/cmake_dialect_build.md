# CMakeLists.txt Walkthrough: Building an MLIR Dialect

This document explains how `lib/Dialect/Poly/CMakeLists.txt` works, covering
the tablegen code-generation pipeline and the CMake library target setup.

---

## Overview

Building an MLIR dialect requires two phases:

```
.td files  ──[tablegen]──►  .inc files  ──[C++ compiler]──►  .a / .so library
```

CMake orchestrates both phases using MLIR-provided helper macros.

---

## Phase 1: TableGen Code Generation

### Step 1 — Set the input file

```cmake
set(LLVM_TARGET_DEFINITIONS PolyOps.td)
```

`LLVM_TARGET_DEFINITIONS` is a CMake variable that all subsequent
`mlir_tablegen` calls read as their implicit input file.
`PolyOps.td` is used as the single entry point because it transitively
includes the other `.td` files:

```
PolyOps.td
├── include "PolyDialect.td"     → Poly_Dialect definition
├── include "PolyTypes.td"       → Polynomial type definition
├── include "mlir/IR/OpBase.td"  → MLIR built-in base classes
└── include "mlir/IR/BuiltinAttributes.td"  → MLIR built-in attrs
```

TableGen processes the full transitive closure, so one entry point suffices.

### Step 2 — Run tablegen for each output

```cmake
mlir_tablegen(PolyOps.h.inc     -gen-op-decls)
mlir_tablegen(PolyOps.cpp.inc   -gen-op-defs)
mlir_tablegen(PolyTypes.h.inc   -gen-typedef-decls -typedefs-dialect=poly)
mlir_tablegen(PolyTypes.cpp.inc -gen-typedef-defs  -typedefs-dialect=poly)
mlir_tablegen(PolyDialect.h.inc -gen-dialect-decls -dialect=poly)
mlir_tablegen(PolyDialect.cpp.inc -gen-dialect-defs -dialect=poly)
```

Each call generates one `.inc` file. The `-gen-*` flag selects what the
tablegen backend emits:

| Output file | Flag | What is generated |
|---|---|---|
| `PolyOps.h.inc` | `-gen-op-decls` | C++ class declarations for every `Op` def |
| `PolyOps.cpp.inc` | `-gen-op-defs` | C++ method bodies for every `Op` def |
| `PolyTypes.h.inc` | `-gen-typedef-decls` | C++ class declarations for every `TypeDef` |
| `PolyTypes.cpp.inc` | `-gen-typedef-defs` | C++ method bodies for every `TypeDef` |
| `PolyDialect.h.inc` | `-gen-dialect-decls` | C++ class declaration for the dialect |
| `PolyDialect.cpp.inc` | `-gen-dialect-defs` | C++ method bodies for the dialect |

#### Why `-typedefs-dialect=poly` and `-dialect=poly` but not for ops?

`PolyOps.td` transitively includes MLIR's own `.td` files, which define
types and dialects for `builtin`, `arith`, etc.
Without the filter flag, `-gen-typedef-decls` would emit those too, causing
duplicate symbol errors.

For ops this is not an issue: MLIR's built-in ops live in completely separate
`.td` files that `PolyOps.td` never includes, so nothing to filter.
Adding `-dialect=poly` to op generators is harmless but unnecessary.

### Step 3 — Bundle into a named target

```cmake
add_public_tablegen_target(MLIRPolyOpsIncGen)
```

This macro:
1. Collects all `.inc` files registered by the preceding `mlir_tablegen` calls.
2. Creates a CMake target `MLIRPolyOpsIncGen` that, when built, runs all
   the registered tablegen commands.
3. Appends `MLIRPolyOpsIncGen` to `LLVM_COMMON_DEPENDS` (a directory-scoped
   accumulator variable), so downstream library targets automatically depend
   on it.

```cmake
add_dependencies(mlir-headers MLIRPolyOpsIncGen)
```

`mlir-headers` is a global MLIR umbrella target meaning "all generated headers
are ready".

**Important: dependency direction.** In CMake, `add_dependencies(A B)` means
**A depends on B**, so **B must be built before A**. Here that means
`MLIRPolyOpsIncGen` must finish generating all `.inc` files **before**
`mlir-headers` is considered complete.

Why is this necessary? MLIR's top-level targets (like `mlir-opt`,
`libMLIR.so`, etc.) commonly depend on `mlir-headers` so that all generated
headers are available before compilation starts. Without this line,
`mlir-headers` wouldn't know about the Poly dialect, and building `mlir-opt`
could fail with missing `PolyOps.h.inc` or similar errors — because the Poly
headers would never be generated.

---

## Phase 1b: Canonicalization Pattern TableGen

```cmake
set(LLVM_TARGET_DEFINITIONS PolyPatterns.td)
mlir_tablegen(PolyCanonicalize.cpp.inc -gen-rewriters)
add_public_tablegen_target(MLIRPolyCanonicalizationIncGen)
```

Same mechanism as above, but with a different input file (`PolyPatterns.td`)
and a different backend (`-gen-rewriters`), which generates C++ rewrite pattern
boilerplate from DRR (Declarative Rewrite Rule) definitions.
`MLIRPolyCanonicalizationIncGen` is also appended to `LLVM_COMMON_DEPENDS`.

---

## Phase 2: C++ Library

```cmake
add_mlir_dialect_library(MLIRPoly
    PolyDialect.cpp
    PolyOps.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/lib/Dialect/Poly

    LINK_LIBS PUBLIC
    )
```

`add_mlir_dialect_library` is an MLIR wrapper around CMake's `add_library`.
It handles several things automatically:

| What | How |
|---|---|
| Links MLIR core libraries | Wired in by the macro (no need to list IR, Support, etc.) |
| Depends on tablegen targets | Reads `LLVM_COMMON_DEPENDS`, which contains both `MLIRPolyOpsIncGen` and `MLIRPolyCanonicalizationIncGen` at this point |
| Adds include path | `ADDITIONAL_HEADER_DIRS` adds `lib/Dialect/Poly/` so `#include "PolyOps.h"` resolves |
| Extra link dependencies | `LINK_LIBS PUBLIC` is empty here — no extra libs needed |

The `LLVM_COMMON_DEPENDS` dependency wiring means you never need to write
`add_dependencies(MLIRPoly MLIRPolyOpsIncGen)` manually — the macro does it.

---

## Full Build Dependency Graph

```
Legend:
  ──►  "generates / produces"  (the arrow points from source to output)
  ═══► "depends on"            (the arrow points from consumer to dependency;
                                 dependency must be built first)

Generation (tablegen runs):
  PolyOps.td ──[tablegen × 6]──► MLIRPolyOpsIncGen
  PolyPatterns.td ──[tablegen]──► MLIRPolyCanonicalizationIncGen

Dependency (build order — built bottom-up):
  MLIRPolyOpsIncGen ◄═══ mlir-headers              # mlir-headers depends on IncGen
  MLIRPolyOpsIncGen ◄═══ MLIRPoly                  # via LLVM_COMMON_DEPENDS
  MLIRPolyCanonicalizationIncGen ◄═══ MLIRPoly     # via LLVM_COMMON_DEPENDS
  PolyDialect.cpp, PolyOps.cpp ◄═══ MLIRPoly       # compiled into the library

Equivalent tree view (build order: leaf → root):
  MLIRPolyOpsIncGen   MLIRPolyCanonicalizationIncGen   PolyDialect.cpp
         ║                        ║                    PolyOps.cpp
         ║                        ║                         ║
         ╚════════════╗  ╔════════╝                         ║
                      ▼  ▼                                  ▼
                   MLIRPoly ◄═══════════════════════════════╝
                      ║
                      ║  (via add_dependencies)
                      ║
               mlir-headers
```

---

## How `.inc` Files Are Used in C++

The generated files are `#include`d directly inside the `.cpp` source files
at specific guarded locations, e.g.:

```cpp
// PolyDialect.cpp
#define GET_TYPEDEF_CLASSES
#include "PolyTypes.cpp.inc"

#define GET_DIALECT_CLASSES
#include "PolyDialect.cpp.inc"

// PolyOps.cpp
#define GET_OP_CLASSES
#include "PolyOps.cpp.inc"
#include "PolyCanonicalize.cpp.inc"
```

The `#define` guards tell each `.inc` file which section to emit, since a
single `.inc` may contain multiple guarded sections.
