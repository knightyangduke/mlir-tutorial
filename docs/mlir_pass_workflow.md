# How to find unimplemented methods in an MLIR pass

## Background: what you must implement

MLIR tablegen generates a CRTP base class `XxxBase<DerivedT>` (in the `.inc` file)
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
