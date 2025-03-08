#include "cache_config.h"

CacheConfig getCacheSizes(const std::string& size) {
    if (size == "small") {
        return {16 * 1024, 128 * 1024, 512 * 1024};  // L1 = 16KB, L2 = 128KB, L3 = 512KB
    } else if (size == "medium") {
        return {64 * 1024, 512 * 1024, 4 * 1024 * 1024}; // L1 = 64KB, L2 = 512KB, L3 = 4MB
    } else {
        return {512 * 1024, 2 * 1024 * 1024, 8 * 1024 * 1024}; // L1 = 512KB, L2 = 2MB, L3 = 8MB
    }
}