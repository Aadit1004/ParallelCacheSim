#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <climits>
#include "../memory/memory.h"

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
    int m_mesi_state = 0;

    CacheLine() = default;

    CacheLine(int tag) : m_tag(tag), m_valid(true), m_dirty(false) {
        std::fill(std::begin(m_data), std::end(m_data), 0);
    }
};

class Cache {

public:
    Cache(int t_cache_size, int t_associativity, std::string t_replacement_policy, std::string t_write_policy, Level t_cache_level, Cache* t_next_level, Memory& t_memory);
    void read(uint32_t t_address);
    void write(uint32_t t_address, int t_value);
    CacheLine* findCacheLine(uint32_t t_address);

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
    void evictCacheLine(int t_index, int t_tag);
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
};