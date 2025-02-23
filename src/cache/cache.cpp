#include "cache.h"

#include <utility>

// address => [tag (t bits) | set index (s bits) | block offset (b bits)]
// t bits, s bits, b bits
// B block size => b = log_2(B)
// S num sets => s = log_2(S)
// l total length bits => t = l - b- s

// associativity (num cache lines per set):
// 1 = directly mapped
// 4 = 4-Way Set-Associative
// 0 = fully associative
Cache::Cache(int t_cache_size, int t_associativity, std::string t_replacement_policy, 
    std::string t_write_policy, Level t_cache_level, Cache* t_next_level, Memory& t_memory) 
    : m_replacement_policy(std::move(t_replacement_policy)),
    m_cache_size(t_cache_size),
    m_associativity(t_associativity),
    m_num_sets(calculateNumberSets()),
    m_offset_bits(static_cast<int>(log2(defaults::BLOCK_SIZE))),
    m_index_bits(static_cast<int>(log2(m_num_sets))),
    m_tag_bits(defaults::ADDRESS_BITS - m_index_bits - m_offset_bits),
    m_fifo_ptr(m_num_sets, 0),
    m_write_policy(std::move(t_write_policy)),
    
    m_next_level_cache(t_next_level),
    m_cache_level(t_cache_level),
    m_memory(t_memory)
    {
    if (m_associativity == 0) {
        m_cache_sets = std::vector<std::vector<CacheLine>>(1, std::vector<CacheLine>(m_cache_size / defaults::BLOCK_SIZE));
    } else {
        m_cache_sets = std::vector<std::vector<CacheLine>>(m_num_sets, std::vector<CacheLine>(m_associativity));
    }
}

// num sets = (total cache size / (block size * associativity)) || 1
int Cache::calculateNumberSets() const {
    if (m_associativity == 0) {
        return 1;
    } 
    return m_cache_size / (defaults::BLOCK_SIZE * m_associativity);
}

int Cache::extractOffset(uint32_t t_address) const {
    return t_address & ((1 << m_offset_bits) - 1);
}

int Cache::extractIndex(uint32_t t_address) const {
    return (t_address >> m_offset_bits) & ((1 << m_index_bits) - 1);
}

int Cache::extractTag(uint32_t t_address) const {
    return (t_address >> (m_offset_bits + m_index_bits));
}

CacheLine* Cache::findCacheLine(uint32_t t_address) {
    int index = extractIndex(t_address);
    int tag = extractTag(t_address);

    for (CacheLine& line : m_cache_sets[index]) {
        if (line.m_tag == tag && line.m_valid) {
            if (m_replacement_policy == "LRU") {
                updateLRU(index, &line);
            } else if (m_replacement_policy == "LFU") {
                line.m_lfu_counter++; 
            }
            return &line;
        }
    }

    return nullptr;
}

void Cache::forwardToNextLevel(uint32_t t_address, bool t_isWrite, int t_value) {
    if (m_next_level_cache != nullptr) {
        if (t_isWrite) {
            m_next_level_cache->write(t_address, t_value);
        } else {
            m_next_level_cache->read(t_address);
        }
    } else { // if there's no next level, access main memory
        if (t_isWrite) {
            m_memory.write(t_address, t_value);
        } else {
            m_memory.read(t_address); // read (no actual effect since memory isn't simulated so do nothing with value)
        }
    }
}

void Cache::handleEviction(int t_index, int t_tag) {
    for (CacheLine& line : m_cache_sets[t_index]) {
        if (!line.m_valid) {  // found an invalid (empty) line
            line.m_tag = t_tag;
            line.m_valid = true;
            line.m_dirty = false;
            line.m_lfu_counter = 1;
            line.m_lru_age = 0;
            std::fill(std::begin(line.m_data), std::end(line.m_data), 0);  // init new block
            return;
        }
    }

    evictCacheLine(t_index, t_tag);  // evict a line from the set

    // find an empty slot or overwrite evicted slot
    for (CacheLine& line : m_cache_sets[t_index]) {
        if (!line.m_valid) {  // found an invalid (empty) line
            line.m_tag = t_tag;
            line.m_valid = true;
            line.m_dirty = false;
            line.m_lfu_counter = 1;
            line.m_lru_age = 0;
            std::fill(std::begin(line.m_data), std::end(line.m_data), 0);  // init new block
            return;
        }
    }

    std::cerr << "[ERROR] Eviction failed: No available slots after eviction!" << std::endl;
}


