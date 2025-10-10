import subprocess
import sys
import os

# Define the path to the binary executable
# Assuming the binary is located in the 'bin/' directory at the project root
BINARY_PATH = os.path.join(os.path.dirname(__file__), '..', 'bin', 'example')
# Ensure BINARY_PATH is an absolute path and unify slashes for cross-platform compatibility
BINARY_PATH = os.path.abspath(BINARY_PATH).replace('\\', '/')

# --- Color Output Configuration ---
ENABLE_COLOR_OUTPUT = True

# ANSI color codes
COLORS = {
    "reset": "\033[0m",
    "blue": "\033[94m",
    "green": "\033[92m",
    "red": "\033[91m",
    "yellow": "\033[93m",
    "cyan": "\033[96m",
    "magenta": "\033[95m",
    "bold": "\033[1m",
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

# --- End Color Output Configuration ---

def run_binary(args):
    """
    Runs the binary executable and captures its stdout, stderr, and return code.
    """
    command = [BINARY_PATH] + args
    colored_print(f"Executing: {' '.join(command)}", color="blue")
    try:
        result = subprocess.run(command, capture_output=True, text=True, check=False, encoding='utf-8')
        return result.stdout, result.stderr, result.returncode
    except FileNotFoundError:
        colored_print(f"Error: Binary not found at {BINARY_PATH}. Please ensure it is compiled and placed there.", color="red", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        colored_print(f"An error occurred: {e}", color="red", file=sys.stderr)
        sys.exit(1)

test_counter = 0
def test_case(name, args, expected_output_substrings: list[str]=[], expected_error_keywords: list[str]=[], expected_return_code=0):
    """
    Runs a test case and validates the results.
    expected_error_keywords: A list of keywords that must be present in stderr.
    """
    global test_counter
    test_counter += 1
    colored_print(f"\n--- [{test_counter}] {name} ---", color="cyan", end='\n')
    stdout, stderr, returncode = run_binary(args)

    colored_print("STDOUT:", color="blue")
    print(stdout)
    colored_print("STDERR:", color="blue")
    print(stderr)
    colored_print(f"Return Code: {returncode}", color="blue")

    success = True

    if expected_output_substrings:
        for sub in expected_output_substrings:
            if sub not in stdout:
                colored_print(f"FAIL: Expected output substring '{sub}' not found in STDOUT.", color="red", file=sys.stderr)
                success = False
            else:
                colored_print(f"PASS: Expected output substring '{sub}' found in STDOUT.", color="green")

    if expected_error_keywords:
        for keyword in expected_error_keywords:
            if keyword.lower() not in stderr.lower():
                colored_print(f"FAIL: Expected error keyword '{keyword}' not found in STDERR.", color="red", file=sys.stderr)
                success = False
            else:
                colored_print(f"PASS: Expected error keyword '{keyword}' found in STDERR.", color="green")
    elif stderr: # If no expected errors, but stderr is not empty, consider it a failure
        colored_print(f"FAIL: Unexpected content in STDERR: {stderr}", color="red", file=sys.stderr)
        success = False

    if returncode != expected_return_code:
        colored_print(f"FAIL: Expected return code {expected_return_code}, got {returncode}.", color="red", file=sys.stderr)
        success = False
    else:
        colored_print(f"PASS: Return code is {returncode}.", color="green")

    if success:
        colored_print(f"--- Test Case {test_counter} '{name}' PASSED ---\n", color="green")
    else:
        colored_print(f"--- Test Case {test_counter} '{name}' FAILED ---\n", color="red", file=sys.stderr)
    return success

def main():
    global test_counter
    test_counter = 0 # Reset counter for main execution
    all_tests_passed = True

    # --- Normal usage test cases ---

    # Basic usage: providing required positional arguments (output-file and at least one input-file)
    all_tests_passed &= test_case(
        "Basic usage with required positional arguments",
        ["my_output.txt", "input1.txt"],
        expected_output_substrings=[
            "Verbose    : false",
            "Switch 1   : false",
            "Switch 2   : false",
            "Debug      : false",
            "Count      : 0",
            "Interval   : 0",
            "Rate       : 123", # Default value
            "Output file: my_output.txt",
            "Output Path: output.txt", # Default value
            "Input files:",
            "input1.txt"
        ]
    )

    # With all flags and some options, including debug mode
    all_tests_passed &= test_case(
        "All flags and some options, including debug mode",
        ["-v", "-1", "--switch2", "-d", "true", "--count", "10", "--interval", "20", "-r", "50.5", "-o", "custom_path.log", "output.log", "file1.txt", "file2.txt"],
        expected_output_substrings=[
            "Verbose    : true",
            "Switch 1   : true",
            "Switch 2   : true",
            "Debug      : true",
            "Count      : 10",
            "Interval   : 20",
            "Rate       : 50.5",
            "Output file: output.log",
            "Output Path: custom_path.log",
            "Input files:",
            "file1.txt",
            "file2.txt"
        ]
    )

    # Mixed short and long options, with debug mode set to false
    all_tests_passed &= test_case(
        "Mixed short and long options, debug false",
        ["--verbose", "-d", "false", "-n", "5", "--out-path", "mixed.txt", "final.out", "input.data"],
        expected_output_substrings=[
            "Verbose    : true",
            "Debug      : false",
            "Count      : 5",
            "Output Path: mixed.txt",
            "Output file: final.out",
            "Input files:",
            "input.data"
        ]
    )

    # Short option combinations: -12v
    all_tests_passed &= test_case(
        "Short option combination -12v",
        ["-12v", "output.txt", "input.txt"],
        expected_output_substrings=[
            "Verbose    : true",
            "Switch 1   : true",
            "Switch 2   : true",
            "Output file: output.txt",
            "Input files:",
            "input.txt"
        ]
    )

    # Short option combination with argument: -12n 10
    all_tests_passed &= test_case(
        "Short option combination with argument -12n 10",
        ["-12n", "10", "output.txt", "input.txt"],
        expected_output_substrings=[
            "Switch 1   : true",
            "Switch 2   : true",
            "Count      : 10",
            "Output file: output.txt",
            "Input files:",
            "input.txt"
        ]
    )

    # Positional argument mixed between options: -1 file
    all_tests_passed &= test_case(
        "Positional argument mixed between options: -1 file",
        ["-1", "my_output.txt", "input.txt"],
        expected_output_substrings=[
            "Switch 1   : true",
            "Output file: my_output.txt",
            "Input files:",
            "input.txt"
        ]
    )

    # Long option with equals sign: --count=15
    all_tests_passed &= test_case(
        "Long option with equals sign: --count=15",
        ["--count=15", "output.txt", "input.txt"],
        expected_output_substrings=[
            "Count      : 15",
            "Output file: output.txt",
            "Input files:",
            "input.txt"
        ]
    )

    # Long option with equals sign: --out-path=path/to/file.log
    all_tests_passed &= test_case(
        "Long option with equals sign: --out-path=path/to/file.log",
        ["--out-path=path/to/file.log", "output.txt", "input.txt"],
        expected_output_substrings=[
            "Output Path: path/to/file.log",
            "Output file: output.txt",
            "Input files:",
            "input.txt"
        ]
    )

    # --- Error usage test cases ---

    # Missing required positional argument 'output-file'
    all_tests_passed &= test_case(
        "Missing required positional argument 'output-file",
        [],
        expected_error_keywords=["Missing", "required", "positional", "argument", "output-file"],
        expected_return_code=1
    )

    # Missing required positional argument 'input-files' (only output-file provided)
    all_tests_passed &= test_case(
        "Missing required positional argument 'input-files",
        ["my_output.txt"],
        expected_error_keywords=["Missing", "required", "positional", "argument", "input-files"],
        expected_return_code=1
    )

    # Invalid option (ensure other parts are valid to reach this error)
    all_tests_passed &= test_case(
        "Invalid option",
        ["--invalid-option", "output.txt", "input.txt"],
        expected_error_keywords=["Invalid", "option", "--invalid-option"],
        expected_return_code=1
    )

    # Invalid option with a value
    all_tests_passed &= test_case(
        "Invalid option with a value",
        ["--invalid-option", "value", "output.txt", "input.txt"],
        expected_error_keywords=["Invalid", "option", "--invalid-option"],
        expected_return_code=1
    )

    # Option argument type mismatch for --count (e.g., passing non-integer)
    all_tests_passed &= test_case(
        "Option argument type mismatch (count)",
        ["--count", "abc", "output.txt", "input.txt"],
        expected_error_keywords=["Option", "--count", "expect", "integer", "got", "abc"],
        expected_return_code=1
    )

    # Option argument type mismatch for -r (e.g., passing non-double)
    all_tests_passed &= test_case(
        "Option argument type mismatch (rate)",
        ["-r", "xyz", "output.txt", "input.txt"],
        expected_error_keywords=["Option", "-r", "expect", "double", "got", "xyz"],
        expected_return_code=1
    )

    # Missing argument for option --count (e.g., not providing a value)
    all_tests_passed &= test_case(
        "Missing argument for option (count)",
        ["--count", "output.txt", "input.txt"], # "output.txt" will be parsed as the value for --count, leading to a type error
        expected_error_keywords=["Option", "--count", "expect", "integer", "got", "output.txt"],
        expected_return_code=1
    )

    # Missing argument for getBool option -d
    all_tests_passed &= test_case(
        "Missing argument for getBool option -d",
        ["-d", "output.txt", "input.txt"], # "output.txt" will be parsed as the value for -d, but it's not a valid boolean string
        expected_error_keywords=["Option", "-d", "expect", "boolean", "got", "output.txt"],
        expected_return_code=1
    )

    if all_tests_passed:
        colored_print("\nAll tests PASSED!", color="green")
        sys.exit(0)
    else:
        colored_print("\nSome tests FAILED!", color="red", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()