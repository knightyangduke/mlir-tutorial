# Shell I/O Redirection

A quick reference for file descriptors, redirection operators, and `tee`.

---

## File Descriptors

| Number | Name | Default destination |
|---|---|---|
| `0` | **stdin** | keyboard |
| `1` | **stdout** | terminal (normal output) |
| `2` | **stderr** | terminal (errors / diagnostics) |

---

## Redirect Operators

### `>` — Redirect stdout to a file (overwrite)

```bash
cmd > file.txt          # stdout → file.txt (shorthand for 1>)
cmd 1> file.txt         # explicit: same as above
```

### `>>` — Redirect stdout to a file (append)

```bash
cmd >> file.txt         # append stdout to file.txt
```

### `2>` — Redirect stderr to a file

```bash
cmd 2> error.log       # stderr → error.log, stdout → terminal
```

### `2>&1` — Merge stderr into stdout

```bash
cmd > file.txt 2>&1    # both stdout + stderr → file.txt
cmd 2>&1 | grep foo    # both streams → grep
```

The `&` prefix tells the shell to treat the following number as a **file descriptor**, not a filename.

### Common Pitfall

| Wrong | What it actually does |
|---|---|
| `2>1` | Creates a **file** named `1` and writes stderr into it |
| `2>&1` | Redirects stderr to wherever **fd 1** (stdout) is currently going |

### Suppress output

```bash
cmd > /dev/null        # discard stdout (keep stderr visible)
cmd > /dev/null 2>&1   # discard both stdout and stderr
cmd 2> /dev/null       # discard stderr only
```

---

## `tee` — Split output to file and screen

`tee` reads from stdin and writes to both a file and stdout simultaneously.

```bash
cmd | tee file.txt              # stdout → file + screen (overwrite)
cmd | tee -a file.txt           # stdout → file + screen (append)
cmd | tee f1.txt f2.txt         # save to multiple files
cmd 2>&1 | tee file.txt         # both stdout + stderr → file + screen
```

---

## Summary of Common Patterns

| Goal | Command |
|---|---|
| Save normal output to a file | `cmd > out.txt` |
| Save errors to a file | `cmd 2> err.txt` |
| Save everything to one file | `cmd > out.txt 2>&1` |
| See everything + save to file | `cmd 2>&1 \| tee log.txt` |
| Check output in a pager | `cmd 2>&1 \| less` |
| Pipe both streams | `cmd 2>&1 \| grep pattern` |
| Run quietly (no output) | `cmd > /dev/null 2>&1` |

---

## Concrete Examples with `tutorial-opt`

These examples use the `ReduceNoiseChecker` pass, which prints debug info to **stderr** (`llvm::errs()`) and the resulting MLIR module to **stdout**.

### Save the output IR, see debug info on screen

```bash
# From the build-ninja/ directory:
tools/tutorial-opt --noisy-reduce-noise-checker ../tests/noisy_checker.mlir \
    > output.mlir
```

- stdout (the MLIR module) → `output.mlir`
- stderr (debug prints like `[ReduceNoiseChecker] noisy.add ...`) → terminal

### Save debug info, see output IR on screen

```bash
tools/tutorial-opt --noisy-reduce-noise-checker ../tests/noisy_checker.mlir \
    2> debug.log
```

- stdout (MLIR module) → terminal
- stderr (debug prints) → `debug.log`

### Save everything to one file

```bash
tools/tutorial-opt --noisy-reduce-noise-checker ../tests/noisy_checker.mlir \
    > everything.mlir 2>&1
```

- Both stdout and stderr → `everything.mlir`

### See everything + save to a file

```bash
tools/tutorial-opt --noisy-reduce-noise-checker ../tests/noisy_checker_overflow.mlir \
    2>&1 | tee output_with_debug.mlir
```

- Both streams → terminal (visible) AND `output_with_debug.mlir` (saved)

### Suppress debug info, keep only the IR

```bash
tools/tutorial-opt --noisy-reduce-noise-checker ../tests/noisy_checker.mlir \
    2> /dev/null
```

- stdout (MLIR module) → terminal
- stderr (debug prints) → discarded

### Pipe the output IR to another tool

```bash
tools/tutorial-opt --noisy-reduce-noise-checker ../tests/noisy_checker.mlir \
    | FileCheck ../tests/noisy_checker.mlir
```

- stdout → `FileCheck` (which validates the output matches expected patterns)
- stderr → terminal (debug prints visible as they happen)

### Pipe both streams (e.g., search for errors)

```bash
tools/tutorial-opt --noisy-reduce-noise-checker ../tests/noisy_checker_overflow.mlir \
    2>&1 | grep "error:"
```

---

## Visual Diagram

```
         ┌────── cmd ──────┐
 stdin   │                 │
 (fd 0)──┤   process .exe  │
         │                 │
         └──┬──────────┬───┘
            │          │
         stdout      stderr
         (fd 1)      (fd 2)
            │          │
            ▼          ▼
         terminal   terminal
         (default)  (default)
```

With redirection:

```
cmd > out.txt 2>&1

stdout ──► out.txt
stderr ──► (follows stdout) ──► out.txt
```

```
cmd 2>&1 | tee log.txt

stdout ──► tee ──┬──► log.txt
stderr ──► ┘     └──► terminal
```
