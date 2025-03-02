#include "../src/io/file_manager.h"
#include "../catch2/catch.hpp"
#include "../src/exception/cache_exception.h"

/*
- unit tests for getNextRequest
*/

TEST_CASE("File Manager - Valid File", "[io]") {
    const std::string filename = "valid_file.txt";
    FileManager fm(filename, false, true);
    const int expectedOperations = 20;

    REQUIRE(fm.isValidFile());
    REQUIRE_NOTHROW(fm.parseFile());
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - File Doesn't Exist", "[io]") {
    const std::string filename = "missing_file.txt";
    FileManager fm(filename, false, true);

    REQUIRE_FALSE(fm.isValidFile());
}

TEST_CASE("File Manager - Profiling Multiple Operations", "[io][profiling]") {
    const std::string filename = "valid_file_profiling.txt";
    FileManager fm(filename, false, true);
    const int expectedOperations = 5000;

    REQUIRE(fm.isValidFile());
    REQUIRE_NOTHROW(fm.parseFile());
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - Valid File with Extra args in line", "[io]") {
    const std::string filename = "valid_file_extra.txt";
    FileManager fm(filename, false, true);
    const int expectedOperations = 20;

    REQUIRE(fm.isValidFile());
    REQUIRE_NOTHROW(fm.parseFile());
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - Valid Empty File", "[io]") {
    const std::string filename = "empty_file.txt";
    FileManager fm(filename, false, true);
    const int expectedOperations = 0;

    REQUIRE(fm.isValidFile());
    REQUIRE_NOTHROW(fm.parseFile());
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager -  Incorrect File Extension", "[io]") {
    const std::string filename = "invalid_file_extension.md";
    FileManager fm(filename, false, true);

    REQUIRE_FALSE(fm.isValidFile());
}

TEST_CASE("File Manager - Invalid Operation", "[io]") {
    const std::string filename = "invalid_op.txt";
    FileManager fm(filename, false, true);
    const int expectedOperations = 0;

    REQUIRE(fm.isValidFile());
    REQUIRE_THROWS_AS(fm.parseFile(), CacheException);
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - Invalid/Missing Read Addresses", "[io]") {
    const std::string filename = GENERATE("invalid_read_address.txt", "missing_read_address.txt");
    FileManager fm(filename, false, true);
    const int expectedOperations = 0;

    REQUIRE(fm.isValidFile());
    REQUIRE_THROWS_AS(fm.parseFile(), CacheException);
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - Invalid/Missing Write Addresses", "[io]") {
    const std::string filename = GENERATE("invalid_write_address.txt", "missing_write_address.txt");
    FileManager fm(filename, false, true);
    const int expectedOperations = 0;

    REQUIRE(fm.isValidFile());
    REQUIRE_THROWS_AS(fm.parseFile(), CacheException);
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - Invalid/Missing Write Value", "[io]") {
    const std::string filename = GENERATE("invalid_write_value.txt", "missing_write_value.txt");
    FileManager fm(filename, false, true);
    const int expectedOperations = 0;

    REQUIRE(fm.isValidFile());
    REQUIRE_THROWS_AS(fm.parseFile(), CacheException);
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - Valid File with Empty Breaks", "[io]") {
    const std::string filename = "valid_file_empty_breaks.txt";
    FileManager fm(filename, false, true);
    const int expectedOperations = 20;

    REQUIRE(fm.isValidFile());
    REQUIRE_NOTHROW(fm.parseFile());
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - Valid File with Extra Spaces", "[io]") {
    const std::string filename = "valid_file_extra_spaces.txt";
    FileManager fm(filename, false, true);
    const int expectedOperations = 20;

    REQUIRE(fm.isValidFile());
    REQUIRE_NOTHROW(fm.parseFile());
    REQUIRE(fm.getNumOperations() == expectedOperations);
}

TEST_CASE("File Manager - getNextRequest Test ", "[io]") {
    const std::string filename = "valid_file_two.txt";
    FileManager fm(filename, false, true);
    REQUIRE(fm.isValidFile());
    REQUIRE_NOTHROW(fm.parseFile());
    REQUIRE(fm.getNumOperations() == 2);

    std::optional<MemoryRequest> requestOpOne = fm.getNextRequest();
    REQUIRE(requestOpOne.has_value());
    MemoryRequest writeOp = requestOpOne.value();
    REQUIRE(writeOp.type == AccessType::WRITE);
    REQUIRE(writeOp.address == 0x1000);
    REQUIRE(writeOp.value == 42);
    REQUIRE(fm.getNumOperations() == 1);

    std::optional<MemoryRequest> requestOpTwo = fm.getNextRequest();
    REQUIRE(requestOpTwo.has_value());
    MemoryRequest readOp = requestOpTwo.value();
    REQUIRE(readOp.type == AccessType::READ);
    REQUIRE(readOp.address == 0x1000);
    REQUIRE(fm.getNumOperations() == 0);

    std::optional<MemoryRequest> requestOpThree = fm.getNextRequest();
    REQUIRE_FALSE(requestOpThree.has_value()); // it is returning std::nullopt
}