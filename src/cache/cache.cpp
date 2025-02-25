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
    std::string t_write_policy, Level t_cache_level, Cache* t_next_level, Memory& t_memory, bool isVerbose) 
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
    m_memory(t_memory),
    m_isVerbose(isVerbose)
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
    int index = (m_associativity == 0) ? 0 : extractIndex(t_address);
    int tag = extractTag(t_address);

    for (CacheLine& line : m_cache_sets[index]) {
        if (line.m_tag == tag && line.m_valid) {
            if (m_replacement_policy == "LRU" && m_associativity != 0) {
                updateLRU(index, &line);
            } else if (m_replacement_policy == "LRU" && m_associativity == 0) {
                updateLRU(0, &line);
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
        if (m_isVerbose) {
            std::cout << "[FORWARD] Address: 0x" << std::hex << t_address 
                      << " | Level: " << (m_cache_level == L1 ? "L1" : "L2") 
                      << " -> Next Level" << std::dec << std::endl;
        }
        if (t_isWrite) {
            m_next_level_cache->write(t_address, t_value);
        } else {
            m_next_level_cache->read(t_address);
        }
    } else { // if there's no next level, access main memory
        if (m_isVerbose) {
            std::cout << "[MEMORY ACCESS] Address: 0x" << std::hex << t_address 
                      << " | Type: " << (t_isWrite ? "Write" : "Read") << std::dec << std::endl;
        }
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
            line.m_lru_age = 1;
            std::fill(std::begin(line.m_data), std::end(line.m_data), 0);  // init new block

            if (m_isVerbose) {
                std::cout << "[ALLOCATE] New Block Assigned | Index: " << t_index 
                          << " | Tag: " << t_tag << std::endl;
            }

            return;
        }
    }

    if (m_isVerbose) {
        std::cout << "[EVICT] No Free Line | Evicting from Index: " << t_index << std::endl;
    }

    evictCacheLine(t_index);  // evict a line from the set

    // find an empty slot or overwrite evicted slot
    for (CacheLine& line : m_cache_sets[t_index]) {
        if (!line.m_valid) {  // found an invalid (empty) line
            line.m_tag = t_tag;
            line.m_valid = true;
            line.m_dirty = false;
            line.m_lfu_counter = 1;
            line.m_lru_age = 1;
            std::fill(std::begin(line.m_data), std::end(line.m_data), 0);  // init new block
            return;
        }
    }

    std::cerr << "[ERROR] Eviction failed: No available slots after eviction!" << std::endl;
}


void Cache::evictCacheLine(int t_index) {
    int evict_index = 0;
    int num_lines = (m_associativity == 0) ? m_cache_sets[0].size() : m_associativity;

    if (m_replacement_policy == "FIFO") {
        evict_index = m_fifo_ptr[t_index];
        m_fifo_ptr[t_index] = (m_fifo_ptr[t_index] + 1) % num_lines;
    } else if (m_replacement_policy == "LRU") {
        int max_lru = -1;
        for (int i = 0; i < num_lines; i++) {
            CacheLine& line = (m_associativity == 0) ? m_cache_sets[0][i] : m_cache_sets[t_index][i];
            if (line.m_valid && line.m_lru_age > max_lru) {
                max_lru = line.m_lru_age;
                evict_index = i; // evict index is most lru
            }
        }
    } else if (m_replacement_policy == "LFU") {
        int min_lfu = INT_MAX;
        for (int i = 0; i < num_lines; i++) {
            CacheLine& line = (m_associativity == 0) ? m_cache_sets[0][i] : m_cache_sets[t_index][i];
            if (line.m_valid && line.m_lfu_counter < min_lfu) {
                min_lfu = line.m_lfu_counter;
                evict_index = i;
            }
        }
    }

    // If WB, write dirty block to memory
    CacheLine& evicted_line = (m_associativity == 0) ? m_cache_sets[0][evict_index] : m_cache_sets[t_index][evict_index];

    if (m_isVerbose) {
        std::cout << "[EVICT] Policy: " << m_replacement_policy 
                  << " | Set Index: " << t_index
                  << " | Line: " << evict_index 
                  << " | Dirty: " << (evicted_line.m_dirty ? "true" : "false") 
                  << std::endl;
    }

    if (evicted_line.m_valid && evicted_line.m_dirty && m_write_policy == "WB") {
        uint32_t block_address = (evicted_line.m_tag << (m_index_bits + m_offset_bits)) | (t_index << m_offset_bits);
        for (size_t i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
            m_memory.write(block_address + (i * sizeof(int)), evicted_line.m_data[i]);
        }
        evicted_line.m_dirty = false;
    }

    // invalidate cache line
    evicted_line.m_valid = false;
    evicted_line.m_dirty = false;
}

void Cache::updateLRU(int t_index, CacheLine* accessedLine) {
    int max_lru = 0;
    
    for (CacheLine& line : m_cache_sets[t_index]) {
        if (line.m_valid && &line != accessedLine) {
            line.m_lru_age++;  // increment all other valid lines
            max_lru = std::max(max_lru, line.m_lru_age);
        }
    }
}

