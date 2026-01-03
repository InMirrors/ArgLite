#!/usr/bin/env python3
import sys
import os
import argparse
from dataclasses import dataclass, field
from typing import List

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
    """Represents a single benchmark case."""
    name: str
    source: str
    compile_time: float = 0.0
    binary_size: int = 0
    extra_compile_args: List[str] = field(default_factory=list)


def magenta(text, end='\n', file=sys.stdout):
    colored_print(text, end=end, file=file)


def test_single_file(source_path: str, compile_args: List[str], iterations: int):
        """ Benchmark a single specified file """
        compile_time, binary_size = perform_compilation_test(
            source_path, compile_args, iterations
        )
        custom_target = BenchmarkTarget(name="Custom File", source=source_path, compile_time=compile_time, binary_size=binary_size)
        print_results([custom_target])


def run_benchmarks(targets: List[BenchmarkTarget], common_compile_args: List[str], iterations: int):
    """Compiles each target n times, records average time and final size."""
    for target in targets:
        magenta(f"=== Benchmarking {target.name} ({iterations} iterations) ===")
        source_path = os.path.join(SCRIPT_DIR, target.source)
        compile_args = common_compile_args + target.extra_compile_args
        target.compile_time, target.binary_size = perform_compilation_test(
            source_path, compile_args, iterations
        )
        magenta(f"Average time for {target.name}: {target.compile_time:.4f}s\n")


def perform_compilation_test(source_path: str, compile_args: List[str], iterations: int):
    """Performs compilation test for a single target and returns average time and binary size."""
    total_time = 0.0
    binary_size = 0

    for i in range(iterations):
        _, size, time = compile_cpp(source_path, compile_args=compile_args)
        total_time += time
        if i == iterations - 1: # On the last iteration, store path and size
            binary_size = size

    avg_time = total_time / iterations
    return avg_time, binary_size


def print_results(targets: List[BenchmarkTarget]):
    """Prints the benchmark results in a Markdown table."""
    magenta("=== Results ===")
    print("| Name         | Time (s) | Size (KB) |")
    print("| ------------ | -------: | --------: |")
    for target in targets:
        time_str = f"{target.compile_time:.2f}"
        size_str = f"{target.binary_size / 1024:.1f}"
        print(f"| {target.name:<12} | {time_str:>8} | {size_str:>9} |")


def main():
    """Main function to define and run benchmarks."""
    parser = argparse.ArgumentParser(description="Run compilation benchmarks.")
    parser.add_argument("iterations", type=int, nargs="?", default=10, help="Number of compilation iterations (default: 10)")
    parser.add_argument("-a", "--arg", dest="extra_args", action="append", default=[], help="Additional compilation arguments (can be specified multiple times)")
    parser.add_argument("-f", "--file", dest="source_file", help="Specify a single source file to benchmark")
    args = parser.parse_args()

    # These arguments are common for all targets
    COMMON_COMPILE_ARGS = [f"-I{INCLUDE_DIR}", "-s", "-DNDEBUG"] + args.extra_args

    if not os.path.exists(BIN_DIR):
        os.makedirs(BIN_DIR)

    if args.source_file:
        # Run a single file benchmark
        test_single_file(args.source_file, COMMON_COMPILE_ARGS, args.iterations)
        return 0

    benchmark_targets = [
        BenchmarkTarget(name="ArgLite Mini", source="minimal.cpp"),
        BenchmarkTarget(name="ArgLite Full", source="full.cpp"),
        BenchmarkTarget(name="CLI11",        source="CLI11.cpp", extra_compile_args=["-lCLI11"]),
        BenchmarkTarget(name="cxxopts",      source="cxxopts.cpp", extra_compile_args=["-DCXXOPTS_NO_REGEX"]),
        BenchmarkTarget(name="args",         source="args.cpp"),
        BenchmarkTarget(name="argparse",     source="argparse.cpp"),
    ]
    run_benchmarks(benchmark_targets, COMMON_COMPILE_ARGS, args.iterations)
    print_results(benchmark_targets)

if __name__ == "__main__":
    main()