void Cache::evictCacheLine(int t_index, int t_tag) {
    int evict_index = 0;


    if (m_associativity == 0) { // Fully associative case
        int max_lru = -1;
        for (long unsigned int i = 0; i < m_cache_sets[0].size(); i++) {
            if (m_cache_sets[0][i].m_valid && m_cache_sets[0][i].m_lru_age > max_lru) {
                max_lru = m_cache_sets[0][i].m_lru_age;
                evict_index = i;
            }
        }
    } else if (m_replacement_policy == "FIFO") {
        evict_index = m_fifo_ptr[t_index];
        m_fifo_ptr[t_index] = (m_fifo_ptr[t_index] + 1) % m_associativity;
    } else if (m_replacement_policy == "LRU") {
        int max_lru = -1;
        for (int i = 0; i < m_associativity; i++) {
            if (m_cache_sets[t_index][i].m_valid && m_cache_sets[t_index][i].m_lru_age > max_lru) {
                max_lru = m_cache_sets[t_index][i].m_lru_age;
                evict_index = i; // evict index is most lru
            }
        }
    } else if (m_replacement_policy == "LFU") {
        int min_lfu = INT_MAX;
        for (int i = 0; i < m_associativity; i++) {
            if (m_cache_sets[t_index][i].m_valid && m_cache_sets[t_index][i].m_lfu_counter < min_lfu) {
                min_lfu = m_cache_sets[t_index][i].m_lfu_counter;
                evict_index = i;
            }
        }
    }

    // If WB, write dirty block to memory
    CacheLine& evicted_line = m_cache_sets[t_index][evict_index];
    if (evicted_line.m_valid && evicted_line.m_dirty && m_write_policy == "WB") {
        uint32_t block_address = (evicted_line.m_tag << (m_index_bits + m_offset_bits)) | (t_index << m_offset_bits);
        for (size_t i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
            m_memory.write(block_address + (i * sizeof(int)), evicted_line.m_data[i]);
        }
        evicted_line.m_dirty = false;
    } else if (m_write_policy == "WT") {
        
    }

    // invalidate cache line
    evicted_line.m_valid = false;
    evicted_line.m_dirty = false;
    evicted_line.m_tag = t_tag;
    evicted_line.m_lfu_counter = 1;
    evicted_line.m_lru_age = 0;
    std::fill(std::begin(evicted_line.m_data), std::end(evicted_line.m_data), 0);
}

void Cache::updateLRU(int t_index, CacheLine* accessedLine) {
    for (CacheLine& line : m_cache_sets[t_index]) {
        if (line.m_valid && &line != accessedLine) {
            line.m_lru_age++;  // increment counter for all valid lines
        }
    }
    accessedLine->m_lru_age = 0;  // reset counter for most recently used
}

void Cache::read(uint32_t t_address) {
    if (t_address % sizeof(int) != 0) {
        // std::cerr << "[ERROR] Unaligned cache read at address 0x" << std::hex << t_address << std::dec << "\n";
        return;
    }

    CacheLine* line = findCacheLine(t_address);
    if (line != nullptr) {
       // We found the line and hit, TODO: log hit
        return;
    }

    forwardToNextLevel(t_address, false);

     // cache miss: fetch from next level (load block into cache)
     int index = extractIndex(t_address);
     int tag = extractTag(t_address);
     handleEviction(index, tag); // evict if needed
 
     line = findCacheLine(t_address);
     if (line == nullptr) { return; // safety check
}
 
     // fetch full block from memory
     uint32_t block_start_address = t_address & ~(defaults::BLOCK_SIZE - 1);
     for (long unsigned int i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
         line->m_data[i] = m_memory.read(block_start_address + (i * sizeof(int)));
     }
}

void Cache::write(uint32_t t_address, int t_value) {
    if (t_address % sizeof(int) != 0) {
        // std::cerr << "[ERROR] Unaligned cache write at address 0x" << std::hex << t_address << std::dec << "\n";
        return;
    }

    int index = extractIndex(t_address);
    int tag = extractTag(t_address);

    CacheLine* line = findCacheLine(t_address);
    if (line != nullptr) { // cache hit: update the value
        int word_offset = extractOffset(t_address) / sizeof(int);
        line->m_data[word_offset] = t_value;

        if (m_write_policy == "WB") {
            line->m_dirty = true; // mark as modified for Write-Back
        } else { // WT
            m_memory.write(t_address, t_value); // WT writes immediately to memory
            forwardToNextLevel(t_address, true, t_value);
        }
        return;
    }

    // cache miss: get block from memory
    handleEviction(index, tag);
    line = findCacheLine(t_address);

    if (line == nullptr) {
        // should never happen, but safety check
        return;
    }

    // fetch block from memory and store in cache
    uint32_t block_start_address = t_address & ~(defaults::BLOCK_SIZE - 1);
    for (long unsigned int i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
        line->m_data[i] = m_memory.read(block_start_address + (i * sizeof(int)));
    }

    // writing new value to line
    int word_offset = extractOffset(t_address) / sizeof(int);
    line->m_data[word_offset] = t_value;

    if (m_write_policy == "WB") {
        line->m_dirty = true;
    } else { // WT
        m_memory.write(t_address, t_value);
        forwardToNextLevel(t_address, true, t_value);
    }
}