#include "../catch2/catch.hpp"
#include "../src/cli/arg_parser.h"
#include <tuple>

/*
Valid input:
./cache_sim -cache_size medium -threads 4 -policy LRU -assoc 1 -write_policy WB -trace memory_access.txt --verbose
*/

TEST_CASE("Arg Parser - Cache Size", "[arg_parser]") {
    auto [cacheParam, cacheSize, expectedResult] = GENERATE(
        std::make_tuple("-cache_size", "small", true),
        std::make_tuple("-cache_size", "medium", true),
        std::make_tuple("-cache_size", "large", true),
        std::make_tuple("-cache_size", "invalid_size", false),
        std::make_tuple("-cache-size", "small", false)
    );

    char* validInput[] = {
        (char*)"./cache_test",
        (char*)cacheParam,
        (char*)cacheSize,
        (char*)"-threads",
        (char*)"4",
        (char*)"-policy",
        (char*)"LRU",
        (char*)"-assoc",
        (char*)"1",
        (char*)"-write_policy",
        (char*)"WB",
        (char*)"-trace",
        (char*)"memory_access.txt",
        (char*)"--verbose",
    };
    int validInputCount = 14;

    ArgParser argParser(validInputCount, validInput);

    REQUIRE(argParser.validateArguments() == expectedResult);
}

TEST_CASE("Arg Parser - Threads", "[arg_parser]") {
    auto [threadParam, threadDigit, expectedResult] = GENERATE(
        std::make_tuple("-threads", "1", true),
        std::make_tuple("-threads", "2", true),
        std::make_tuple("-threads", "10", true),
        std::make_tuple("-threads", "16", true),
        std::make_tuple("-threads", "18", false),
        std::make_tuple("-threads", "one", false),
        std::make_tuple("-thread", "1", false)
    );

    char* validInput[] = {
        (char*)"./cache_test",
        (char*)"-cache_size",
        (char*)"small",
        (char*)threadParam,
        (char*)threadDigit,
        (char*)"-policy",
        (char*)"LRU",
        (char*)"-assoc",
        (char*)"1",
        (char*)"-write_policy",
        (char*)"WB",
        (char*)"-trace",
        (char*)"memory_access.txt",
        (char*)"--verbose",
    };
    int validInputCount = 14;

    ArgParser argParser(validInputCount, validInput);

    REQUIRE(argParser.validateArguments() == expectedResult);
}

TEST_CASE("Arg Parser - Replacement Policy", "[arg_parser]") {
    auto [policyParam, policyValue, expectedResult] = GENERATE(
        std::make_tuple("-policy", "LFU", true),
        std::make_tuple("-policy", "LRU", true),
        std::make_tuple("-policy", "FIFO", true),
        std::make_tuple("-policy", "TEST", false),
        std::make_tuple("-policies", "LRU", false)
    );

    char* validInput[] = {
        (char*)"./cache_test",
        (char*)"-cache_size",
        (char*)"small",
        (char*)"-threads",
        (char*)"4",
        (char*)policyParam,
        (char*)policyValue,
        (char*)"-assoc",
        (char*)"1",
        (char*)"-write_policy",
        (char*)"WB",
        (char*)"-trace",
        (char*)"memory_access.txt",
        (char*)"--verbose",
    };
    int validInputCount = 14;

    ArgParser argParser(validInputCount, validInput);

    REQUIRE(argParser.validateArguments() == expectedResult);
}

