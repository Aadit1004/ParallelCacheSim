#include "arg_parser.h"

ArgParser::ArgParser(int t_argc, char *t_argv[]) {
    this->m_argc = t_argc - 1;
    for (int i = 1; i < t_argc; i++) {
        m_argument.push_back(t_argv[i]);
    }
}

bool ArgParser::validateArguments() {
    if (m_argc != 12 && m_argc != 13) {
        return false;
    }
    return validateCaches() && validateThreads() && validatePolicy() && validateTraceAndVerbose();
    
}

bool ArgParser::isNumber(const std::string& t_str) {
    return !t_str.empty() && std::all_of(t_str.begin(), t_str.end(), ::isdigit);
}

bool ArgParser::validateCaches() {
    auto validateCacheSizes = [this](int cacheIndex) {
        return m_argument[cacheIndex] == "small" || 
               m_argument[cacheIndex] == "medium" || 
               m_argument[cacheIndex] == "large";
    };
    bool isL1Valid = validateCacheSizes(1);
    bool isL2Valid = validateCacheSizes(3);
    bool isL3Valid = validateCacheSizes(5);
    return m_argument[0] == "-l1" && isL1Valid &&
       m_argument[2] == "-l2" && isL2Valid &&
       m_argument[4] == "-l3" && isL3Valid;
}

bool ArgParser::validateThreads() {
    if (!isNumber(m_argument[7]) || m_argument[6] != "-threads") return false;
    int coreValue = std::stoi(m_argument[7]);
    if (coreValue < 1 || coreValue > 16) return false;
    if (coreValue > 1 && (coreValue % 2 != 0)) return false;
    return true;
}

bool ArgParser::validatePolicy() {
    return m_argument[8] == "-policy" && 
    (m_argument[9] == "LRU" || m_argument[9] == "FIFO" || m_argument[9] == "LFU");
}

bool ArgParser::validateTraceAndVerbose() {
    bool isTraceValid = m_argument[10] == "-trace" && !m_argument[11].empty();
    bool isVerboseValid = (m_argc == 13) ? (m_argument[12] == "-verbose") : true;
    return isTraceValid && isVerboseValid;
}