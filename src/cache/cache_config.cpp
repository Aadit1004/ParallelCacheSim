#include "cache_config.h"

CacheConfig getCacheSizes(const std::string& size) {
    if (size == "small") {
        return {8, 32, 256};  // L1 = 8KB, L2 = 32KB, L3 = 256KB
    } else if (size == "medium") {
        return {16, 64, 512}; // L1 = 16KB, L2 = 64KB, L3 = 512KB
    } else {
        return {32, 128, 1024}; // L1 = 32KB, L2 = 128KB, L3 = 1MB
    }
}