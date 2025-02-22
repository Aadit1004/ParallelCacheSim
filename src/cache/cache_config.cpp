#include "cache_config.h"

CacheConfig getCacheSizes(const std::string& size) {
    if (size == "small") {
        return {8 * 1024, 32 * 1024, 256 * 1024};  // L1 = 8KB, L2 = 32KB, L3 = 256KB
    } else if (size == "medium") {
        return {16 * 1024, 64 * 1024, 512 * 1024}; // L1 = 16KB, L2 = 64KB, L3 = 512KB
    } else {
        return {32 * 1024, 128 * 1024, 1024 * 1024}; // L1 = 32KB, L2 = 128KB, L3 = 1MB
    }
}