TEST_CASE("Arg Parser - Associativity", "[arg_parser]") {
    auto [assocyParam, assocValue, expectedResult] = GENERATE(
        std::make_tuple("-assoc", "1", true),
        std::make_tuple("-assoc", "4", true),
        std::make_tuple("-assoc", "0", true),
        std::make_tuple("-assoc", "2", false),
        std::make_tuple("-assoc", "one", false),
        std::make_tuple("-associativity", "1", false)
    );

    char* validInput[] = {
        (char*)"./cache_test",
        (char*)"-cache_size",
        (char*)"small",
        (char*)"-threads",
        (char*)"4",
        (char*)"-policy",
        (char*)"LRU",
        (char*)assocyParam,
        (char*)assocValue,
        (char*)"-write_policy",
        (char*)"WB",
        (char*)"-trace",
        (char*)"memory_access.txt",
        (char*)"--verbose",
    };
    int validInputCount = 14;

    ArgParser argParser(validInputCount, validInput);

    REQUIRE(argParser.validateArguments() == expectedResult);
}

TEST_CASE("Arg Parser - Write Policy", "[arg_parser]") {
    auto [writeParam, writeValue, expectedResult] = GENERATE(
        std::make_tuple("-write_policy", "WB", true),
        std::make_tuple("-write_policy", "WT", true),
        std::make_tuple("-write_policy", "WA", false),
        std::make_tuple("-writepolicy", "WB", false)
    );

    char* validInput[] = {
        (char*)"./cache_test",
        (char*)"-cache_size",
        (char*)"small",
        (char*)"-threads",
        (char*)"4",
        (char*)"-policy",
        (char*)"LRU",
        (char*)"-assoc",
        (char*)"1",
        (char*)writeParam,
        (char*)writeValue,
        (char*)"-trace",
        (char*)"memory_access.txt",
        (char*)"--verbose", 
    };
    int validInputCount = 14;

    ArgParser argParser(validInputCount, validInput);

    REQUIRE(argParser.validateArguments() == expectedResult);
}

TEST_CASE("Arg Parser - Trace", "[arg_parser]") {
    auto [traceParam, traceValue, expectedResult] = GENERATE(
        std::make_tuple("-trace", "memory_access.txt", true),
        std::make_tuple("-trace", "", false),
        std::make_tuple("-traces", "memory_access.txt", false)
    );

    char* validInput[] = {
        (char*)"./cache_test",
        (char*)"-cache_size",
        (char*)"small",
        (char*)"-threads",
        (char*)"4",
        (char*)"-policy",
        (char*)"LRU",
        (char*)"-assoc",
        (char*)"1",
        (char*)"-write_policy",
        (char*)"WB",
        (char*)traceParam,
        (char*)traceValue,
        (char*)"--verbose",
    };
    int validInputCount = 14;

    ArgParser argParser(validInputCount, validInput);

    REQUIRE(argParser.validateArguments() == expectedResult);
}

TEST_CASE("Arg Parser - Verbose", "[arg_parser]") {
    auto [verboseParam, expectedResult] = GENERATE(
        std::make_tuple("--verbose", true),
        std::make_tuple("-verbose", false)
    );

    char* validInput[] = {
        (char*)"./cache_test",
        (char*)"-cache_size",
        (char*)"small",
        (char*)"-threads",
        (char*)"4",
        (char*)"-policy",
        (char*)"LRU",
        (char*)"-assoc",
        (char*)"1",
        (char*)"-write_policy",
        (char*)"WB",
        (char*)"-trace",
        (char*)"memory_access.txt",
        (char*)verboseParam
    };
    int validInputCount = 14;

    ArgParser argParser(validInputCount, validInput);

    REQUIRE(argParser.validateArguments() == expectedResult);
}

TEST_CASE("Arg Parser - Valid without Verbose", "[arg_parser]") {
    char* validInput[] = {
        (char*)"./cache_test",
        (char*)"-cache_size",
        (char*)"small",
        (char*)"-threads",
        (char*)"4",
        (char*)"-policy",
        (char*)"LRU",
        (char*)"-assoc",
        (char*)"1",
        (char*)"-write_policy",
        (char*)"WB",
        (char*)"-trace",
        (char*)"memory_access.txt"
    };
    int validInputCount = 13;

    ArgParser argParser(validInputCount, validInput);

    REQUIRE(argParser.validateArguments());
}