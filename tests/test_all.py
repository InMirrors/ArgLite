#!/usr/bin/env python3
import sys
import os
import subprocess
from dataclasses import dataclass, field
from typing import List, Optional

from test_utils import colored_print, compile_cpp, run_binary, error, success

# Make all paths absolute to run the script from anywhere
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '..'))
INCLUDE_DIR = os.path.abspath(os.path.join(REPO_DIR, 'include'))
BIN_DIR = os.path.abspath(os.path.join(REPO_DIR, 'bin'))

@dataclass
class TestTarget:
    """Represents a single, self-contained test case."""
    source: str
    description: str = ""
    # Suffix for the output binary file to allow multiple compilations of the same source.
    suffix: str = ""
    bin_path: str = ""
    # If None, the compiled binary is executed directly as its own test.
    test_script: Optional[str] = None
    # Arguments for a single run of the test script.
    test_script_args: List[str] = field(default_factory=list)
    extra_compile_args: List[str] = field(default_factory=list)


def magenta(text, end='\n', file=sys.stdout):
    colored_print(text, end=end, file=file)


def compile_files(test_targets: List[TestTarget], common_compile_args: List[str]):
    for target in test_targets:
        source_path = os.path.join(SCRIPT_DIR, target.source)
        basename, _ = os.path.splitext(target.source)
        bin_basename = f"{basename}{target.suffix}"
        bin_name = f"{bin_basename}.exe" if sys.platform == "win32" else bin_basename
        bin_path = os.path.join(BIN_DIR, bin_name)
        target.bin_path = bin_path

        compile_args = common_compile_args + target.extra_compile_args
        # compile_cpp will exit on failure.
        compile_cpp(source_path, bin_name, compile_args)


def run_test_script(script_path, *args) -> int:
    """Runs a Python test script and returns its exit code."""
    command = [sys.executable, script_path, *args]
    magenta(f"Running test script: {' '.join(command)}")
    try:
        # Use check=False to capture failures without raising an exception immediately
        process = subprocess.run(command, check=False)
        if process.returncode != 0:
            error(f"Test script {script_path} {' '.join(args)} FAILED with exit code {process.returncode}", file=sys.stderr)
        return process.returncode
    except FileNotFoundError:
        error(f"Error: Test script not found at {script_path}", file=sys.stderr)
        exit(1)


def run_test_binary(target: TestTarget) -> int:
    """Compiles all test targets and runs their respective tests."""
    stdout, stderr, returncode = run_binary([], binary_path=target.bin_path)
    print(stdout)
    if stderr:
        print(stderr, file=sys.stderr) # Print stderr for debugging

    if returncode != 0:
        error(f"Test '{target.source}' FAILED with return code {returncode}", file=sys.stderr)
        return 1
    else:
        success(f"Test '{target.source}' PASSED")
        return 0


def main():
    """Main function to define and run all tests."""
    COMMON_COMPILE_ARGS = [f"-I{INCLUDE_DIR}"]

    # Data-driven list of test targets. Each item is a complete, independent test.
    test_targets = [
        TestTarget(source="example.cpp", description="Test full version",
                   test_script="test_example_binary.py"),
        TestTarget(source="minimal_example.cpp", description="Test minimal version",
                   test_script="test_example_binary.py", test_script_args=["m"]),
        TestTarget(source="other_features.cpp", description="Test other features",
                   test_script="test_other_features.py"),
        TestTarget(source="subcommand.cpp", description="Test subcommands",
                   test_script="test_subcommand_binary.py"),
        TestTarget(source="test_get_pos.cpp", description="Test positional argument logic (Minimal)",
                   suffix="_minimal", extra_compile_args=["-DMINIMAL"]),
        TestTarget(source="test_get_pos.cpp", description="Test positional argument logic (Full)",
                   suffix="_full"),
    ]

    # --- Compilation Phase ---
    magenta("--- Starting Compilation Phase ---")
    compile_files(test_targets, COMMON_COMPILE_ARGS)
    success("\n--- All sources compiled successfully. Starting Test Phase ---")

    # --- Test Execution Phase ---
    total_failures = 0
    for i, target in enumerate(test_targets):
        horizontal_line = '=' * 80
        magenta(f"\n{horizontal_line}")
        magenta(f"Running Test {i+1}/{len(test_targets)}: {target.description}")
        magenta(f"{horizontal_line}\n")

        # Run associated Python test script
        if target.test_script:
            script_path = os.path.join(SCRIPT_DIR, target.test_script)
            # Pass the binary path to the test script as the first argument
            total_failures += run_test_script(script_path, *target.test_script_args)

        # Run self-contained C++ test
        else:
            run_test_binary(target)

    # --- Final Summary ---
    if total_failures == 0:
        success("\n--- ALL TESTS PASSED! ---")
        sys.exit(0)
    else:
        error(f"\n--- {total_failures} TEST(S) FAILED! ---", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
