#include <iostream>
#include "cli/arg_parser.h"

int main(int argc, char *argv[]) {
    ArgParser argParser(argc, argv);
    if (argParser.validateArguments()) {
        ValidParams params = argParser.getValidParams();
        std::cout << "Valid argmunets passed" << std::endl;
    } else {
        std::cout << "Invalid argmunets passed" << std::endl;
    }
    return 0;
}