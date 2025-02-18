#pragma once
#include <unordered_map>
#include <cstdint>
#include <iostream>

class Memory {
public:
    Memory(int memory_size_kb);
    
    int read(uint32_t address);
    void write(uint32_t address, int value);
    void printMemoryState();  // for debugging

private:
    std::unordered_map<uint32_t, int> m_memory; 
    int m_memory_size_kb;
};
