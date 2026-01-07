import subprocess
import sys
import os
import time
from typing import Optional

# --- Color Output Configuration ---
ENABLE_COLOR_OUTPUT = True

# ANSI color codes
COLORS = {
    "reset"  : "\033[0m",
    "black"  : "\033[30m",
    "red"    : "\033[91m",
    "green"  : "\033[92m",
    "yellow" : "\033[93m",
    "blue"   : "\033[94m",
    "magenta": "\033[95m",
    "cyan"   : "\033[96m",
    "white"  : "\033[97m",
    "bold"     : "\033[1m",
    "underline": "\033[4m"
}

def colored_print(text, color=None, end='\n', file=sys.stdout):
    """
    Prints text with optional ANSI color.
    """
    if ENABLE_COLOR_OUTPUT and color in COLORS:
        print(f"{COLORS[color]}{text}{COLORS['reset']}", end=end, file=file)
    else:
        print(text, end=end, file=file)

def error(text, end='\n', file=sys.stderr):
    colored_print(text, color="red", end=end, file=file)

def warn(text, end='\n', file=sys.stderr):
    colored_print(text, color="yellow", end=end, file=sys.stderr)

def success(text, end='\n', file=sys.stdout):
    colored_print(text, color="green", end=end, file=file)

def blue(text, end='\n', file=sys.stdout):
    colored_print(text, color="blue", end=end, file=file)

# --- End Color Output Configuration ---

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '..'))
INCLUDE_PATH = os.path.abspath(os.path.join(REPO_DIR, 'include'))
BIN_DIR = os.path.abspath(os.path.join(REPO_DIR, 'bin'))
BINARY_PATH = None
test_counter = 0

