#!/usr/bin/env python3
"""
This script automates the process of updating the `setShortNonFlagOptsStr()`
function call in C++ source files that use the ArgLite library.

It scans for valued options (like `getInt`, `get<string>`) and collects their
short option characters, then updates the corresponding `setShortNonFlagOptsStr()` call.

How to use:
1. Configure the patterns and subcommand information in the 'CONFIGURATION' section below.
2. Run the script from the command line:
   python3 update_short_opts.py your_file.cpp [your_other_file.cpp ...]
"""

import os
import sys
import argparse
import re
from typing import List, Pattern
from pathlib import Path
from dataclasses import dataclass

# Adjust the path to import utils
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'tests')))
from test_utils import REPO_DIR, colored_print


# ==============================================================================
# --- CONFIGURATION ---
# You should modify this section to match your project structure.
# ==============================================================================

# --- Default files to process ---
# Modify this list to target your specific source files.
DEFAULT_FILES = [
    # "examples/simple_wrapped.cpp",
    "examples/subcommand_wrapped.cpp",
]

# --- Main Parser Patterns ---
# Regular expression to find the `setShortNonFlagOptsStr()` call for the main parser.
# It should contain one capture group for the string content.
MAIN_PARSER_SETTER_PATTERN = r'Parser::setShortNonFlagOptsStr\s*\(\s*"([^"]*)"\s*\)'

# Regular expression to find valued options for the main parser.
# It should contain one capture group for the short option character.

# Choose the pattern that matches your ArgLite version by uncommenting one.
# For Minimal version (e.g., getInt, getString)
# MAIN_PARSER_GETTER_PATTERN = r'(?:Parser::get(?:Int|Double|String|Bool)|(?:getInt))\s*\(\s*"(\w)(?:,|")'
# For Full version (e.g., get<int>)
MAIN_PARSER_GETTER_PATTERN = r'Parser::get<.*?>\s*\(\s*"(\w)(?:,|")'


# --- Subcommand Patterns & Configuration ---

@dataclass
class SubcommandConfig:
    """Stores configuration for a single subcommand."""
    # The variable name of the subcommand object in the C++ code.
    name: str
    # The prefix for the setter pattern.
    setter_prefix: str
    # The prefix for the getter pattern.
    getter_prefix: str


# A list of all subcommands to process. Add your subcommands here.
# Example from `subcommand_wrapped.cpp`.
SUBCOMMAND_NAMES = [
    "commit",
    "grep",
]

# This script is designed specifically for the example file. If your code is based on
# this example and follows a similar style, the script should work correctly.
# Otherwise, you might need to manually define the list of SubcommandConfig objects
# instead of relying on the script's automatic generation. Alternatively,
# you can modify the `generate_subcommand_configs()` function to suit your project.
# Example of manual definition:
# SUBCOMMANDS: List[SubcommandConfig] = [
#     SubcommandConfig(name='commit', setter_prefix=r'commit', getter_prefix=r'(?:commitCmd\(\))'),
#     SubcommandConfig(name='grep', setter_prefix=r'grep', getter_prefix=r'(?:grepCmd\(\))'),
# ]

# Base pattern for the setter function, will be prefixed with `setter_prefix`.
SUBCOMMAND_SETTER_PATTERN_BASE = r'\.setShortNonFlagOptsStr\s*\(\s*"([^"]*)"\s*\)'

# Base pattern for getter functions, will be prefixed with `getter_prefix`.
SUBCOMMAND_GETTER_PATTERN_BASE = r'\.get<.*?>\s*\(\s*"(\w)(?:,|")'


# ==============================================================================
# --- UTILITIES ---
# (Usually no changes are needed for this section)
# ==============================================================================

error_count = 0

def error(text, end='\n', file=sys.stderr):
    """Prints an error message and increments the global error counter."""
    global error_count
    error_count += 1
    colored_print("[error] " + text, color="red", end=end, file=file)

def warn(text, end='\n', file=sys.stderr):
    colored_print("[warning]" + text, color="yellow", end=end, file=file)

def success(text, end='\n', file=sys.stdout):
    colored_print("[info] " + text, color="green", end=end, file=file)

def info_blue(text, end='\n', file=sys.stdout):
    colored_print("[info] " + text, color="blue", end=end, file=file)

def info_cyan(text, end='\n', file=sys.stdout):
    colored_print("[info] " + text, color="cyan", end=end, file=file)

def info(text, end='\n', file=sys.stdout):
    colored_print("[info] " + text, end=end, file=file)

def debug(text, end='\n', file=sys.stdout):
    global args
    if args.debug:
        colored_print("[debug] " + text, color="black", end=end, file=file)


# ==============================================================================
# --- CORE LOGIC ---
# (Usually no changes are needed for this section)
# ==============================================================================

