# Cache Simulator Tests

This folder contains unit tests for the Cache Simulator, written using [Catch2](https://github.com/catchorg/Catch2).

## Running Tests

To run all tests, simply use:

```sh
make
./cache_test
```
This will build and execute all tests.

## Filtering Tests by Category
You can run specific categories of tests using Catch2's filtering feature. Use the category name inside square brackets (`[category]`). For example, to run only the argument parser tests:
```sh
./cache_test [arg_parser]
```

## Available Test Categories
Here are the valid test categories you can filter with:
- `[arg_parser]` - Tests related to CLI argument parsing
- `[cache]` - Cache behavior tests
- `[memory]` - Tests for memory initialization and read/write operations
- `[cache_config]` - Tests for cache configuration and validation
- More to come...

## Running a Single Test Case
To run a specific test case, use its name:
```sh
./cache_test "Arg Parser - Cache Size"
```
For more filtering options, refer to the [Catch2 documentation](https://github.com/catchorg/Catch2/blob/devel/docs/command-line.md).