#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <climits>
#include "../memory/memory.h"
#include "../exception/cache_exception.h"
#include "mesi.h"

enum Level {
    L1,
    L2,
    L3
};

namespace defaults
{
    static constexpr int BLOCK_SIZE = 16;
    static const int ADDRESS_BITS = sizeof(uint32_t) * 8;
}

struct CacheLine {
    int m_tag;
    int m_data[defaults::BLOCK_SIZE / sizeof(int)];
    bool m_valid = false;
    bool m_dirty = false;
    int m_lru_age = 0;
    int m_lfu_counter = 0;
    MESI_State m_mesi_state = MESI_State::INVALID;

    CacheLine() = default;

    CacheLine(int tag) : m_tag(tag), m_valid(true), m_dirty(false), m_mesi_state(MESI_State::INVALID) {
        std::fill(std::begin(m_data), std::end(m_data), 0);
    }
};

struct CacheStats {
    int total_operations = 0;
    int read_operations = 0;
    int write_operations = 0;
    int cache_hits = 0;
    int cache_misses = 0;
    int read_hits = 0;
    int write_hits = 0;
    int read_misses = 0;
    int write_misses = 0;
    int evictions = 0;
    int dirty_evictions = 0;
    int memory_accesses = 0;

    void printSummary() const {
        std::cout << "\n===== Cache Simulation Summary =====\n";
        std::cout << "Total Operations: " << total_operations << "\n";
        std::cout << "Read Operations: " << read_operations << "\n";
        std::cout << "Write Operations: " << write_operations << "\n";
        std::cout << "Cache Hits: " << cache_hits << " (" << (100.0 * cache_hits / total_operations) << "%)\n";
        std::cout << "Cache Misses: " << cache_misses << " (" << (100.0 * cache_misses / total_operations) << "%)\n";
        std::cout << "Read Hits: " << read_hits << "\n";
        std::cout << "Write Hits: " << write_hits << "\n";
        std::cout << "Read Misses: " << read_misses << "\n";
        std::cout << "Write Misses: " << write_misses << "\n";
        std::cout << "Evictions: " << evictions << "\n";
        std::cout << "Dirty Evictions: " << dirty_evictions << "\n";
        std::cout << "Memory Accesses: " << memory_accesses << "\n";
        std::cout << "====================================\n";
    }
};

class Cache {

public:
    Cache(int t_cache_size, int t_associativity, std::string t_replacement_policy, std::string t_write_policy, Level t_cache_level, Cache* t_next_level, Memory& t_memory, bool isVerbose = false);
    int read(uint32_t t_address);
    void write(uint32_t t_address, int t_value);
    CacheLine* findCacheLine(uint32_t t_address);
    void flushCache();

    // public getters for testing
    int getOffsetBits() const { return m_offset_bits; }
    int getIndexBits() const { return m_index_bits; }
    int getTagBits() const { return m_tag_bits; }
    int getNumSets() const { return m_num_sets; }
    std::string getReplacementPolicy() const {return m_replacement_policy; } 

private:
    int calculateNumberSets() const;
    int extractTag(uint32_t t_address) const;
    int extractIndex(uint32_t t_address) const;
    int extractOffset(uint32_t t_address) const;
    void updateLRU(int t_index, CacheLine* accessedLine);    
    void evictCacheLine(int t_index);
    void handleEviction(int t_index, int t_tag);
    void forwardToNextLevel(uint32_t t_address, bool t_isWrite, int t_value = 0);

    std::string m_replacement_policy;
    int m_cache_size;
    int m_associativity;
    int m_num_sets;
    int m_offset_bits;
    int m_index_bits;
    int m_tag_bits;
    std::vector<int> m_fifo_ptr;
    std::string m_write_policy; // "WB" or "WT"
    std::vector<std::vector<CacheLine>> m_cache_sets;
    Cache* m_next_level_cache; // pointer to next cache line L1->L2->L3
    Level m_cache_level;
    Memory& m_memory;
    bool m_isVerbose;
};