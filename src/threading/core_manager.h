#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include "../cache/cache.h"
#include "../cache/mesi.h"
#include "../cli/arg_parser.h"
#include "../io/file_manager.h"
#include "../memory/memory.h"

// forwarding declarations to avoid circular dependency issues
class Cache;
struct CacheStats;

class CoreManager {

public:
    CoreManager(int t_num_threads, ValidParams* t_params, FileManager* t_fm, Memory& t_memory,  bool t_isVerbose, CacheStats* t_stats);
    ~CoreManager();

    void startSimulation();
    void workerThread(int thread_id);
    void invalidateOtherCaches(uint32_t address, Cache* requester);
    void downgradeModifiedToShared(uint32_t address, Cache* requester);
    void handleWriteBackBeforeInvalidation(uint32_t address, Cache* requester);

    // for testing
    int getNumL1Caches() const { return L1_caches.size(); }
    int getNumL2Caches() const { return L2_caches.size(); }
    int getNumL3Caches() const { return L3_caches.size(); }
private:
    int num_threads;
    ValidParams* params;
    FileManager* fm;
    Memory memory;
    bool isVerbose;
    CacheStats* m_stats;
    std::vector<std::thread> threads;
    std::vector<Cache*> L1_caches;
    std::vector<Cache*> L2_caches;
    std::vector<Cache*> L3_caches;
    std::mutex fm_mutex;
};