import sys
import os
import test_utils
from test_utils import colored_print, test_case, reset_test_counter

def main():
    reset_test_counter() # Reset counter for main execution
    all_tests_passed = True

    # --- Normal usage test cases ---

    # mmv is not a valid subcommand, should be treated as a main command positional argument
    all_tests_passed &= test_case(
        "Main command: 'mmv' as positional argument, grouped short options",
        ["mmv", "-vxvi123", "input1", "input2"],
        expected_output_substrings=[
            "Verbose    : 2",
            "Feature X  : true",
            "Indent     : 123",
            "Output file: mmv",
            "Input files:",
            "input1",
            "input2"
        ]
    )

    # Should print main command help
    all_tests_passed &= test_case(
        "Main command help with subcommand name",
        ["-h", "mv"],
        expected_output_substrings=[
            "Usage: subcommand [SUBCOMMAND] [OPTIONS]",
            "A simple program to demonstrate ArgLite subcommand feature.",
            "Options:",
            "Subcommands:",
            "status",
            "commit",
            "mv"
        ],
    )

    all_tests_passed &= test_case(
        "Commit subcommand: '-asmcommitmsg' mixed short options with value",
        ["commit", "-asmcommitmsg", "-s", "--no-signoff", "path1", "path2"],
        expected_output_substrings=[
            "Commit command is active.",
            "all     : true",
            "signoff : true",
            "message : commitmsg",
            "pathspec:",
            "path1",
            "path2"
        ]
    )

    all_tests_passed &= test_case(
        "Commit subcommand: mutual exclusive flags, no positional arguments",
        ["commit", "-s", "--no-signoff"],
        expected_output_substrings=[
            "Commit command is active.",
            "signoff : false", # --no-signoff should override -s
            "pathspec:",
        ]
    )

    all_tests_passed &= test_case(
        "Grep subcommand: multiple -e options",
        ["grep", "-eabc", "-e", "123"],
        expected_output_substrings=[
            "Grep command is active.",
            "patterns:",
            "abc\n123",
        ]
    )

    all_tests_passed &= test_case(
        "Move subcommand: flexible positional arguments",
        ["mv", "src", "-f", "dst"],
        expected_output_substrings=[
            "Move command is active.",
            "force      : true",
            "source     : src",
            "destination: dst"
        ]
    )

    all_tests_passed &= test_case(
        "Status subcommand: simple activation",
        ["status"],
        expected_output_substrings=[
            "Status command is active."
        ]
    )

    # --- Error usage test cases ---

    # Missing required positional argument for mv
    all_tests_passed &= test_case(
        "Move subcommand: missing destination positional argument",
        ["mv", "source_only"],
        expected_error_keywords=["Missing", "required", "positional", "argument", "destination"],
        expected_return_code=1
    )

    # Commit subcommand: missing argument for -m
    all_tests_passed &= test_case(
        "Commit subcommand: missing option value",
        ["commit", "-m", "-a"],
        expected_error_keywords=["Option", "-m, --message", "requires", "a", "value"],
        expected_return_code=1
    )

    # Commit subcommand: missing argument for --date (type mismatch)
    all_tests_passed &= test_case(
        "Commit subcommand: missing argument for --date (type mismatch)",
        ["commit", "--date", "not_a_date"],
        expected_error_keywords=["Option", "--date", "expect", "integer", "got", "not_a_date"],
        expected_return_code=1
    )

    # Invalid option for commit subcommand
    all_tests_passed &= test_case(
        "Commit subcommand: invalid option",
        ["commit", "--invalid-commit-option"],
        expected_error_keywords=["Invalid", "option", "--invalid-commit-option"],
        expected_return_code=1
    )

    # Invalid option for main command when subcommand is active
    all_tests_passed &= test_case(
        "Commit subcommand: invalid main command option when subcommand is active",
        ["commit", "--verbose"], # --verbose is only valid for main
        expected_error_keywords=["Unrecognized", "option", "--verbose"],
        expected_return_code=1
    )

    # Main command: invalid option
    all_tests_passed &= test_case(
        "Main command: invalid subcommand option",
        ["-m", "commit message"],
        expected_error_keywords=["Unrecognized", "option", "-m"],
        expected_return_code=1
    )

    if all_tests_passed:
        colored_print("\nAll subcommand tests PASSED!", color="green")
        sys.exit(0)
    else:
        colored_print("\nSome subcommand tests FAILED!", color="red", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    _local_binary_name = "subcommand"
    colored_print(f"Running {_local_binary_name} tests...", color="blue")
    # Define the path to the binary executable
    _local_binary_path = os.path.join(os.path.dirname(__file__), '..', 'bin', _local_binary_name)
    # Ensure _local_binary_path is an absolute path and unify slashes for cross-platform compatibility
    _local_binary_path = os.path.abspath(_local_binary_path).replace('\\', '/')

    # Set the global BINARY_PATH in test_utils
    test_utils.BINARY_PATH = _local_binary_path

    main()
