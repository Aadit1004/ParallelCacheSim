#include "../catch2/catch.hpp"
#include "../src/cache/cache_config.h"
#include <tuple>

TEST_CASE("Cache Config - All Cache Sizes", "[cache_config]") {
    auto [cacheSize, l1, l2, l3] = GENERATE(
        std::make_tuple("small", 16 * 1024, 128 * 1024, 512 * 1024),
        std::make_tuple("medium", 64 * 1024, 512 * 1024, 4 * 1024 * 1024),
        std::make_tuple("large", 512 * 1024, 2 * 1024 * 1024, 8 * 1024 * 1024)
    );

    CacheConfig config = getCacheSizes(cacheSize);

    REQUIRE(config.l1_size == l1);
    REQUIRE(config.l2_size == l2);
    REQUIRE(config.l3_size == l3);
}