#include "arg_parser.h"

ArgParser::ArgParser(int t_argc, char *t_argv[]) : m_argc(t_argc - 1) {
    for (int i = 1; i < t_argc; i++) {
        m_argument.emplace_back(t_argv[i]);
    }
}

ArgParser::~ArgParser() {
    m_argument.clear();
}

// EXAMPLE: ./cache_sim -cache_size medium -threads 4 -policy LRU -assoc 1 -write_policy WB -trace memory_access.txt --verbose
bool ArgParser::validateArguments() {
    if (m_argc != 12 && m_argc != 13) {
        return false;
    }
    return validateCaches() && validateThreads() && validatePolicy() && 
    validateAssociativity() && validateWritePolicy() && validateTraceAndVerbose();
}

bool ArgParser::isNumber(const std::string& t_str) {
    return !t_str.empty() && std::all_of(t_str.begin(), t_str.end(), ::isdigit);
}

bool ArgParser::validateCaches() {
    bool validCacheSize =  m_argument[1] == "small" || m_argument[1] == "medium" || m_argument[1] == "large";
    return m_argument[0] == "-cache_size" && validCacheSize;
}

bool ArgParser::validateThreads() {
    if (!isNumber(m_argument[3]) || m_argument[2] != "-threads") { 
        return false;
    }
    int threadValue = std::stoi(m_argument[3]);
    if (threadValue < 1 || threadValue > 16) { 
        return false;
    }
    if (threadValue > 1 && (threadValue % 2 != 0)) { 
        return false;
    }
    return true;
}

bool ArgParser::validatePolicy() {
    return m_argument[4] == "-policy" && 
    (m_argument[5] == "LRU" || m_argument[5] == "FIFO" || m_argument[5] == "LFU");
}

bool ArgParser::validateAssociativity() {
    if (!isNumber(m_argument[7]) || m_argument[6] != "-assoc") { return false;
}
    int assocNum = std::stoi(m_argument[7]);
    return assocNum == 1 || assocNum == 4 || assocNum == 0;
}

bool ArgParser::validateWritePolicy() {
    return m_argument[8] == "-write_policy" && (m_argument[9] == "WB" || m_argument[9] == "WT");
}

bool ArgParser::validateTraceAndVerbose() {
    bool isTraceValid = m_argument[10] == "-trace" && !m_argument[11].empty();
    bool isVerboseValid = (m_argc == 13) ? (m_argument[12] == "--verbose") : true;
    return isTraceValid && isVerboseValid;
}

ValidParams ArgParser::getValidParams() {
    ValidParams params;

    CacheConfig cache_config = getCacheSizes(m_argument[1]);
    params.l1_cache_size = cache_config.l1_size;
    params.l2_cache_size = cache_config.l2_size;
    params.l3_cache_size = cache_config.l3_size;
    params.memory_size = m_argument[1];
    params.num_threads = std::stoi(m_argument[3]);
    params.replacement_policy = m_argument[5];
    params.associativity = std::stoi(m_argument[7]);
    params.write_policy = m_argument[9];
    params.access_file_name = m_argument[11];
    params.isVerbose = (m_argc == 13);

    return params;
}