#!/usr/bin/env python3
import sys
import os
import re
import subprocess
import argparse
import glob
from dataclasses import dataclass
from typing import List, Tuple

# Adjust the path to import from the parent directory
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from test_utils import colored_print, compile_cpp

# Make all paths absolute to run the script from anywhere
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '..', '..'))
INCLUDE_DIR = os.path.abspath(os.path.join(REPO_DIR, 'include'))
BIN_DIR = os.path.abspath(os.path.join(REPO_DIR, 'bin'))

@dataclass
class BenchmarkTarget:
    """Represents a single benchmark case for runtime analysis."""
    name: str
    binary_name: str = ""
    allocs: int = 0
    allocated_bytes: int = 0
    peak_heap_usage: float = 0.0

def run_valgrind_massif(binary_path: str, run_args: List[str], name: str) -> float:
    """
    Runs a binary with valgrind's massif tool to find peak heap usage.
    Returns the peak heap usage in KB.
    """
    massif_command = ['valgrind', '--tool=massif', binary_path] + run_args
    colored_print(f"Executing Massif: {' '.join(massif_command)[:100]}", color="blue")

    try:
        # Run massif
        subprocess.run(massif_command, capture_output=True, text=True, check=False, encoding='utf-8')

        # Find the massif output file
        massif_files = glob.glob('massif.out.*')
        if not massif_files:
            colored_print("WARN: Massif output file not found.", color="yellow", file=sys.stderr)
            return 0.0

        latest_massif_file = max(massif_files, key=os.path.getctime)
        colored_print(f"Found massif output: {latest_massif_file}", "cyan")

        # Run ms_print to analyze the massif output
        ms_print_command = ['ms_print', latest_massif_file]
        ms_print_result = subprocess.run(ms_print_command, capture_output=True, text=True, check=False, encoding='utf-8')

        # Parse the output of ms_print
        ms_print_output = ms_print_result.stdout
        match_kb = re.search(r'KB\n\s*(\d+\.\d+)', ms_print_output)
        match_mb = re.search(r'MB\n\s*(\d+\.\d+)', ms_print_output)
        if match_kb:
            # Clean up the massif file
            os.remove(latest_massif_file)
            peak_kb = float(match_kb.group(1))
            return peak_kb
        elif match_mb:
            os.remove(latest_massif_file)
            peak_mb = float(match_mb.group(1))
            return peak_mb * 1024
        else:
            os.rename(latest_massif_file, f"{name}.massif")
            colored_print("WARN: Could not parse peak heap usage from ms_print output.", color="yellow", file=sys.stderr)
            colored_print("ms_print STDOUT (first 500 chars):", "yellow", file=sys.stderr)
            print(ms_print_output[:500], file=sys.stderr)
            return 0.0

    except FileNotFoundError as e:
        colored_print(f"Error: Command '{e.filename}' not found. Please ensure it is installed and in your PATH.", color="red", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        colored_print(f"An unexpected error occurred during massif execution: {e}", color="red", file=sys.stderr)
        sys.exit(1)


def run_valgrind_memcheck(binary_path: str, run_args: List[str]) -> Tuple[int, int]:
    """
    Runs a binary with valgrind and parses its heap memory usage from stderr.
    Returns allocs, allocated_bytes.
    """
    command = ['valgrind', binary_path] + run_args
    colored_print(f"Executing Valgrind: {' '.join(command)[:100]}", color="blue")

    try:
        result = subprocess.run(command, capture_output=True, text=True, check=False, encoding='utf-8')

        valgrind_stderr = result.stderr

        match = re.search(r'total heap usage: (\d+) allocs, \d+ frees, ([\d,]+) bytes allocated', valgrind_stderr)
        if match:
            allocs = int(match.group(1))
            allocated_bytes = int(match.group(2).replace(',', ''))
            return allocs, allocated_bytes
        else:
            colored_print("WARN: Valgrind heap usage line not found in stderr.", color="yellow", file=sys.stderr)
            colored_print("Valgrind STDERR (first 500 chars):", color="yellow", file=sys.stderr)
            print(valgrind_stderr[:500], file=sys.stderr)
            return 0, 0

    except FileNotFoundError:
        colored_print("Error: valgrind not found. Please ensure it is installed and in your PATH.", color="red", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        colored_print(f"An unexpected error occurred during valgrind execution: {e}", color="red", file=sys.stderr)
        sys.exit(1)


def run_benchmarks(targets: List[BenchmarkTarget], run_args: List[str]):
    """Runs valgrind benchmarks for each target."""
    for target in targets:
        colored_print(f"=== Benchmarking Runtime for {target.name} ===", color="magenta")

        binary_path = os.path.join(BIN_DIR, target.binary_name)

        if not os.path.exists(binary_path):
            colored_print(f"Error: Binary not found for {target.name} at {binary_path}. Please compile it first.", color="red", file=sys.stderr)
            exit(1)

        target.allocs, target.allocated_bytes = run_valgrind_memcheck(binary_path, run_args)
        target.peak_heap_usage = run_valgrind_massif(binary_path, run_args, target.binary_name)


def print_results(targets: List[BenchmarkTarget], baseline: BenchmarkTarget, no_baseline: bool):
    """Prints the benchmark results in a Markdown table."""
    colored_print("=== Runtime Benchmark Results ===", color="magenta")
    if not no_baseline:
        print("NOTE: Baseline measurements have been subtracted.")

    print("| Name         | Allocs | Allocated (KB) | Peak Heap (KB) |")
    print("| ------------ | -----: | -------------: | -------------: |")

    if not no_baseline:
        allocated_str = f"{(baseline.allocated_bytes/1024):.2f}"
        peak_str = f"{baseline.peak_heap_usage:.2f}"
        print(f"| {baseline.name:<12} | {baseline.allocs:>6} | {allocated_str:>14} | {peak_str:>14} |")

    for target in targets:
        allocs_val = target.allocs - baseline.allocs if not no_baseline else target.allocs
        allocated_val = target.allocated_bytes - baseline.allocated_bytes if not no_baseline else target.allocated_bytes
        peak_val = target.peak_heap_usage - baseline.peak_heap_usage if not no_baseline else target.peak_heap_usage

        allocs_str = str(allocs_val)
        allocated_str = f"{allocated_val/1024:.2f}"
        peak_str = f"{peak_val:.2f}"

        print(f"| {target.name:<12} | {allocs_str:>6} | {allocated_str:>14} | {peak_str:>14} |")


def main():
    """Main function to define and run runtime benchmarks."""
    parser = argparse.ArgumentParser(description="Run runtime benchmarks with Valgrind.")
    parser.add_argument("-a", "--arg", dest="extra_args", action="append", default=[], help="Additional arguments to pass to the benchmarked binary (can be specified multiple times)")
    parser.add_argument("-b", "--no-baseline", action="store_true", help="Do not subtract baseline measurements from a 'hello world' program.")
    parser.add_argument("-n", "--num-infiles", type=int, help="Generate n infile arguments to test performance with many arguments.")
    args = parser.parse_args()

    COMMON_RUN_ARGS = ["-v", "-c", "123", "outfile", "infile1", "infile2"] + args.extra_args

    if args.num_infiles:
        if args.num_infiles > 0:
            extra_infiles = [f"infile{i}" for i in range(3, 3 + args.num_infiles)]
            COMMON_RUN_ARGS.extend(extra_infiles)

    if not os.path.exists(BIN_DIR):
        os.makedirs(BIN_DIR)

    baseline = BenchmarkTarget(name="Baseline", binary_name="hello_world")
    if not args.no_baseline:
        # Compile and run the baseline hello_world
        hello_world_src = os.path.join(SCRIPT_DIR, 'hello_world.cpp')
        hello_world_bin, _, _ = compile_cpp(hello_world_src, ["-s", "-O2"])
        baseline.allocs, baseline.allocated_bytes = run_valgrind_memcheck(hello_world_bin, [])
        baseline.peak_heap_usage = run_valgrind_massif(hello_world_bin, [], baseline.binary_name)
        colored_print(f"hello_world baseline: allocs={baseline.allocs}, allocated_bytes={baseline.allocated_bytes}, peak_heap={baseline.peak_heap_usage}KB", color="green")

    benchmark_targets = [
        BenchmarkTarget(name="ArgLite Mini", binary_name="minimal"),
        BenchmarkTarget(name="ArgLite Full", binary_name="full"),
        BenchmarkTarget(name="CLI11",        binary_name="CLI11"),
        BenchmarkTarget(name="cxxopts",      binary_name="cxxopts"),
        BenchmarkTarget(name="args",         binary_name="args"),
        BenchmarkTarget(name="argparse",     binary_name="argparse"),
    ]
    run_benchmarks(benchmark_targets, COMMON_RUN_ARGS)
    print_results(benchmark_targets, baseline, args.no_baseline)

if __name__ == "__main__":
    main()
