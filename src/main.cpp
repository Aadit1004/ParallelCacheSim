#include <iostream>
#include <unordered_map>
#include <chrono>
#include "cli/arg_parser.h"
#include "memory/memory.h"
#include "io/file_manager.h"
#include "cache/cache.h"
#include "threading/core_manager.h"

int getMemorySize(std::string& t_size) {
    if (t_size == "small") { 
        return 4 * 1024 * 1024;  // 4MB
    } else if (t_size == "medium") { 
        return 16 * 1024 * 1024;  // 16MB
    } else {
        return 64 * 1024 * 1024;  // 64MB
    }
}

void handleSingleThread(ValidParams& params, Memory& memory, FileManager& fm, CacheStats* stats) {
    Cache* L3_cache = new Cache(params.l3_cache_size, params.associativity, params.replacement_policy, params.write_policy, L3, nullptr, memory, stats, params.isVerbose);
    Cache* L2_cache = new Cache(params.l2_cache_size, params.associativity, params.replacement_policy, params.write_policy, L2, L3_cache, memory, stats, params.isVerbose);
    Cache* L1_cache = new Cache(params.l1_cache_size, params.associativity, params.replacement_policy, params.write_policy, L1, L2_cache, memory, stats, params.isVerbose, nullptr);
    while (fm.getNumOperations() != 0) {
        std::optional<MemoryRequest> opt_request = fm.getNextRequest();
        if (opt_request.has_value()) {
            MemoryRequest request = opt_request.value();
            if (request.type == AccessType::READ) {
                L1_cache->read(request.address);
            } else {
                L1_cache->write(request.address, request.value);
            }
        } else {
            throw CacheException("Error retrieving next memory request.");
        }
    }
    if (params.write_policy == "WB") {
        L1_cache->flushCache();
        L2_cache->flushCache();
        L3_cache->flushCache();
    }
    delete L1_cache;
    delete L2_cache;
    delete L3_cache;
}

int main(int argc, char *argv[]) {
    try {
        ArgParser argParser(argc, argv);
        if (argParser.validateArguments()) {
            std::cout << "===== Valid argmunets passed =====" << std::endl;
            ValidParams params = argParser.getValidParams();

            int memory_size_bytes = getMemorySize(params.memory_size);
            Memory memory(memory_size_bytes, params.isVerbose);
            std::cout << "L1 Cache Size: " << (params.l1_cache_size / 1024) << " MB" <<std::endl;
            std::cout << "L2 Cache Size: " << (params.l2_cache_size / 1024) << " MB" <<std::endl;
            std::cout << "L3 Cache Size: " << (params.l3_cache_size / 1024) << " MB" <<std::endl;
            FileManager fm(params.access_file_name, params.isVerbose);
            if (fm.isValidFile()) {
                fm.parseFile();
                CacheStats stats;
                if (params.num_threads == 1) {
                    auto t1 = std::chrono::high_resolution_clock::now();
                    handleSingleThread(params, memory, fm, &stats);
                    auto t2 = std::chrono::high_resolution_clock::now();
                    std::cout << "time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << "ms" << std::endl;
                    stats.printSummary();
                } else {
                    CoreManager* core_manager = new CoreManager(params.num_threads, &params, &fm, memory, params.isVerbose, &stats);
                    auto t1 = std::chrono::high_resolution_clock::now();
                    core_manager->startSimulation();
                    auto t2 = std::chrono::high_resolution_clock::now();
                    std::cout << "time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count() << "ms" << std::endl;
                    stats.printSummary();
                    delete core_manager;
                }
            } else {
                std::cout << "Invalid file - Not a .txt extension or located in examples/" << std::endl;
            }
        } else {
            std::cout << "Invalid argmunets passed" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "\n===== Cache Simulation Complete =====" << std::endl;
    return EXIT_SUCCESS;
}