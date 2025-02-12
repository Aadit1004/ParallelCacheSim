#pragma once
#include <string>

struct CacheConfig {
    int l1_size_kb;
    int l2_size_kb;
    int l3_size_kb;
};

CacheConfig getCacheSizes(const std::string& t_size);