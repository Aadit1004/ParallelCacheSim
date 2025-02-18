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

CacheLine* Cache::findCacheLine(uint32_t t_address) {
    int index = extractIndex(t_address);
    int tag = extractTag(t_address);

    for (CacheLine& line : m_cache_sets[index]) {
        if (line.m_tag == tag && line.m_valid) {
            return &line;
        }
    }

    return nullptr;
}

void Cache::forwardToNextLevel(uint32_t t_address, bool t_isWrite, int t_value) {
    if (m_next_level_cache) {
        if (t_isWrite) {
            m_next_level_cache->write(t_address, t_value);
        } else {
            m_next_level_cache->read(t_address);
        }
    } else { // if there's no next level, access main memory
        if (t_isWrite) {
            // simulated_memory[t_address] = t_value;
        } else {
            // simulated_memory[t_address]; // read (no actual effect since memory isn't simulated)
        }
    }
}

void Cache::handleEviction(int t_index, int t_tag) {
    evictCacheLine(t_index);  // evict a line from the set

    // find an empty slot or overwrite evicted slot
    for (CacheLine& line : m_cache_sets[t_index]) {
        if (!line.m_valid) {  // found an invalid (empty) line
            line.m_tag = t_tag;
            line.m_valid = true;
            line.m_dirty = false;
            std::fill(std::begin(line.m_data), std::end(line.m_data), 0);  // init new block
            return;
        }
    }
}
