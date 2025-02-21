#include "../catch2/catch.hpp"
#include "../src/memory/memory.h"

TEST_CASE("Memory Initialization", "[memory]") {
    Memory memory(1024);
    REQUIRE(true);
}

TEST_CASE("Read from Uninitialized Memory", "[memory]") {
    Memory memory(1024);

    uint32_t address = 0x1000;
    REQUIRE(memory.read(address) == 0);
}

TEST_CASE("Write and Read from Memory", "[memory]") {
    Memory memory(1024);

    uint32_t address = 0x2000;
    int value = 42;

    memory.write(address, value);
    REQUIRE(memory.read(address) == value);
}

TEST_CASE("Overwrite Memory Value", "[memory]") {
    Memory memory(1024);

    uint32_t address = 0x3000;
    memory.write(address, 10);
    REQUIRE(memory.read(address) == 10);

    memory.write(address, 99);
    REQUIRE(memory.read(address) == 99);
}

TEST_CASE("Read from Multiple Memory Locations", "[memory]") {
    Memory memory(1024);

    uint32_t address1 = 0x4000, address2 = 0x5000;
    memory.write(address1, 123);
    memory.write(address2, 456);

    REQUIRE(memory.read(address1) == 123);
    REQUIRE(memory.read(address2) == 456);
}