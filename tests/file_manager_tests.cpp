#include "../src/io/file_manager.h"
#include "../catch2/catch.hpp"
#include "../src/exception/cache_exception.h"

/*
test cases to do under [io] and [profiling] category:

- valid W but invalid address
- valid W but missing address
- valid W and address but invalid int (it is string / lower than int min / higher than int max)
- valid W and address but missing int
- file not existing
- empty lines between operations (should be valid?)
- whitespace variations 
    Test cases where there are extra spaces/tabs between tokens:
    "R 0x1A3F"
    " W 0x2B7D 42 "
- any extra test cases

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