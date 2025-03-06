#include "../catch2/catch.hpp"
#include "../src/threading/core_manager.h"
#include "../src/cli/arg_parser.h"
#include "../src/memory/memory.h"
#include "../src/io/file_manager.h"

const int memorySize = 16 * 1024 * 1024;

TEST_CASE("Core Manager - Correct Number of Caches", "[core_manager]") {
    auto [numThreads, expectedL1Size, expectedL2Size, expectedL3Size] = GENERATE(
        std::make_tuple(2, 2, 1, 1),
        std::make_tuple(10, 10, 5, 3),
        std::make_tuple(16, 16, 8, 4)
    );

    ValidParams params;
    params.l1_cache_size = 16 * 1024;
    params.l2_cache_size = 64 * 1024;
    params.l3_cache_size = 512 * 1024;
    params.memory_size = "medium";
    params.num_threads = numThreads;
    params.replacement_policy = "LRU";
    params.write_policy = "WB";
    params.access_file_name = "valid_file.txt";
    params.isVerbose = false;
    params.associativity = 1;

    Memory memory(memorySize, params.isVerbose);
    FileManager fm(params.access_file_name, params.isVerbose, true);
    REQUIRE(fm.isValidFile());
    fm.parseFile();
    CacheStats stats;
    CoreManager coreManager(params.num_threads, &params, &fm, memory, params.isVerbose, &stats);
    
    REQUIRE(coreManager.getNumL1Caches() == expectedL1Size);
    REQUIRE(coreManager.getNumL2Caches() == expectedL2Size);
    REQUIRE(coreManager.getNumL3Caches() == expectedL3Size);
}