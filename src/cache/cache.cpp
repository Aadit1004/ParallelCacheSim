#include "cache.h"

// address => [tag (t bits) | set index (s bits) | block offset (b bits)]
// t bits, s bits, b bits
// B block size => b = log_2(B)
// S num sets => s = log_2(S)
// l total length bits => t = l - b- s

// associativity (num cache lines per set):
// 1 = directly mapped
// 4 = 4-Way Set-Associative
// -1 = fully associative
Cache::Cache(int t_cache_size_kb, int t_associativity, std::string t_replacement_policy, std::string t_write_policy, Level t_cache_level, Cache* t_next_level) {
    m_cache_size_kb = t_cache_size_kb;
    m_associativity = t_associativity;
    m_replacement_policy = t_replacement_policy;
    m_write_policy = t_write_policy;
    m_cache_level = t_cache_level;
    m_next_level_cache = t_next_level;
    m_num_sets = calculateNumberSets();

    m_offset_bits = static_cast<int>(log2(defaults::BLOCK_SIZE));
    m_index_bits = static_cast<int>(log2(m_num_sets));
    m_tag_bits = defaults::ADDRESS_BITS - m_index_bits - m_offset_bits;

    if (m_associativity == -1) {
        m_cache_sets = std::vector<std::vector<CacheLine>>(1, std::vector<CacheLine>(m_cache_size_kb * 1024 / defaults::BLOCK_SIZE));
    } else {
        m_cache_sets = std::vector<std::vector<CacheLine>>(m_num_sets, std::vector<CacheLine>(m_associativity));
    }
}

// num sets = (total cache size / (block size * associativity)) || 1
int Cache::calculateNumberSets() {
    const int cache_size_in_bytes = m_cache_size_kb * 1024;
    if (m_associativity == -1) {
        return 1;
    } else {
        return cache_size_in_bytes / (defaults::BLOCK_SIZE * m_associativity);
    }
}

int Cache::extractOffset(uint32_t t_address) {
    return t_address & ((1 << m_offset_bits) - 1);
}

int Cache::extractIndex(uint32_t t_address) {
    return (t_address >> m_offset_bits) & ((1 << m_index_bits) - 1);
}

int Cache::extractTag(uint32_t t_address) {
    return (t_address >> (m_offset_bits + m_index_bits));
}