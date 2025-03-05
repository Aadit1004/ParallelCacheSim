#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include "../cache/cache.h"
#include "../cli/arg_parser.h"
#include "../io/file_manager.h"
#include "../memory/memory.h"

class CoreManager {

public:
    CoreManager(int t_num_threads, ValidParams* t_params, FileManager* t_fm, Memory& t_memory,  bool t_isVerbose);
    ~CoreManager();

    void startSimulation();
    void workerThread(int thread_id);
private:
    int num_threads;
    ValidParams* params;
    FileManager* fm;
    Memory memory;
    bool isVerbose;
    std::vector<std::thread> threads;
    std::vector<Cache*> L1_caches;
    std::vector<Cache*> L2_caches;
    std::vector<Cache*> L3_caches;
    std::mutex fm_mutex;
};