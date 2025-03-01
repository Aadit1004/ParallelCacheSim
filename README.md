# Parallel CPU Cache Simulation

## Overview

This project is a performance-focused simulator designed to model multi-level CPU caching behavior with configurable cache sizes, associativity, replacement policies, and write policies. The simulation supports multi-threaded execution to evaluate cache efficiency under parallel workloads.

## Requirements

To build and run the simulator, ensure you have the following dependencies installed:
- **Compiler**: GCC or Clang with C++17 support
-  **Build System**: make
- **Catch2** (for running unit tests)
- **Valgrind** (optional, for memory analysis)
- **gprof** (optional, for profiling performance)
- **Clang-Tidy** (optional, for static analysis and code linting)

To install dependencies on **Ubuntu**:
```bash
sudo apt update && sudo apt install build-essential valgrind gprof clang-tidy
```
On **MacOS** (via Homebrew):
```bash
brew install gcc valgrind clang-tidy
```
<br>

## Building the Simulator

Compile the simulator using:
```bash
make
```
This will generate the `cache_sim` executable in the project root.
<br>

## Running the Cache Simulator

To run the simulator, use:
```bash
./cache_sim -cache_size <size> -threads <num> -policy <replacement> -assoc <ways> -write_policy <wp> -trace <file> [--verbose]

```
**For detailed argument descriptions, refer to the project's [CLI Arguments Guide](https://github.com/Aadit1004/ParallelCacheSim/blob/main/src/cli/README.md)**

### Example Usage
```bash
./cache_sim -cache_size medium -threads 4 -policy LRU -assoc 1 -write_policy WB -trace memory_access.txt --verbose
```
<br>

## Running Tests

The project includes unit tests written with Catch2.

To run all tests:
```bash
make
./cache_test
```

To run specific test categories:
```bash
./cache_test [cache]
./cache_test [profiling]
```
**For detailed testing instructions, refer to the project's [Testing Guide](https://github.com/Aadit1004/ParallelCacheSim/blob/main/tests/README.md)**

<br>

## Advanced Usage

### Running with Gprof (Performance Profiling)
To profile the simulator execution time:

```bash
make gprof-run ARGS="-cache_size <size> -threads <num> -policy <replacement> -assoc <ways> -write_policy <wp> -trace <file> [--verbose]"
```
The profiling report is saved in `gprof_report.txt` in the root directory.

To profile tests execution time:

```bash
make gprof-test-run
```
The profiling report is saved in `gprof_test_report.txt` in the root directory.
<br>

### Running with Valgrind (Memory Leak Detection)
To check for memory leaks:

```bash
make valgrind ARGS="-cache_size <size> -threads <num> -policy <replacement> -assoc <ways> -write_policy <wp> -trace <file> [--verbose]"
```

To check the tests for memory leaks:

```bash
make valgrind_test
```
<br>

### Clang-Tidy (Static Analysis & Linting)
To run Clang-Tidy on all source files and automatically apply fixes:

```bash
make tidy
```
This will apply performance and static analysis checks to improve code quality.