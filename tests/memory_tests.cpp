#include "../catch2/catch.hpp"
#include "../src/memory/memory.h"

const int memory_size = 4 * 1024 * 1024;

TEST_CASE("Memory Initialization", "[memory]") {
    Memory memory(memory_size);
    REQUIRE(true);
}

TEST_CASE("Read from Uninitialized Memory", "[memory]") {
    Memory memory(memory_size);

    uint32_t address = 0x1000;
    REQUIRE(memory.read(address) == 0);
}

TEST_CASE("Write and Read from Memory", "[memory]") {
    Memory memory(memory_size);

    uint32_t address = 0x2000;
    int value = 42;

    memory.write(address, value);
    REQUIRE(memory.read(address) == value);
}

TEST_CASE("Overwrite Memory Value", "[memory]") {
    Memory memory(memory_size);

    uint32_t address = 0x3000;
    memory.write(address, 10);
    REQUIRE(memory.read(address) == 10);

    memory.write(address, 99);
    REQUIRE(memory.read(address) == 99);
}

TEST_CASE("Read from Multiple Memory Locations", "[memory]") {
    Memory memory(memory_size);

    uint32_t address1 = 0x4000, address2 = 0x5000;
    memory.write(address1, 123);
    memory.write(address2, 456);

    REQUIRE(memory.read(address1) == 123);
    REQUIRE(memory.read(address2) == 456);
}

TEST_CASE("Unaligned Memory Access Should Fail", "[memory]") {
    Memory memory(memory_size);

    uint32_t unaligned_address = 0x1003; // not multiple of 4
    memory.write(unaligned_address, 77);
    REQUIRE(memory.read(unaligned_address) == 0);
}

TEST_CASE("Memory Access Below Base Address Should Fail", "[memory]") {
    Memory memory(memory_size);

    uint32_t invalid_address = 0x0FFF; // below base address of 0x1000
    memory.write(invalid_address, 55);
    REQUIRE(memory.read(invalid_address) == 0);
}

TEST_CASE("Memory Access Above End Address Should Fail", "[memory]") {
    Memory memory(memory_size);

    uint32_t invalid_address = 0x1000 + memory_size; // address after allocated memory
    memory.write(invalid_address, 88);
    REQUIRE(memory.read(invalid_address) == 0);
}