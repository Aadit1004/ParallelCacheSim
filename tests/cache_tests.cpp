#include "../catch2/catch.hpp"
#include "../src/cache/cache.h"
#include "../src/memory/memory.h"

const int memorySize = 4 * 1024 * 1024;

TEST_CASE("Cache Initialization", "[cache]") {
    Memory memory(memorySize);
    int cache_size = 8 * 1024;
    auto associativity = GENERATE(1, 4, 0);
    Cache cache(cache_size, associativity, "LRU", "WB", L1, nullptr, memory);
    
    int expected_offset_bits = static_cast<int>(log2(defaults::BLOCK_SIZE));
    int num_sets = (associativity == 0) ? 1 : (8 * 1024) / (defaults::BLOCK_SIZE * associativity);
    int expected_index_bits = static_cast<int>(log2(num_sets));
    int expected_tag_bits = defaults::ADDRESS_BITS - expected_index_bits - expected_offset_bits;

    REQUIRE(cache.getOffsetBits() == expected_offset_bits);
    REQUIRE(cache.getIndexBits() == expected_index_bits);
    REQUIRE(cache.getTagBits() == expected_tag_bits);
    REQUIRE(cache.getNumSets() == num_sets);
}

TEST_CASE("Cache - findCacheLine() behavior", "[cache]") {
    Memory memory(memorySize);
    std::string replacementPolicy = GENERATE("LRU", "LFU", "FIFO");
    Cache cache(8 * 1024, 4, replacementPolicy, "WB", L1, nullptr, memory);

    uint32_t test_address = 0x1000;
    int expected_tag = test_address >> (cache.getOffsetBits() + cache.getIndexBits());

    // should be cache miss
    REQUIRE(cache.findCacheLine(test_address) == nullptr);

    cache.write(test_address, 42); // cache should have this address stored

    CacheLine* found_line = cache.findCacheLine(test_address);
    REQUIRE(found_line != nullptr);
    REQUIRE(found_line->m_valid == true);
    REQUIRE(found_line->m_tag == expected_tag);

 
    if (cache.getReplacementPolicy() == "LRU") {
        REQUIRE(found_line->m_lru_age == 0);
    }

    if (cache.getReplacementPolicy() == "LFU") {
        int initial_lfu_count = found_line->m_lfu_counter;
        cache.findCacheLine(test_address);
        REQUIRE(found_line->m_lfu_counter == initial_lfu_count + 1);
    }

    if (cache.getReplacementPolicy() == "FIFO") {
        uint32_t addr1 = 0x2000;
        uint32_t addr2 = 0x3000;
        uint32_t addr3 = 0x4000;
        uint32_t addr4 = 0x5000;  // tthis address should evict test_address

        cache.write(addr1, 10);
        cache.write(addr2, 20);
        cache.write(addr3, 30);
        cache.write(addr4, 40);  // FIFO should evict test_address

        REQUIRE(cache.findCacheLine(test_address) == nullptr); // test_address should be evicted
        REQUIRE(cache.findCacheLine(addr1) != nullptr);
        REQUIRE(cache.findCacheLine(addr2) != nullptr);
        REQUIRE(cache.findCacheLine(addr3) != nullptr);
        REQUIRE(cache.findCacheLine(addr4) != nullptr);
    }
}

TEST_CASE("Cache - Replacement Policies", "[cache]") {
    Memory memory(memorySize);
    uint32_t addresses[] = {0x1000, 0x2000, 0x3000, 0x4000};

    SECTION("FIFO Replacement") {
        Cache cache(8 * 1024, 2, "FIFO", "WB", L1, nullptr, memory);

        // fill cache
        for (auto addr : addresses) {
            cache.write(addr, 42);
        }

        // evict first-written address
        cache.write(0x5000, 99);
        REQUIRE(cache.findCacheLine(0x1000) == nullptr);
    }

    SECTION("LRU Replacement") {
        Cache cache(8 * 1024, 2, "LRU", "WB", L1, nullptr, memory);

        // fill cache
        for (auto addr : addresses) {
            cache.write(addr, 42);
        }

        // use 0x1000 again to make it most recently used
        cache.read(0x1000);
        cache.read(0x4000);
        cache.read(0x3000);
        cache.read(0x1000);

        // should evict
        cache.write(0x5000, 99);

        // 0x2000 is least recently used and was evicted
        REQUIRE(cache.findCacheLine(0x2000) == nullptr);
    }

    SECTION("LFU Replacement") {
        Cache cache(8 * 1024, 2, "LFU", "WB", L1, nullptr, memory);

        // fill cache
        for (auto addr : addresses) {
            cache.write(addr, 42);
        }

        // read addresses multiple times except 0x2000
        cache.read(0x1000);
        cache.read(0x1000);
        cache.read(0x1000);
        cache.read(0x3000);
        cache.read(0x3000);
        cache.read(0x4000);
        cache.read(0x4000);

        cache.write(0x5000, 99);

        REQUIRE(cache.findCacheLine(0x2000) == nullptr);
    }
}

TEST_CASE("Profiling Test", "[cache]") {
    Memory memory(memorySize);
    Cache cache(8 * 1024, 4, "LRU", "WB", L1, nullptr, memory);

    uint32_t test_address = 0x1000;
    int value = 42;

    for (int i = 0; i < 1000000; i++) {
        cache.write(test_address + (i % 1024) * 4, value);
        cache.read(test_address + (i % 1024) * 4);
    }
}