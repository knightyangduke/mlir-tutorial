# Using mlir-reduce with Custom Dialect Reduction Patterns

This guide covers how to wire up `DialectReductionPatternInterface` for a custom
dialect and how to use `mlir-reduce` in different modes.

---

## 1. Architecture Overview

`mlir-reduce` shrinks MLIR input while preserving an "interestingness" condition.
It uses three strategies:

```
mlir-reduce input.mlir
    │
    ├── --reduction-tree        (comprehensive: all three strategies)
    │     ├── Operation elimination (built-in, always on)
    │     ├── Dialect rewrite patterns (from DialectReductionPatternInterface)
    │     └── Built-in opt passes (symbol-dce, etc.)
    │
    └── --opt-reduction-pass    (runs ONLY specified optimization passes)
          └── --opt-pass=<string>  e.g. "symbol-dce,canonicalize"
```

**Operation elimination cannot be disabled** — it's fundamental to how the
reduction-tree algorithm works.

---

## 2. Implementing DialectReductionPatternInterface

### Step 1: Create shared reduction patterns

```cpp
// lib/Dialect/MyDialect/MyReductionPatterns.h
namespace mlir::mydialect {
void populateMyReductionPatterns(RewritePatternSet &patterns);
}

// lib/Dialect/MyDialect/MyReductionPatterns.cpp
struct SimplifyFoo : public OpRewritePattern<FooOp> { /* ... */ };
struct SimplifyBar : public OpRewritePattern<BarOp> { /* ... */ };

void populateMyReductionPatterns(RewritePatternSet &patterns) {
  patterns.add<SimplifyFoo, SimplifyBar>(patterns.getContext());
}
```

### Step 2: Register the interface on the dialect

```cpp
// lib/Dialect/MyDialect/MyDialect.cpp
#include "mlir/Reducer/ReductionPatternInterface.h"

namespace {
struct MyReductionPatternInterface : public DialectReductionPatternInterface {
  MyReductionPatternInterface(Dialect *dialect)
      : DialectReductionPatternInterface(dialect) {}

  void populateReductionPatterns(RewritePatternSet &patterns) const final {
    populateMyReductionPatterns(patterns);
  }
};
} // namespace

void MyDialect::initialize() {
  // ... add types, ops ...
  addInterface<MyReductionPatternInterface>();  // ← THE critical hook
}
```

### Step 3: Link against MLIRReduceLib

```cmake
add_mlir_dialect_library(MLIRMyDialect
    MyDialect.cpp
    MyReductionPatterns.cpp
    # ...
    LINK_LIBS PUBLIC
    MLIRReduceLib
)
```

**That's it.** The interface is auto-discovered when the dialect is loaded.

---

## 3. Building a Custom mlir-reduce Binary

