# -*- Python -*-

import os

import lit.formats
import lit.util

from lit.llvm import llvm_config

# Configuration file for the 'lit' test runner.

# name: The name of this test suite.
config.name = "MLIR_TUTORIAL"

config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)

# suffixes: A list of file extensions to treat as test files.
config.suffixes = [".mlir"]

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join(config.project_binary_dir, "tests")

config.substitutions.append(("%PATH%", config.environment["PATH"]))
config.substitutions.append(("%shlibext", config.llvm_shlib_ext))
config.substitutions.append(("%project_source_dir", config.project_source_dir))

llvm_config.with_system_environment(["HOME", "INCLUDE", "LIB", "TMP", "TEMP"])

llvm_config.use_default_substitutions()

# excludes: A list of directories to exclude from the testsuite. The 'Inputs'
# subdirectories contain auxiliary inputs for various tests in their parent
# directories.
config.excludes = ["Inputs", "Examples", "CMakeLists.txt", "README.txt", "LICENSE.txt",
    # Reference output files (no RUN: lines, used by other tests).
    "affine_loop_unroll_transformed.mlir",
    "after_polly_to_llvm.mlir",
    "mul-to-shift.mlir",
    "mul-to-shift_transformed.mlir",
    "mul_to_add_transformed.mlir",
    "mul_to_add_transformed_peel_has_higher_priority.mlir",
    "noisy_check_overflow_error.mlir",
    "poly_syntax_failed_2.mlir",
    "poly_syntax_failure.mlir",
    "poly_syntax_with_materialization.mlir",
    "poly_to_standard_transformed.mlir",
    "sccp_transformed.mlir",
]

# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join(config.project_binary_dir, "test")
config.project_tools_dir = os.path.join(config.project_binary_dir, "tools")

# Tweak the PATH to include the tools dir.
llvm_config.with_environment("PATH", config.llvm_tools_dir, append_path=True)

tool_dirs = [config.project_tools_dir, config.llvm_tools_dir]
tools = [
    "mlir-opt",
    "mlir-runner",
    "tutorial-opt",
    "tutorial-reduce",
]

llvm_config.add_tool_substitutions(tools, tool_dirs)
