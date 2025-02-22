#pragma once
#include <string>

struct CacheConfig {
    // size in bytes
    int l1_size;
    int l2_size;
    int l3_size;
};

CacheConfig getCacheSizes(const std::string& t_size);