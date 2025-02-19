#include <iostream>
#include <unordered_map>
#include "cli/arg_parser.h"
#include "memory/memory.h"

int getMemorySize(std::string& t_size) {
    if (t_size == "small") { 
        return 4 * 1024;  // 4MB
    } else if (t_size == "medium") { 
        return 16 * 1024;  // 16MB
    } else {
        return 64 * 1024;  // 64MB
    }
}

int main(int argc, char *argv[]) {
    ArgParser argParser(argc, argv);
    if (argParser.validateArguments()) {
        ValidParams params = argParser.getValidParams();

        int memory_size_kb = getMemorySize(params.memory_size);
        Memory memory(memory_size_kb);

        std::cout << "Valid argmunets passed" << std::endl;
    } else {
        std::cout << "Invalid argmunets passed" << std::endl;
    }
    return 0;
}