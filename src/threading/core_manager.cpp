#include "core_manager.h"

CoreManager::CoreManager(int t_num_threads, ValidParams* t_params, FileManager* t_fm, Memory& t_memory,  bool t_isVerbose) : 
    num_threads(t_num_threads), params(t_params), fm(t_fm), memory(t_memory), isVerbose(t_isVerbose) {
    threads.resize(num_threads);

    L3_caches.resize(num_threads / 4, nullptr);
    for (size_t i = 0; i < L3_caches.size(); i++) {
        L3_caches[i] = new Cache(params->l3_cache_size, params->associativity, params->replacement_policy, params->write_policy, L3, nullptr, memory, params->isVerbose);
    }
    if (isVerbose) {
        std::cout << "[Core Manager] Initialized " << L3_caches.size() << " L3 Caches" << std::endl;
    }
    L2_caches.resize(num_threads / 2, nullptr);
    for (size_t j = 0; j < L2_caches.size(); j++) {
        L2_caches[j] = new Cache(params->l2_cache_size, params->associativity, params->replacement_policy, params->write_policy, L2, L3_caches[j/2], memory, params->isVerbose);
    }
    if (isVerbose) {
        std::cout << "[Core Manager] Initialized " << L2_caches.size() << " L2 Caches" << std::endl;
    }
    L1_caches.resize(num_threads, nullptr);
    for (size_t k = 0; k < L1_caches.size(); k++) {
        L1_caches[k] = new Cache(params->l1_cache_size, params->associativity, params->replacement_policy, params->write_policy, L1, L2_caches[k/2], memory, params->isVerbose);
    }
    if (isVerbose) {
        std::cout << "[Core Manager] Initialized " << L1_caches.size() << " L1 Caches" << std::endl;
    }
}

CoreManager::~CoreManager() {
    for (Cache* L1_cache : L1_caches) delete L1_cache;
    for (Cache* L2_cache : L2_caches) delete L2_cache;
    for (Cache* L3_cache : L3_caches) delete L3_cache;

    L1_caches.clear();
    L2_caches.clear();
    L3_caches.clear();
}

void CoreManager::startSimulation() {
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(&CoreManager::workerThread, this, i);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }

    for (Cache* L1_cache : L1_caches) L1_cache->flushCache();
    for (Cache* L2_cache : L2_caches) L2_cache->flushCache();
    for (Cache* L3_cache : L3_caches) L3_cache->flushCache();
}

void CoreManager::workerThread(int thread_id) {
    Cache* L1_cache = L1_caches[thread_id];
    while (true) {
        std::optional<MemoryRequest> opt_request;
        {
            std::lock_guard<std::mutex> lock(fm_mutex);
            if (fm->getNumOperations() == 0) break;
            opt_request = fm->getNextRequest();
        }
        if (opt_request.has_value()) {
            MemoryRequest request = opt_request.value();
            if (request.type == AccessType::READ) {
                int value = L1_cache->read(request.address);
                if (isVerbose) {
                    std::cout << "[CORE " << thread_id << "] Read Address: 0x" << std::hex 
                              << request.address << " | Value: " << value << std::dec << std::endl;
                }
            } else {
                L1_cache->write(request.address, request.value);
                if (isVerbose) {
                    std::cout << "[CORE " << thread_id << "] Wrote Address: 0x" << std::hex 
                              << request.address << " | Value: " << request.value << std::dec << std::endl;
                }
            }
        } else {
            throw CacheException("Error retrieving next memory request.");
        }
    }
}