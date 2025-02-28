#include "../catch2/catch.hpp"
#include "../src/memory/memory.h"
#include "../src/exception/cache_exception.h"

const int memory_size = 4 * 1024 * 1024;

TEST_CASE("Memory Initialization", "[memory]") {
    Memory memory(memory_size, false);
    REQUIRE(true);
}

TEST_CASE("Read from Uninitialized Memory", "[memory]") {
    Memory memory(memory_size, false);

    uint32_t address = 0x1000;
    REQUIRE(memory.read(address) == 0);
}

TEST_CASE("Write and Read from Memory", "[memory]") {
    Memory memory(memory_size, false);

    uint32_t address = 0x2000;
    int value = 42;

    memory.write(address, value);
    REQUIRE(memory.read(address) == value);
}

TEST_CASE("Overwrite Memory Value", "[memory]") {
    Memory memory(memory_size, false);

    uint32_t address = 0x3000;
    memory.write(address, 10);
    REQUIRE(memory.read(address) == 10);

    memory.write(address, 99);
    REQUIRE(memory.read(address) == 99);
}

TEST_CASE("Read from Multiple Memory Locations", "[memory]") {
    Memory memory(memory_size, false);

    uint32_t address1 = 0x4000, address2 = 0x5000;
    memory.write(address1, 123);
    memory.write(address2, 456);

    REQUIRE(memory.read(address1) == 123);
    REQUIRE(memory.read(address2) == 456);
}

TEST_CASE("Unaligned Memory Access Should Fail", "[memory]") {
    Memory memory(memory_size, false);

    uint32_t unaligned_address = 0x1003; // not multiple of 4
    REQUIRE_THROWS_AS(memory.write(unaligned_address, 77), CacheException);
    REQUIRE_THROWS_AS(memory.read(unaligned_address), CacheException);
}

TEST_CASE("Memory Access Below Base Address Should Fail", "[memory]") {
    Memory memory(memory_size, false);

    uint32_t invalid_address = 0x0FFF; // below base address of 0x1000
    REQUIRE_THROWS_AS(memory.write(invalid_address, 55), CacheException);
    REQUIRE_THROWS_AS(memory.read(invalid_address), CacheException);
}

TEST_CASE("Memory Access Above End Address Should Fail", "[memory]") {
    Memory memory(memory_size, false);

    uint32_t invalid_address = 0x1000 + memory_size; // address after allocated memory
    REQUIRE_THROWS_AS(memory.write(invalid_address, 88), CacheException);
    REQUIRE_THROWS_AS(memory.read(invalid_address), CacheException);
}

TEST_CASE("Profiling - Memory Initialization", "[memory][profiling]") {
    for (int i = 0; i < 1000; ++i) {
        Memory memory(memory_size, false);
        REQUIRE(true);
    }
}

TEST_CASE("Profiling - Sequential Memory Writes", "[memory][profiling]") {
    Memory memory(memory_size, false);
    uint32_t base_address = 0x1000;

    for (uint32_t i = 0; i < 100000; i += 4) {
        memory.write(base_address + i, i);
    }

    REQUIRE(true);
}

TEST_CASE("Profiling - Sequential Memory Reads", "[memory][profiling]") {
    Memory memory(memory_size, false);
    uint32_t base_address = 0x1000;

    for (uint32_t i = 0; i < 100000; i += 4) {
        memory.write(base_address + i, i);
    }

    for (uint32_t i = 0; i < 100000; i += 4) {
        memory.read(base_address + i);
    }

    REQUIRE(true);
}

TEST_CASE("Profiling - Random Memory Access", "[memory][profiling]") {
    Memory memory(memory_size, false);
    uint32_t base_address = 0x1000;
    uint32_t max_valid_address = base_address + memory_size - 4;

    for (uint32_t i = 0; i < 100000; i += 4) {
        uint32_t rand_address = base_address + (rand() % (memory_size / 4)) * 4;
        REQUIRE(rand_address >= base_address);
        REQUIRE(rand_address <= max_valid_address);
        memory.write(rand_address, i);
    }

    for (uint32_t i = 0; i < 100000; i += 4) {
        uint32_t rand_address = base_address + (rand() % (memory_size / 4)) * 4;
        REQUIRE(rand_address >= base_address);
        REQUIRE(rand_address <= max_valid_address);
        memory.read(rand_address);
    }

    REQUIRE(true);
}