def generate_subcommand_configs(names: List[str]) -> List['SubcommandConfig']:
    """Generates a list of SubcommandConfig objects from a list of names."""
    return [
        SubcommandConfig(
            name=name,
            setter_prefix=fr'{name}',
            getter_prefix=fr'(?:{name}Cmd\(\))'
        ) for name in names
    ]


def process_file_content(content: str, setter_pattern: Pattern[str], getter_pattern: Pattern[str]) -> str:
    """
    Finds short options using the getter pattern and updates the string
    in the setter pattern call.

    Args:
        content: The source code as a string.
        setter_pattern: Regex to find the `setShortNonFlagOptsStr()` call.
        getter_pattern: Regex to find all valued options and capture short chars.

    Returns:
        The updated source code content.
    """
    # Find all short options defined in the file.
    short_opts = getter_pattern.findall(content)

    # Check for duplicates
    if len(short_opts) != len(set(short_opts)):
        seen = set()
        duplicates = {x for x in short_opts if x in seen or seen.add(x)}
        error(f"   - Found duplicate short options: {', '.join(duplicates)}")
        return content

    # Check if any options were found.
    if not short_opts:
        error("   - No options found, please check your configuration. Use '--debug' to see more details.")
        return content

    # To sort them alphabetically, uncomment the following line:
    # short_opts.sort()

    new_opts_str = "".join(short_opts)

    # Find the `setShortNonFlagOptsStr()` call and check if it needs updating.
    match = setter_pattern.search(content)
    if not match:
        error("   - Can not find `setShortNonFlagOptsStr()`.")
        return content

    current_opts_str = match.group(1)
    if current_opts_str == new_opts_str:
        info(f"    - Options are already up-to-date ('{current_opts_str}'). No changes needed.")
        return content

    if args.dry_run:
        info_cyan(f"    - Need to update: '{current_opts_str}' -> '{new_opts_str}'")
        return content
    else:
        info_cyan(f"    - Updating options: '{current_opts_str}' -> '{new_opts_str}'")
        start, end = match.span(1)
        return content[:start] + new_opts_str + content[end:]


def process_file(filepath: Path, subcommands: list[SubcommandConfig]) -> None:
    """
    Processes a single C++ source file to update all configured
    `setShortNonFlagOptsStr()` calls.

    Args:
        filepath: Path to the C++ source file.
    """
    info_blue(f"Processing file: {filepath}")
    try:
        content = filepath.read_text(encoding='utf-8')
        original_content = content

        # 1. Process Main Parser
        info("  -> Checking Main Parser...")
        debug("    - Setter pattern: " + MAIN_PARSER_SETTER_PATTERN)
        debug("    - Getter pattern: " + MAIN_PARSER_GETTER_PATTERN)
        content = process_file_content(
            content,
            re.compile(MAIN_PARSER_SETTER_PATTERN),
            re.compile(MAIN_PARSER_GETTER_PATTERN),
        )

        # 2. Process Subcommands
        for sub_config in subcommands:
            info(f"  -> Checking Subcommand '{sub_config.name}'...")

            # Dynamically construct the full regex patterns for the subcommand
            sub_setter_pattern = sub_config.setter_prefix + SUBCOMMAND_SETTER_PATTERN_BASE
            sub_getter_pattern = sub_config.getter_prefix + SUBCOMMAND_GETTER_PATTERN_BASE
            debug("    - Setter pattern: " + sub_setter_pattern)
            debug("    - Getter pattern: " + sub_getter_pattern)
            setter_re = re.compile(sub_setter_pattern)
            getter_re = re.compile(sub_getter_pattern)

            content = process_file_content(content, setter_re, getter_re)

        # 3. Write back to file if changes were made
        if args.dry_run:
            return
        if content != original_content:
            filepath.write_text(content, encoding='utf-8')
            success(f"  => File '{filepath}' was updated successfully.")
        else:
            info(f"  => No updates were necessary for '{filepath}'.")

    except FileNotFoundError:
        error(f"Error: File not found at '{filepath}'")
    except Exception as e:
        error(f"An unexpected error occurred while processing '{filepath}': {e}")


def main(args: argparse.Namespace) -> None:
    files_to_process = args.files or DEFAULT_FILES

    file_paths = [Path(os.path.join(REPO_DIR, f)) for f in files_to_process]
    subcommands = generate_subcommand_configs(SUBCOMMAND_NAMES)

    if args.dry_run:
        info_cyan("Dry run mode enabled, no files will be modified.")

    for file_path in file_paths:
        process_file(file_path, subcommands)

    if error_count > 0:
        error(f"Found {error_count} error(s). Exiting.")
        sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Auto-update `setShortNonFlagOptsStr()` in ArgLite C++ files."
    )
    parser.add_argument("-d", "--debug", action="store_true", help="Enable debug output.")
    parser.add_argument("-r", "--dry-run", action="store_true", help="Perform a dry run without modifying files.",)
    parser.add_argument("files", nargs="*", type=Path, help="One or more C++ source files to process.",)

    args = parser.parse_args()

    main(args)
