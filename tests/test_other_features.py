import sys
import os
import test_utils
from test_utils import colored_print, test_case, reset_test_counter

def main():
    reset_test_counter() # Reset counter for main execution
    all_tests_passed = True

    # --- Normal usage test cases ---

    all_tests_passed &= test_case(
        "Repeated option -I, --include",
        ["-nAdmin", "-I123", "-v", "--include", "a,bc", "-d,", "-I", "45,6", "--include=def"],
        expected_output_substrings=[
            "Verbose    : 1",
            "Delimiter  : ','",
            "Include:",
            "123\na\nbc\n45\n6\ndef",
        ]
    )

    all_tests_passed &= test_case(
        "Repeated option -I, --include",
        ["-nAdmin", "-I123", "-v", "--include", "a,bc", "-i30,", "-d ", "-I", "45,6", "--include=def"],
        expected_output_substrings=[
            "Verbose    : 1",
            "Indent     : 30",
            "Delimiter  : ' '",
            "Include:",
            "123\na,bc\n45,6\ndef",
        ]
    )

    all_tests_passed &= test_case(
        "Repeated flag -v, --verbose",
        ["-nAdmin", "-I123", "-vv", "--include", "a,bc", "-v", "-i30,", "-d "],
        expected_output_substrings=[
            "Verbose    : 3",
            "Delimiter  : ' '",
            "Include:",
            "123\na,bc",
        ]
    )

    all_tests_passed &= test_case(
        "Optional option -o, --optional: not set",
        ["-nAdmin", "-x"],
        expected_output_substrings=[
            "Feature X  : true",
            "Optional   : (not set)",
        ]
    )

    all_tests_passed &= test_case(
        "Optional option -o, --optional: set to an empty string",
        ["-nAdmin", "-x", "-o", ""],
        expected_output_substrings=[
            "Feature X  : true",
            "Optional   : ",
        ]
    )

    all_tests_passed &= test_case(
        "Usage in the help message",
        ["-h"],
        expected_output_substrings=[
            "[OPTIONS] --name=<string>",
        ]
    )

    # --- Error usage test cases ---

    all_tests_passed &= test_case(
        "Missing required option -n, --name",
        ["-x"],
        expected_error_keywords=[
            "Error:",
            "-n, --name",
            "required",
        ],
        expected_return_code=1
    )

    all_tests_passed &= test_case(
        "Missing the value for required option -n, --name",
        ["-n"],
        expected_error_keywords=[
            "-n, --name",
            "requires a value",
        ],
        expected_return_code=1
    )

    if all_tests_passed:
        colored_print("\nAll tests PASSED!", color="green")
        sys.exit(0)
    else:
        colored_print("\nSome tests FAILED!", color="red", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    _local_binary_name = "other_features"
    colored_print(f"Running {_local_binary_name} tests...", color="blue")
    # Define the path to the binary executable
    _local_binary_path = os.path.join(os.path.dirname(__file__), '..', 'bin', _local_binary_name)
    # Ensure _local_binary_path is an absolute path and unify slashes for cross-platform compatibility
    _local_binary_path = os.path.abspath(_local_binary_path).replace('\\', '/')

    # Set the global BINARY_PATH in test_utils
    test_utils.BINARY_PATH = _local_binary_path

    main()
