import sys
import os
import test_utils
from test_utils import colored_print, test_case, reset_test_counter

# Define the path to the binary executable
# Assuming the binary is located in the 'bin/' directory at the project root
_local_binary_path = os.path.join(os.path.dirname(__file__), '..', 'bin', 'example')
# Ensure _local_binary_path is an absolute path and unify slashes for cross-platform compatibility
_local_binary_path = os.path.abspath(_local_binary_path).replace('\\', '/')

# Set the global BINARY_PATH in test_utils
test_utils.BINARY_PATH = _local_binary_path

def main():
    reset_test_counter() # Reset counter for main execution
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