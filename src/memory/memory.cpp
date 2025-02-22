#include "memory.h"

Memory::Memory(int memory_size) : m_memory_size(memory_size), endAddress(baseAddress + memory_size - 4) {
    // std::cout << "Initialized simulated memory: " << memory_size_bytes << " KB\n";
    // can print base to end address range
}

int Memory::read(uint32_t address) {
    if (!isValidAddress(address)) {
        return 0;
    }
    return m_memory.count(address) ? m_memory.at(address) : 0;
}

void Memory::write(uint32_t address, int value) {
    if (!isValidAddress(address)) {
        return;
    }
    // std::cout << "[MEMORY] Writing value " << value << " to address 0x" << std::hex << address << std::dec << "\n";
    m_memory[address] = value;
}

void Memory::printMemoryState() {
    // std::cout << "[MEMORY] State:\n";
    // for (const auto& pair : m_memory) {
    //     std::cout << "  Address: 0x" << std::hex << pair.first << " -> Value: " << std::dec << pair.second << "\n";
    // }
}

bool Memory::isValidAddress(uint32_t address) const {
    if (address % 4 != 0) {
        // std::cerr << "[ERROR] Unaligned memory access at 0x" << std::hex << address << std::dec << "\n";
        return false;
    }
    if (address < baseAddress || address > endAddress) {
        // std::cerr << "[ERROR] Out-of-bounds memory access at 0x" << std::hex << address << std::dec << "\n";
        return false;
    }
    return true;
}