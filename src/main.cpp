#include <iostream>
#include "cli/arg_parser.h"
#include <unordered_map>

std::unordered_map<uint32_t, int> simulated_memory;

int getMemorySize(std::string& t_size) {
    if (t_size == "small") return 4 * 1024 * 1024;  // 4MB
    else if (t_size == "medium") return 16 * 1024 * 1024;  // 16MB
    else return 64 * 1024 * 1024;  // 64MB
}

int main(int argc, char *argv[]) {
    ArgParser argParser(argc, argv);
    if (argParser.validateArguments()) {
        ValidParams params = argParser.getValidParams();

        int memory_size = getMemorySize(params.memory_size);

        std::cout << "Valid argmunets passed" << std::endl;
    } else {
        std::cout << "Invalid argmunets passed" << std::endl;
    }
    return 0;
}