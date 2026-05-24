# VSCode / clangd Navigation Setup for MLIR Projects

## The Problem

Out of the box, Go-to-Definition and hover-type-info don't work in this project because:

1. **No `compile_commands.json` at the workspace root** — clangd doesn't know the
   compiler flags (include paths, defines, `-std` version) for each file.
2. **Generated `.inc` headers only exist in the build directory** — files like
   `build-ninja/lib/Transform/Affine/Passes.h.inc` are produced by `mlir-tblgen`
   at build time and are not in the source tree. Without the correct `-I` flags,
   clangd can't find them.
3. **Pre-built LLVM** — the MLIR/LLVM source is in `llvm-project/`, which is outside
   the project's own build. Without pointing clangd there, cross-project navigation
   won't work.

---

## Step 1 — Generate `compile_commands.json`

CMake with Ninja does not generate it by default. Regenerate the build with:

```bash
cd build-ninja
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

This writes `build-ninja/compile_commands.json` with one entry per `.cpp` file.
Each entry contains the full compiler invocation, including all `-I` paths, e.g.:

```json
{
  "file": ".../AffineFullUnrollPatternRewrite.cpp",
  "command": "c++ -I/home/yeyang/llvm-project/mlir/include
                   -I/home/yeyang/llvm-project/build-release/include
                   -I/home/yeyang/mlir-tutorial/build-ninja
                   ..."
}
```

The `-I/home/yeyang/mlir-tutorial/build-ninja` entry is what lets clangd resolve
`#include "lib/Transform/Affine/Passes.h.inc"` to the generated file.

Then symlink it to the workspace root so clangd finds it automatically:

```bash
cd /home/yeyang/mlir-tutorial
ln -s build-ninja/compile_commands.json compile_commands.json
```

---

## Step 2 — Create a `.clangd` config file

Create `/home/yeyang/mlir-tutorial/.clangd` to tell clangd exactly where the
compilation database lives, and (optionally) to also index the LLVM source:

```yaml
CompileFlags:
  CompilationDatabase: build-ninja
```

---

## Step 3 (Optional) — Enable navigation into LLVM/MLIR source `.cpp` files

After Steps 1–2, Go-to-Definition navigates to **header declarations** in
`llvm-project/mlir/include/...` but not to **implementation bodies** in
`llvm-project/mlir/lib/...`.

This is because the LLVM `.cpp` files are not listed in this project's
`compile_commands.json`. To fix it, check whether LLVM's own database exists:

```bash
ls /home/yeyang/llvm-project/build-release/compile_commands.json
```

If it exists, add an external index to `.clangd`:

```yaml
CompileFlags:
  CompilationDatabase: build-ninja

Index:
  Background: Build
  External:
    File: /home/yeyang/llvm-project/build-release/compile_commands.json
```

Alternatively, merge both databases into one (more thorough, larger file):

```bash
python3 -c "
import json
with open('build-ninja/compile_commands.json') as f: a = json.load(f)
with open('/home/yeyang/llvm-project/build-release/compile_commands.json') as f: b = json.load(f)
with open('compile_commands.json', 'w') as f: json.dump(a + b, f)
"
```

---

## Step 4 (Optional) — `.inc` files get C++ syntax highlighting

By default VS Code treats `.inc` as plain text. Add this to your VS Code
`settings.json` (Command Palette → *Preferences: Open User Settings (JSON)*):

```json
"files.associations": {
    "*.inc": "cpp"
}
```

---

## Why Navigation from Raw `.inc` Files Still Doesn't Work

Even with the above setup, Go-to-Definition from inside a raw `.inc` file
(e.g. `Passes.h.inc`) may fail for symbols inside `#ifdef` blocks.

The reason: `Passes.h.inc` is not a standalone translation unit — it is designed
to be `#include`d multiple times with different `#define`s active:

| Who includes it | `#define` active | What becomes visible |
|---|---|---|
| `AffineFullUnroll.cpp` | `GEN_PASS_DEF_AFFINEFULLUNROLL` | `AffineFullUnrollBase` body |
| `AffineFullUnroll.h` | `GEN_PASS_DECL_AFFINEFULLUNROLL` | `createAffineFullUnroll()` decl |
| `Passes.h` | `GEN_PASS_REGISTRATION` | `registerAffineFullUnroll()` |

When clangd opens `Passes.h.inc` directly, none of these macros are defined, so
all blocks are preprocessed away and invisible to the indexer.

**Workaround**: always invoke Go-to-Definition from a `.cpp` or `.h` file that
includes `Passes.h.inc` with the right `#define` — navigation will work correctly
from those call sites.

A partial fix via `.clangd` forces defines for `.inc` files, but because each
block `#undef`s its own guard, only the first active block parses cleanly:

```yaml
---
If:
  PathMatch: .*\.inc
CompileFlags:
  Add:
    - -DGEN_PASS_REGISTRATION
    - -DGEN_PASS_DEF_AFFINEFULLUNROLL
    - -DGEN_PASS_DECL_AFFINEFULLUNROLL
    # add more as needed
```

---

## Summary

| Problem | Fix |
|---|---|
| clangd doesn't know include paths | `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON` + symlink |
| clangd can't find the compilation database | create `.clangd` with `CompilationDatabase: build-ninja` |
| Can't navigate into LLVM `.cpp` bodies | add `Index.External` pointing at LLVM's database |
| `.inc` files show as plain text | add `"*.inc": "cpp"` to `files.associations` |
| Go-to-Definition fails inside `.inc` | navigate from a `.cpp`/`.h` caller instead |
