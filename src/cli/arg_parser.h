#pragma once
#include <string>
#include <vector>
#include <algorithm>

class ArgParser {
public:
    ArgParser(int t_argc, char *t_argv[]);

    bool validateArguments();

private:
    int m_argc;
    std::vector<std::string> m_argument;

    bool validateCaches();
    bool validateThreads();
    bool validatePolicy();
    bool validateTraceAndVerbose();

    bool isNumber(const std::string& t_str);
};