int Cache::read(uint32_t t_address) {
    if (t_address % sizeof(int) != 0) {
        std::cerr << "[ERROR] Unaligned cache read at address 0x" << std::hex << t_address << std::dec << "\n";
        return 0; // TODO: some invalid value indication ?? throw exception maybe
    }

    if (m_isVerbose) {
        std::cout << "[READ] Address: 0x" << std::hex << t_address << std::dec << std::endl;
    }

    int index = extractIndex(t_address);
     int tag = extractTag(t_address);

    CacheLine* line = findCacheLine(t_address);
    if (line != nullptr) {
        // We found the line and hit, TODO: log hit
        int value_offset = extractOffset(t_address) / sizeof(int);
        int retrieved_value = line->m_data[value_offset];

        if (m_isVerbose) {
            std::cout << "[CACHE HIT] Address: 0x" << std::hex << t_address 
                      << " | Index: " << index << " | Offset: " << value_offset 
                      << " | Value: " << retrieved_value << std::dec << std::endl;
        }

        if (m_replacement_policy == "LRU") {
            updateLRU((m_associativity == 0) ? 0 : index, line);
        }

        return retrieved_value;

    }

    if (m_isVerbose) {
        std::cout << "[CACHE MISS] Address: 0x" << std::hex << t_address 
                  << " | Tag: " << tag << " | Index: " << index << std::dec << std::endl;
    }

    forwardToNextLevel(t_address, false);

     // cache miss: fetch from next level (load block into cache)
     handleEviction(index, tag); // evict if needed
 
     line = findCacheLine(t_address);
     if (line == nullptr) { 
        std::cerr << "[ERROR] Unexpected null cache line after eviction." << std::endl;
        // throw exception
        return 0; // safety check
    }
 
     // fetch full block from memory
     uint32_t block_start_address = t_address & ~(defaults::BLOCK_SIZE - 1);
     for (long unsigned int i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
         line->m_data[i] = m_memory.read(block_start_address + (i * sizeof(int)));
     }

     if (m_isVerbose) {
        std::cout << "[FETCH] Block loaded from memory into cache. Address Range: 0x" 
                  << std::hex << block_start_address << " - 0x" 
                  << (block_start_address + defaults::BLOCK_SIZE) << std::dec << std::endl;
    }

    int value_offset = extractOffset(t_address) / sizeof(int);
    int retrieved_value = line->m_data[value_offset];

    if (m_isVerbose) {
        std::cout << "[READ COMPLETE] Retrieved Value: " << retrieved_value 
                  << " from Address: 0x" << std::hex << t_address << std::dec << std::endl;
    }
    return retrieved_value;
}

void Cache::write(uint32_t t_address, int t_value) {
    if (t_address % sizeof(int) != 0) {
        std::cerr << "[ERROR] Unaligned cache write at address 0x" << std::hex << t_address << std::dec << "\n";
        // TODO: above will throw an exception
        return;
    }
    if (m_isVerbose) {
        std::cout << "[WRITE] Address: 0x" << std::hex << t_address
        << " | Value: " << t_value << std::dec << std::endl;
    }

    int index = extractIndex(t_address);
    int tag = extractTag(t_address);

    CacheLine* line = findCacheLine(t_address);
    if (line != nullptr) { // cache hit: update the value
        int word_offset = extractOffset(t_address) / sizeof(int);
        line->m_data[word_offset] = t_value;

        if (m_isVerbose) {
            std::cout << "[CACHE HIT] Value updated at Index: " << index 
            << " | Offset: " << word_offset << " | New Value: " << t_value << std::endl;
        }

        if (m_write_policy == "WB") {
            line->m_dirty = true; // mark as modified for Write-Back
            if (m_isVerbose) {
                std::cout << "[WRITE BACK] Marking line as dirty\n";
            }
        } else { // WT
            m_memory.write(t_address, t_value); // WT writes immediately to memory
            forwardToNextLevel(t_address, true, t_value);
            if (m_isVerbose) {
                std::cout << "[WRITE THROUGH] Value written to memory at address: 0x" 
                          << std::hex << t_address << std::dec << std::endl;
            }
        }

        if (m_replacement_policy == "LRU") {
            updateLRU((m_associativity == 0) ? 0 : index, line);
        }
        return;
    }

    if (m_isVerbose) {
        std::cout << "[CACHE MISS] Address: 0x" << std::hex << t_address 
                  << " | Tag: " << tag << " | Index: " << index << std::dec << std::endl;
    }

    // cache miss: get block from memory
    handleEviction(index, tag);
    line = findCacheLine(t_address);

    if (line == nullptr) {
        // should never happen, but safety check
        std::cerr << "[ERROR] Unexpected null cache line after eviction." << std::endl; // will replace to throwing exception
        return;
    }

    // fetch block from memory and store in cache
    uint32_t block_start_address = t_address & ~(defaults::BLOCK_SIZE - 1);
    for (long unsigned int i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
        line->m_data[i] = m_memory.read(block_start_address + (i * sizeof(int))); // read block size from memory
    }

    // writing new value to line
    int word_offset = extractOffset(t_address) / sizeof(int);
    line->m_data[word_offset] = t_value;

    if (m_isVerbose) {
        std::cout << "[FETCH] Block loaded from memory into cache. Address Range: 0x" 
                  << std::hex << block_start_address << " - 0x" 
                  << (block_start_address + defaults::BLOCK_SIZE) << std::dec << std::endl;
    }

    if (m_write_policy == "WB") {
        line->m_dirty = true;
        if (m_isVerbose) {
            std::cout << "[WRITE BACK] Marking line as dirty\n";
        }
    } else { // WT
        m_memory.write(t_address, t_value);
        forwardToNextLevel(t_address, true, t_value);
        if (m_isVerbose) {
            std::cout << "[WRITE THROUGH] Value written to memory at address: 0x" 
                      << std::hex << t_address << std::dec << std::endl;
        }
    }
}