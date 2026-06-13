#pragma once

#include "llvm/Support/CommandLine.h"
#include <string>

// Global CLI flag: --my-debug-tag=<tag>
// `inline` (C++17) guarantees exactly one instance across all translation
// units that include this header — no .cpp definition file needed.
inline llvm::cl::opt<std::string> debugActionTag(
    "my-debug-tag",
    llvm::cl::desc("The action tag to intercept and log (e.g. poly-optimize-add)"),
    llvm::cl::value_desc("tag-name"),
    llvm::cl::init("") // Default is empty (don't intercept anything)
);