**You cannot use the system `mlir-reduce` with custom dialects.** MLIR supports
loading dialect plugins via shared libraries (`DialectPlugin`,
`--load-dialect-plugin` on `mlir-opt`), but `mlir-reduce` doesn't expose this
flag — `MlirReduceMain` has no plugin support.  The [official docs](https://mlir.llvm.org/docs/Tools/mlir-reduce/#build-a-custom-mlir-reduce) confirm you must build your own:

```cpp
// tools/my-reduce.cpp
#include "lib/Dialect/MyDialect/MyDialect.h"
#include "lib/Transform/MyPasses.h"          // for --opt-reduction-pass
#include "mlir/IR/DialectRegistry.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Tools/mlir-reduce/MlirReduceMain.h"

int main(int argc, char **argv) {
  DialectRegistry registry;
  registerAllDialects(registry);
  registry.insert<mydialect::MyDialect>();

  // Register passes so --opt-reduction-pass can use them.
  mydialect::registerMyPasses();

  MLIRContext context(registry);
  return failed(mlirReduceMain(argc, argv, context));
}
```

CMake:
```cmake
add_llvm_executable(my-reduce my-reduce.cpp)
target_link_libraries(my-reduce PRIVATE
    ${dialect_libs} ${conversion_libs}
    MLIRReduceLib MLIRMyDialect MyTransformPasses)
```

---

## 4. The Interestingness Script

`mlir-reduce` needs a script that answers: "Is this reduced variant still interesting?"

Convention: **exit 1 = interesting** (keep), **exit 0 = not interesting** (discard).

```python
#!/usr/bin/env python3
"""Returns 1 if the input parses successfully, 0 otherwise."""
import subprocess, sys, os

OPT = os.path.join(os.path.dirname(__file__),
                   "..", "build-ninja", "tools", "tutorial-opt")

result = subprocess.run([OPT, sys.argv[1]],
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
sys.exit(1 if result.returncode == 0 else 0)
```

A real bug-reduction script would check for a specific error message:

```bash
#!/bin/bash
# Check that the specific crash still happens
my-opt "$1" 2>&1 | grep -q "segmentation fault"
```

---

## 5. Running mlir-reduce

### Mode A: reduction-tree (all strategies)

```bash
my-reduce input.mlir \
  -reduction-tree='traversal-mode=0 test=interestingness.py'
```

Uses operation elimination + your dialect patterns + built-in passes.

### Mode B: opt-reduction-pass (only specific passes)

```bash
my-reduce input.mlir \
  --opt-reduction-pass='opt-pass=my-pass,canonicalize test=interestingness.py'
```

Runs ONLY the named passes. No operation elimination, no dialect patterns.

### Mode C: reduction-tree with your pass added

```bash
my-reduce input.mlir \
  -reduction-tree='traversal-mode=0 test=interestingness.py' \
  --opt-pass=my-pass
```

Adds `my-pass` to the built-in passes run during reduction-tree.

---

## 6. Writing Lit Tests

```mlir
// RUN: my-reduce %s -reduction-tree='traversal-mode=0 test=%S/interesting.py' 2>/dev/null | FileCheck %s
// RUN: my-reduce %s --opt-reduction-pass='opt-pass=my-pass test=%S/interesting.py' 2>/dev/null | FileCheck %s
//
// `2>/dev/null` suppresses stderr from intermediate invalid reduction
// attempts (e.g., "null operand found"). These are expected — they are
// variants the interestingness script correctly rejected.
//
// To run manually from project root:
//   ./build-ninja/tools/my-reduce tests/my_test.mlir \
//     -reduction-tree='traversal-mode=0 test=tests/interesting.py'

// CHECK-LABEL: func.func @test
func.func @test(%arg0: i32) -> i32 {
  %c1 = arith.constant 1 : i32
  %0 = arith.muli %arg0, %c1 : i32
  return %0 : i32
}

// After reduction, multiply-by-1 should be gone:
// CHECK-NOT: arith.muli
// CHECK: return %arg0
```

### Lit test gotchas

- **Comments after `CHECK:` are part of the pattern!** Keep them on separate lines:
  ```mlir
  // BAD:  // CHECK: return %arg0   ← this arrow breaks matching
  // GOOD: // MyPattern should eliminate the mul.
  // GOOD: // CHECK: return %arg0
  ```

- **`pipefail` + error-reporting passes**: If your pass exits non-zero on error,
  wrap it: `{ my-opt %s --error-pass 2>&1 || true; } | FileCheck %s`

- **Register the binary** in `tests/lit.cmake.cfg.py` tools list and
  `tests/CMakeLists.txt` test dependencies.

---

## 7. Debugging Tips

### See what mlir-reduce is doing
The stderr from `--reduction-tree` shows intermediate attempts. Don't redirect
it when debugging:

```bash
my-reduce input.mlir -reduction-tree='...'   # keep stderr visible
```

Expect to see errors like `null operand found` — these are valid rejected variants.

### Test your interestingness script standalone

```bash
./interestingness.py input.mlir; echo "exit: $?"
# Should print "exit: 1" for a valid input
```

### Test your patterns in isolation

```bash
my-opt input.mlir --my-pass   # verify the pattern fires correctly
```

### Check which reduction strategies ran
The final output shows what survived. If your pattern should have fired but didn't:
- Is the dialect loaded? (`registry.insert<MyDialect>()`)
- Is `addInterface<MyReductionPatternInterface>()` in `initialize()`?
- Does the pattern actually match the IR that survived to that point?
```

## 8. File Layout Summary

```
lib/Dialect/MyDialect/
├── MyReductionPatterns.h        # public: populateMyReductionPatterns()
├── MyReductionPatterns.cpp      # pattern definitions
├── MyDialect.cpp                # addInterface<MyReductionPatternInterface>()
└── CMakeLists.txt               # LINK_LIBS PUBLIC MLIRReduceLib

tools/
├── my-reduce.cpp                # custom mlir-reduce main
└── CMakeLists.txt               # link MLIRReduceLib + dialect + passes

tests/
├── my_reduce_test.mlir          # lit test (RUN + CHECK)
├── interesting.py               # interestingness script
├── lit.cmake.cfg.py             # add binary to tools list
└── CMakeLists.txt               # add binary to TEST_DEPENDS
```
