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
// 8 = 8-Way Set-Associative
// 0 = fully associative
Cache::Cache(int t_cache_size, int t_associativity, std::string t_replacement_policy, 
    std::string t_write_policy, Level t_cache_level, Cache* t_next_level, Memory& t_memory, CacheStats* t_stats, bool isVerbose, CoreManager* t_core_manager) 
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
    m_stats(t_stats),
    m_isVerbose(isVerbose),
    m_core_manager(t_core_manager)
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
            m_stats->memory_accesses++;
        } else {
            m_memory.read(t_address); // read (no actual effect since memory isn't simulated so do nothing with value)
            m_stats->memory_accesses++;
        }
    }
}

void Cache::handleEviction(int t_index, int t_tag) {
    m_stats->evictions++;
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

    if (m_isVerbose) std::cerr << "[ERROR] Eviction failed: No available slots after eviction." << std::endl;
    throw CacheException("Eviction failed: No available slots after eviction.");
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
        m_stats->dirty_evictions++;
        uint32_t block_address = (evicted_line.m_tag << (m_index_bits + m_offset_bits)) | (t_index << m_offset_bits);
        for (size_t i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
            m_memory.write(block_address + (i * sizeof(int)), evicted_line.m_data[i]);
            m_stats->memory_accesses++;
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
        if (m_isVerbose) std::cerr << "[ERROR] Unaligned cache read at address 0x" << std::hex << t_address << std::dec << "\n";
        throw CacheException("Unaligned cache read.");
    }

    if (m_isVerbose) {
        std::cout << "[READ] Address: 0x" << std::hex << t_address << std::dec << std::endl;
    }

    if (m_cache_level == Level::L1) {
        m_stats->total_operations++;
        m_stats->read_operations++;
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
        if (m_cache_level == Level::L1) {
            m_stats->l1_hits++;
        } else if (m_cache_level == Level::L2) {
            m_stats->l2_hits++;
        } else if (m_cache_level == Level::L3) {
            m_stats->l3_hits++;
        }

        if (m_replacement_policy == "LRU") {
            updateLRU((m_associativity == 0) ? 0 : index, line);
        }

        if (m_core_manager != nullptr) {
            // if another core has this line in MESI_State::MODIFIED, it must downgrade it
            m_core_manager->downgradeModifiedToShared(t_address, this);
            updateMESI(t_address, MESI_State::SHARED);
        }

        return retrieved_value;

    }

    if (m_isVerbose) {
        std::cout << "[CACHE MISS] Address: 0x" << std::hex << t_address 
                  << " | Tag: " << tag << " | Index: " << index << std::dec << std::endl;
    }
    if (m_cache_level == Level::L1) {
        m_stats->l1_misses++;
    } else if (m_cache_level == Level::L2) {
        m_stats->l2_misses++;
    } else if (m_cache_level == Level::L3) {
        m_stats->l3_misses++;
    }

    forwardToNextLevel(t_address, false);

     // cache miss: fetch from next level (load block into cache)
     handleEviction(index, tag); // evict if needed
 
     line = findCacheLine(t_address);
     if (line == nullptr) { // safety check
        if (m_isVerbose) std::cerr << "[ERROR] Unexpected null cache line after eviction." << std::endl;
        throw CacheException("Unexpected null cache line after eviction in cache read.");
    }
 
     // fetch full block from memory
     uint32_t block_start_address = t_address & ~(defaults::BLOCK_SIZE - 1);
     for (long unsigned int i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
         line->m_data[i] = m_memory.read(block_start_address + (i * sizeof(int)));
         m_stats->memory_accesses++;
     }

     if (m_core_manager != nullptr) {
        updateMESI(t_address, MESI_State::EXCLUSIVE);
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
        if (m_isVerbose) std::cerr << "[ERROR] Unaligned cache write at address 0x" << std::hex << t_address << std::dec << "\n";
        throw CacheException("Unaligned cache write");
    }
    if (m_isVerbose) {
        std::cout << "[WRITE] Address: 0x" << std::hex << t_address
        << " | Value: " << t_value << std::dec << std::endl;
    }

    if (m_cache_level == Level::L1) {
        m_stats->total_operations++;
        m_stats->write_operations++;
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
        if (m_cache_level == Level::L1) {
            m_stats->l1_hits++;
        } else if (m_cache_level == Level::L2) {
            m_stats->l2_hits++;
        } else if (m_cache_level == Level::L3) {
            m_stats->l3_hits++;
        }

        if (m_core_manager != nullptr) {
            // if another core has this in MESI_State::MODIFIED, force WB
            m_core_manager->handleWriteBackBeforeInvalidation(t_address, this);
            m_core_manager->invalidateOtherCaches(t_address, this);
        }

        updateMESI(t_address, MESI_State::MODIFIED);

        if (m_write_policy == "WB") {
            line->m_dirty = true; // mark as modified for Write-Back
            if (m_isVerbose) {
                std::cout << "[WRITE BACK] Marking line as dirty\n";
            }
        } else { // WT
            m_memory.write(t_address, t_value); // WT writes immediately to memory
            m_stats->memory_accesses++;
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
    if (m_cache_level == Level::L1) {
        m_stats->l1_misses++;
    } else if (m_cache_level == Level::L2) {
        m_stats->l2_misses++;
    } else if (m_cache_level == Level::L3) {
        m_stats->l3_misses++;
    }

    // cache miss: get block from memory
    handleEviction(index, tag);
    line = findCacheLine(t_address);

    if (line == nullptr) {
        // should never happen, but safety check
        if (m_isVerbose) std::cerr << "[ERROR] Unexpected null cache line after eviction." << std::endl;
        throw CacheException("Unexpected null cache line after eviction in cache write.");
    }

    // fetch block from memory and store in cache
    uint32_t block_start_address = t_address & ~(defaults::BLOCK_SIZE - 1);
    for (long unsigned int i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
        line->m_data[i] = m_memory.read(block_start_address + (i * sizeof(int))); // read block size from memory
        m_stats->memory_accesses++;
    }

    // writing new value to line
    int word_offset = extractOffset(t_address) / sizeof(int);
    line->m_data[word_offset] = t_value;

    if (m_isVerbose) {
        std::cout << "[FETCH] Block loaded from memory into cache. Address Range: 0x" 
                  << std::hex << block_start_address << " - 0x" 
                  << (block_start_address + defaults::BLOCK_SIZE) << std::dec << std::endl;
    }

    if (m_core_manager != nullptr) {
        m_core_manager->invalidateOtherCaches(t_address, this);
    }

    updateMESI(t_address, MESI_State::MODIFIED);

    if (m_write_policy == "WB") {
        line->m_dirty = true;
        if (m_isVerbose) {
            std::cout << "[WRITE BACK] Marking line as dirty\n";
        }
    } else { // WT
        m_memory.write(t_address, t_value);
        m_stats->memory_accesses++;
        forwardToNextLevel(t_address, true, t_value);
        if (m_isVerbose) {
            std::cout << "[WRITE THROUGH] Value written to memory at address: 0x" 
                      << std::hex << t_address << std::dec << std::endl;
        }
    }
}

void Cache::flushCache() {
    for (size_t set_index = 0; set_index < m_cache_sets.size(); set_index++) {
        for (CacheLine& line : m_cache_sets[set_index]) {
            if (line.m_valid && line.m_dirty) {
                uint32_t block_address = (line.m_tag << (m_index_bits + m_offset_bits)) | (set_index << m_offset_bits);
                if (m_isVerbose) {
                    std::cout << "[FLUSH] Writing dirty cache line to memory | Address Range: 0x"
                              << std::hex << block_address << " - 0x" 
                              << (block_address + defaults::BLOCK_SIZE) << std::dec << std::endl;
                }
                for (size_t i = 0; i < defaults::BLOCK_SIZE / sizeof(int); i++) {
                    m_memory.write(block_address + (i * sizeof(int)), line.m_data[i]);
                    m_stats->memory_accesses++;
                }
                line.m_dirty = false;
            }
        }
    }
}

void Cache::updateMESI(uint32_t t_address, MESI_State new_state) {
    std::lock_guard<std::mutex> lock(cache_mutex);

    CacheLine* line = findCacheLine(t_address);
    if (!line) return;  // if the line is not found, we just return

    if (m_isVerbose) {
        std::cout << "[MESI] Updating MESI state for Address: 0x" << std::hex << t_address 
                  << " | Old: " << line->m_mesi_state << " -> New: " << new_state << std::dec << std::endl;
    }

    line->m_mesi_state = new_state;
}
