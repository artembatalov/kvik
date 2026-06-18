#pragma once
#include <vector>
#include <string>
#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <deque>

class List {
public:
    List() = default;

    List(const std::vector<std::string>& list)
        : list_(list.begin(), list.end()) {}

    std::string GetString() const {
        std::string res;
        res.reserve(list_.size() * 8 + 2);
        res += "[";
        for (int i = 0; i < (int)list_.size(); i++) {
            res += list_[i];
            if (i != (int)list_.size() - 1) res += " ";
        }
        res += "]";
        return res;
    }

    void PushLeft(const std::vector<std::string>& other) {
        list_.insert(list_.begin(), other.rbegin(), other.rend());
    }

    void PushRight(const std::vector<std::string>& other) {
        list_.insert(list_.end(), other.begin(), other.end());
    }

    std::vector<std::string> PopLeft(int count) {
        count = std::min(count, (int)list_.size());
        std::vector<std::string> result(list_.begin(), list_.begin() + count);
        list_.erase(list_.begin(), list_.begin() + count);
        return result;
    }

    std::vector<std::string> PopRight(int count) {
        count = std::min(count, (int)list_.size());
        std::vector<std::string> result(list_.end() - count, list_.end());
        list_.erase(list_.end() - count, list_.end());
        std::reverse(result.begin(), result.end());
        return result;
    }

    std::vector<std::string> Range(int start, int stop) const {
        int n = (int)list_.size();
        if (start < 0) start = n + start;
        if (stop < 0) stop = n + stop;
        start = std::max(0, start);
        stop = std::min(n - 1, stop);
        if (start > stop) return {};
        return std::vector<std::string>(list_.begin() + start, list_.begin() + stop + 1);
    }

    const std::string& Index(int idx) const {
        int n = (int)list_.size();
        if (idx < 0) idx = n + idx;
        if (idx < 0 || idx >= n) throw std::runtime_error("ERR index out of range");
        return list_[idx];
    }

    void Set(int idx, const std::string& value) {
        int n = (int)list_.size();
        if (idx < 0) idx = n + idx;
        if (idx < 0 || idx >= n) throw std::runtime_error("ERR index out of range");
        list_[idx] = value;
    }

    bool Insert(const std::string& pivot, const std::string& value, bool before) {
        for (auto it = list_.begin(); it != list_.end(); ++it) {
            if (*it == pivot) {
                if (!before) ++it;
                list_.insert(it, value);
                return true;
            }
        }
        return false;
    }

    size_t Size() const {
        return list_.size();
    }

    size_t MemoryUsage() const {
        size_t total = sizeof(std::deque<std::string>);
        for (auto& s : list_) total += s.size() + sizeof(std::string);
        return total;
    }

private:
    std::deque<std::string> list_;
};
