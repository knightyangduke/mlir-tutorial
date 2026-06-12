#!/usr/bin/env python3
"""poly_reduce_interesting.py
Returns 1 if tutorial-opt can parse the input without errors, 0 otherwise.
"""
import subprocess, sys, os

TUTORIAL_OPT = os.path.join(os.path.dirname(__file__),
                            "..", "build-ninja", "tools", "tutorial-opt")

result = subprocess.run([TUTORIAL_OPT, sys.argv[1]],
                        stdout=subprocess.DEVNULL,
                        stderr=subprocess.DEVNULL)
sys.exit(1 if result.returncode == 0 else 0)
