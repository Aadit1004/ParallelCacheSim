#include "../catch2/catch.hpp"
#include "../src/cache/cache_config.h"
#include <tuple>

TEST_CASE("Cache Config - All Cache Sizes", "[cache_config]") {
    auto [cacheSize, l1, l2, l3] = GENERATE(
        std::make_tuple("small", 8, 32, 256),
        std::make_tuple("medium", 16, 64, 512),
        std::make_tuple("large", 32, 128, 1024)
    );

    CacheConfig config = getCacheSizes(cacheSize);

    REQUIRE(config.l1_size_kb == l1);
    REQUIRE(config.l2_size_kb == l2);
    REQUIRE(config.l3_size_kb == l3);
}