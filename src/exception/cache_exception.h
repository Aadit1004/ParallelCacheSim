#pragma once
#include <exception>
#include <string>

class CacheException : public std::exception {
private:
    std::string m_message;

public:
    explicit CacheException(const std::string& message) 
        : m_message(message) {}

    const char* what() const noexcept override {
        return m_message.c_str();
    }
};
