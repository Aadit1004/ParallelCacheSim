#include "memory.h"

Memory::Memory(int memory_size_kb) : m_memory_size_kb(memory_size_kb) {
    // std::cout << "Initialized simulated memory: " << m_memory_size_kb << " KB\n";
}

int Memory::read(uint32_t address) {
    if (m_memory.find(address) == m_memory.end()) {
        // std::cout << "[MEMORY] Read miss at address 0x" << std::hex << address << std::dec << "\n";
        return 0; 
    }
    return m_memory[address];
}

void Memory::write(uint32_t address, int value) {
    // std::cout << "[MEMORY] Writing value " << value << " to address 0x" << std::hex << address << std::dec << "\n";
    m_memory[address] = value;
}

void Memory::printMemoryState() {
    // std::cout << "[MEMORY] State:\n";
    // for (const auto& pair : m_memory) {
    //     std::cout << "  Address: 0x" << std::hex << pair.first << " -> Value: " << std::dec << pair.second << "\n";
    // }
}
