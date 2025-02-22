#pragma once
#include <unordered_map>
#include <cstdint>
#include <iostream>

class Memory {
public:
    Memory(int memory_size);
    
    int read(uint32_t address);
    void write(uint32_t address, int value);
    void printMemoryState();  // for debugging

private:
    bool isValidAddress(uint32_t address) const;

    const uint32_t baseAddress = 0x1000;
    std::unordered_map<uint32_t, int> m_memory; 
    int m_memory_size; // value in bytes
    const uint32_t endAddress;
};
