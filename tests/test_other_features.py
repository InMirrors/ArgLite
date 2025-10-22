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
        ["-I123", "-v", "--include", "a,bc", "-d,", "-I", "45,6", "--include=def"],
        expected_output_substrings=[
            "Verbose    : true",
            "Feature X  : false",
            "Indent     : 26",
            "Delimiter  : ','",
            "Include:",
            "123\na\nbc\n45\n6\ndef",
        ]
    )

    all_tests_passed &= test_case(
        "Repeated option -I, --include",
        ["-I123", "-v", "--include", "a,bc", "-i30,", "-d ", "-I", "45,6", "--include=def"],
        expected_output_substrings=[
            "Verbose    : true",
            "Feature X  : false",
            "Indent     : 30",
            "Delimiter  : ' '",
            "Include:",
            "123\na,bc\n45,6\ndef",
        ]
    )

    # --- Error usage test cases ---

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
