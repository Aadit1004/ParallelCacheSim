#include "cache_config.h"


CacheConfig getCacheSizes(const std::string& size) {
    if (size == "small") {
        return {16, 128, 4096};  // L1 = 16KB, L2 = 128KB, L3 = 4MB
    } else if (size == "medium") {
        return {32, 256, 8192};  // L1 = 32KB, L2 = 256KB, L3 = 8MB
    } else {
        return {64, 512, 16384}; // L1 = 64KB, L2 = 512KB, L3 = 16MB
    }
}