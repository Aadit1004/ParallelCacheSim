#pragma once
#include <string>
#include <queue>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <filesystem>
#include <optional>
#include <algorithm>
#include "../exception/cache_exception.h"

enum AccessType {
    READ,
    WRITE
};

struct MemoryRequest {
    AccessType type;
    uint32_t address;
    int value;

    MemoryRequest(AccessType t_type, uint32_t t_address, int t_value = 0)
        : type(t_type), address(t_address), value(t_value) {}
};

class FileManager {
    public:
        FileManager(const std::string& filename, bool isVerbose = false, bool isTest = false);
    
        bool isValidFile() const;
        void parseFile();
        std::optional<MemoryRequest> getNextRequest();
        int getNumOperations() const;
    
    private:
        std::string m_filename;
        std::queue<MemoryRequest> m_requests;
        mutable std::mutex m_mutex;
        bool m_isVerbose;
        bool m_isTest; // for testing
    
        bool fileExists(const std::string& path) const;
        std::vector<std::string> splitBySpace(const std::string& input);
        bool isValidHexAddress(const std::string& address);
        bool isValidInt(const std::string& value);
        void clearRequests();
        std::string trim(const std::string& str);
};