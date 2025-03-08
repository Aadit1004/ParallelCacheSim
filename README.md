# Parallel CPU D-Cache Simulation

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Requirements](#requirements)
- [Building the Simulator](#building-the-simulator)
- [Running the Cache Simulator](#running-the-cache-simulator)
- [Running Tests](#running-tests)
- [Advanced Usage](#example-usage)
    - [Running with Gprof (Performance Profiling)](#running-with-gprof-performance-profiling)
    - [Running with Valgrind (Memory Leak Detection)](#running-with-valgrind-memory-leak-detection)
    - [Clang-Tidy (Static Analysis & Linting)](#clang-tidy-static-analysis--linting)
- [Sources](#sources)

## Overview

This project is a performance-focused simulator designed to model multi-level data caching (D-Cache) behavior in CPUs, with configurable cache sizes, associativity, replacement policies, and write policies. The simulation supports multi-threaded execution to evaluate cache efficiency under parallel workloads.

## Features

This simulator supports configurable cache and memory sizes, allowing for realistic CPU caching behavior analysis. Each cache block is 64 bytes, and the associativity options include direct-mapped, fully associative, 4-way, and 8-way set associative configurations. It implements FIFO (First-In-First-Out), LRU (Least Recently Used), and LFU (Least Frequently Used) replacement policies, providing flexibility in cache management strategies. 

Both Write-Back (WB) and Write-Through (WT) write policies are supported to simulate different memory consistency models. The simulator runs in single-threaded mode or can scale up to 16 threads for parallel workload simulations. Additionally, verbose logging is available for detailed execution insights but is not recommended for large memory access files.

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

You can use one of the predefined example files located in the `examples/` directory, or create your own memory access file.

**To ensure your custom file follows the correct format, refer to the project's [Memory Access Format Guide](https://github.com/Aadit1004/ParallelCacheSim/blob/main/src/io/README.md)**


### Example Usage
```bash
./cache_sim -cache_size medium -threads 4 -policy LRU -assoc 1 -write_policy WB -trace memory_access.txt --verbose
```

**For detailed argument descriptions, refer to the project's [CLI Arguments Guide](https://github.com/Aadit1004/ParallelCacheSim/blob/main/src/cli/README.md)**
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

### Sources

- [UCSD - The Basics of Cache](https://cseweb.ucsd.edu/classes/su07/cse141/cache-handout.pdf)
- [Wikipedia - CPU Cache](https://en.wikipedia.org/wiki/CPU_cache#:~:text=A%20CPU%20cache%20is%20a,levels%20are%20implemented%20with%20eDRAM)
- [University of Pittsburgh - The MESI protocol](https://people.cs.pitt.edu/~melhem/courses/2410p/ch5-4.pdf)
- [Wikipedia - MESI protocol](https://en.wikipedia.org/wiki/MESI_protocol)
- [How Cache Works Inside a CPU](https://youtu.be/zF4VMombo7U?si=G-msyXsnpYtLq48j)
- [L11 4 how caches work](https://youtu.be/_ENicOgC6ks?si=bBfXUD2Mqr_tyQRU)
- [What is Cache Memory? L1, L2, and L3 Cache Memory Explained](https://youtu.be/IA8au8Qr3lo?si=ykWy38Rrrpg9UaBS)
- [Cache Memory - Ben Lutkevich](https://www.techtarget.com/searchstorage/definition/cache-memory)