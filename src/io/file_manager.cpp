#include "file_manager.h"

FileManager::FileManager(const std::string& filename, bool isVerbose, bool isTest) : m_filename(filename), m_isVerbose(isVerbose), m_isTest(isTest) {}

bool FileManager::fileExists(const std::string& path) const {
    return std::filesystem::exists(path);
}

int FileManager::getNumOperations() const {
    return m_requests.size();
}

bool FileManager::isValidFile() const {
    std::filesystem::path filePath = (m_isTest ? "examples/tests/" : "examples/") + m_filename;
    return std::filesystem::exists(filePath) && filePath.extension() == ".txt";
}

std::optional<MemoryRequest> FileManager::getNextRequest() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_requests.empty()) {
        return std::nullopt;
    }
    MemoryRequest request = m_requests.front();
    m_requests.pop();
    return request;
}

void FileManager::parseFile() {
    std::ifstream file((m_isTest ? "examples/tests/" : "examples/") + m_filename);
    if (!file.is_open()) {
        throw CacheException("Failed to open file: " + m_filename);
    }
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (m_isVerbose) {
            std::cout << "[FILE] Read line: " << line << std::endl;
        }
        std::vector<std::string> tokens = splitBySpace(line);

        if (tokens.empty() || (tokens[0] != "R" && tokens[0] != "W")) {
            clearRequests();
            throw CacheException("[ERROR] Invalid operation in file: " + line);
        }

        // if invalid, throw exception (no need to check valid addresses) but check syntax (R address || W address value)
        // if valid, create MemoryRequest and add to queue
        if (tokens[0] == "R") {
            if (tokens.size() < 2 || !isValidHexAddress(tokens[1])) {
                clearRequests();
                throw CacheException("[ERROR] Invalid read format: " + line);
            }
            uint32_t address = std::stoul(tokens[1], nullptr, 16);
            MemoryRequest request(READ, address);
            m_requests.push(request);
        } else if (tokens[0] == "W") {
            if (tokens.size() < 3 || !isValidHexAddress(tokens[1]) || !isValidInt(tokens[2])) {
                clearRequests();
                throw CacheException("[ERROR] Invalid write format: " + line);
            }
            uint32_t address = std::stoul(tokens[1], nullptr, 16);
            int value = std::stoi(tokens[2]);
            MemoryRequest request(WRITE, address, value);
            m_requests.push(request);
        }
    }
    file.close();
}

std::vector<std::string> FileManager::splitBySpace(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    
    while (stream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

bool FileManager::isValidHexAddress(const std::string& address) {
    return address.size() > 2 && address[0] == '0' && address[1] == 'x' && 
           address.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
}

bool FileManager::isValidInt(const std::string& value) {
    if (value.empty()) return false;
    size_t start = (value[0] == '-') ? 1 : 0;
    if (value.find_first_not_of("0123456789", start) != std::string::npos) {
        return false;
    }

    try {
        int num = std::stoi(value);
        (void)num; // to avoid unused variable warning
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void FileManager::clearRequests() {
    std::queue<MemoryRequest> empty;
    std::swap(m_requests, empty);
}

std::string FileManager::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}