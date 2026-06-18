#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include <cctype>
#include <algorithm>

class DatabaseParser {
public:
    DatabaseParser() = default;

    std::vector<std::string> Parse(const std::string& cmd) {
        parsed_.clear();
        Split(cmd);
        if (parsed_.empty()) {
            throw std::runtime_error("ERR empty command");
        }
        std::transform(parsed_[0].begin(), parsed_[0].end(), parsed_[0].begin(), ::toupper);
        ProcessNWordsCommands();
        return parsed_;
    }

private:
    void Split(const std::string& cmd) {
        parsed_.reserve(8);
        size_t pos = 0;
        const size_t n = cmd.size();
        while (pos < n) {
            while (pos < n && cmd[pos] == ' ') pos++;
            if (pos >= n) break;
            size_t start = pos;
            while (pos < n && cmd[pos] != ' ') pos++;
            parsed_.emplace_back(cmd.substr(start, pos - start));
        }
    }

    void ProcessNWordsCommands() {
        if (parsed_.size() < 2) {
            return;
        }
        std::string& a = parsed_[0];
        std::string& b = parsed_[1];
        std::string b_upper = b;
        std::transform(b_upper.begin(), b_upper.end(), b_upper.begin(), ::toupper);
        if ((a == "MEMORY" && b_upper == "USAGE") ||
            (a == "CONFIG" && b_upper == "GET") ||
            (a == "CONFIG" && b_upper == "SET")) {
            parsed_[1] = a + " " + b_upper;
            parsed_.erase(parsed_.begin());
        }
    }

    std::vector<std::string> parsed_;
};
