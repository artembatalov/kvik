#pragma once
#include <string>
#include <cstddef>

class String {
public:
    String(std::string string) : str_(std::move(string)) {}

    const std::string& GetString() const {
        return str_;
    }

    size_t Size() const {
        return str_.size();
    }

    void Append(const std::string& other) {
        str_ += other;
    }

    size_t MemoryUsage() const {
        return str_.size() + sizeof(std::string);
    }

private:
    std::string str_;
};