def compile_cpp(source_path: str, bin_name: Optional[str] = None, compile_args: Optional[list] = None) -> tuple[str, int, float]:
    """
    Compiles a C++ source file using g++ and returns information about the compilation.

    Args:
        source_path (str): The path to the source file.
        compile_args (Optional[list], optional): A list of additional arguments for g++. Defaults to None.

    Returns:
        tuple[str, int, float]: A tuple containing:
            - The path to the compiled binary.
            - The size of the binary in bytes.
            - The compilation time in seconds.
    """
    if not os.path.exists(BIN_DIR):
        os.makedirs(BIN_DIR)

    source_filename = os.path.basename(source_path)
    base_name, _ = os.path.splitext(source_filename)
    if bin_name:
        output_name = bin_name
    else:
        output_name = f"{base_name}.exe" if sys.platform == "win32" else base_name
    output_path = os.path.join(BIN_DIR, output_name)

    command = ['g++', source_path, '-o', output_path, f'-I{INCLUDE_PATH}'
               '-std=c++17', '-Werror', '-Wall', '-Wextra', '-Wpedantic', '-Wno-missing-field-initializers']
    if compile_args:
        command.extend(compile_args)

    blue(f"Compiling: {' '.join(command)}")

    start_time = time.time()

    try:
        result = subprocess.run(command, capture_output=True, text=True, check=True, encoding='utf-8')

        end_time = time.time()
        compile_time = end_time - start_time

        if result.stdout:
            blue("Compiler STDOUT:")
            print(result.stdout)
        if result.stderr:
            blue("Compiler STDERR:")
            print(result.stderr)

        binary_size = os.path.getsize(output_path)
        success(f"Successfully compiled {source_path} to {output_path}")
        success(f"Binary size: {binary_size} bytes, Time: {compile_time:.2f}s")

        return output_path, binary_size, compile_time

    except subprocess.CalledProcessError as e:
        error(f"Compilation failed for {source_path}.", file=sys.stderr)
        error(f"Return Code: {e.returncode}", file=sys.stderr)
        error("STDOUT:", file=sys.stderr)
        print(e.stdout, file=sys.stderr)
        error("STDERR:", file=sys.stderr)
        print(e.stderr, file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError as e:
        error("Error: g++ not found. Please ensure it is installed and in your PATH.", file=sys.stderr)
        print(e, file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        error(f"An unexpected error occurred during compilation: {e}", file=sys.stderr)
        sys.exit(1)


def run_binary(args: list, binary_path: Optional[str] = None) -> tuple[str, str, int]:
    """
    Runs the binary executable and captures its stdout, stderr, and return code.
    If binary_path is not provided, it uses the global BINARY_PATH.
    """
    path_to_binary = binary_path if binary_path else BINARY_PATH
    if not path_to_binary:
        error("Error: Binary path not set. Please set BINARY_PATH globally or pass it to run_binary.", file=sys.stderr)
        sys.exit(1)

    command = [path_to_binary] + args
    blue(f"Executing: {' '.join(command)}")
    try:
        result = subprocess.run(command, capture_output=True, text=True, check=False, encoding='utf-8')
        return result.stdout, result.stderr, result.returncode
    except FileNotFoundError:
        error(f"Error: Binary not found at {path_to_binary}. Please ensure it is compiled and placed there.", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        error(f"An error occurred: {e}", file=sys.stderr)
        sys.exit(1)


def test_case(name: str, args: list,
              expected_output_substrings: list[str] = [],
              expected_error_keywords: list[str] = [],
              expected_return_code: int = 0,
              binary_path: Optional[str] = None) -> bool:
    """
    Runs a test case and validates the results.

    Args:
        name (str): The name of the test case for identification purposes.
        binary_path (str): The path to the binary executable.
        args (list): A list of arguments to be passed to the binary.
        expected_output_substrings (list[str], optional): A list of substrings that are expected to be present in the stdout. Defaults to an empty list.
        expected_error_keywords (list[str], optional): A list of keywords that are expected to be present in the stderr. Defaults to an empty list.
        expected_return_code (int, optional): The expected return code from the binary execution. Defaults to 0.

    Returns:
        bool: True if the test case passes all checks, False otherwise.
    """
    global test_counter
    test_counter += 1
    colored_print(f"\n--- [{test_counter}] {name} ---", color="cyan", end='\n')
    stdout, stderr, returncode = run_binary(args, binary_path)

    blue("STDOUT:")
    print(stdout)
    blue("STDERR:")
    print(stderr)
    blue(f"Return Code: {returncode}")

    is_success = True

    # Check if all expected output substrings are present in stdout
    if expected_output_substrings:
        for sub in expected_output_substrings:
            if sub not in stdout:
                error(f"FAIL: Expected output substring '{sub}' not found in STDOUT.", file=sys.stderr)
                is_success = False
            else:
                success(f"PASS: Expected output substring '{sub}' found in STDOUT.")

    # Check if all expected error keywords are present in stderr
    if expected_error_keywords:
        for keyword in expected_error_keywords:
            if keyword.lower() not in stderr.lower():
                error(f"FAIL: Expected error keyword '{keyword}' not found in STDERR.", file=sys.stderr)
                is_success = False
            else:
                success(f"PASS: Expected error keyword '{keyword}' found in STDERR.")
    # If no errors are expected but stderr is not empty, consider it a failure
    elif stderr:
        error(f"FAIL: Unexpected content in STDERR: {stderr}", file=sys.stderr)
        is_success = False

    # Check if the return code matches the expected return code
    if returncode != expected_return_code:
        error(f"FAIL: Expected return code {expected_return_code}, got {returncode}.", file=sys.stderr)
        is_success = False
    else:
        success(f"PASS: Return code is {returncode}.")

    # Print the final result of the test case
    if success:
        success(f"--- Test Case {test_counter} '{name}' PASSED ---\n")
    else:
        error(f"--- Test Case {test_counter} '{name}' FAILED ---\n", file=sys.stderr)

    return is_success


def reset_test_counter():
    global test_counter
    test_counter = 0
