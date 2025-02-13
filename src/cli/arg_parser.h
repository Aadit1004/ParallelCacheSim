#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "../cache/cache_config.h"

struct ValidParams {
    int l1_cache_size_kb;
    int l2_cache_size_kb;
    int l3_cache_size_kb;
    std::string memory_size;
    int num_threads;
    std::string replacement_policy;
    std::string access_file_name;
    int associativity;
    std::string write_policy;
    bool isVerbose;
};

class ArgParser {
public:
    ArgParser(int t_argc, char *t_argv[]);

    bool validateArguments();
    ValidParams getValidParams();

private:
    int m_argc;
    std::vector<std::string> m_argument;

    bool validateCaches();
    bool validateThreads();
    bool validatePolicy();
    bool validateAssociativity();
    bool validateWritePolicy();
    bool validateTraceAndVerbose();

    bool isNumber(const std::string& t_str);
};