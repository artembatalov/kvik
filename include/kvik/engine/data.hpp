#pragma once
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "../types/geoindex.hpp"
#include "../types/list.hpp"
#include "../types/set.hpp"
#include "../types/string.hpp"

class DatabaseData {
 public:
  DatabaseData(const uint64_t &max_memory = 0) {
    max_memory_ = max_memory;
    max_memory_set_ = (max_memory_ != 0);
    ttl_enabled_ = false;
  }

  template <typename Type>
  void Add(const std::string &key, const Type &value) {
    auto it = data_.find(key);
    size_t old_mem =
        (it != data_.end()) ? EntryMemory(it->first, it->second) : 0;
    size_t new_mem = key.size() + 64 + EstimateValueSize(value);
    if (max_memory_set_) {
      if (current_memory_ - old_mem + new_mem > max_memory_)
        throw std::runtime_error(
            "OOM command not allowed when used memory > 'maxmemory'");
    }
    current_memory_ = current_memory_ - old_mem + new_mem;
    if (it != data_.end()) {
      it->second = Value{.element = value, .ttl = 0};
    } else {
      data_.emplace(key, Value{.element = value, .ttl = 0});
    }
  }

  template <typename Type>
  Type &Get(const std::string &key) {
    auto it = data_.find(key);
    if (it == data_.end()) throw std::runtime_error("ERR no such key");
    if (it->second.ttl > 0 && now_ > 0 && now_ >= it->second.ttl) {
      current_memory_ -= EntryMemory(it->first, it->second);
      data_.erase(it);
      throw std::runtime_error("ERR no such key");
    }
    if (!std::holds_alternative<Type>(it->second.element))
      throw std::runtime_error(
          "WRONGTYPE Operation against a key holding the wrong kind of value");
    return std::get<Type>(it->second.element);
  }

  template <typename Type>
  Type Extract(const std::string &key) {
    auto it = data_.find(key);
    if (it == data_.end()) throw std::runtime_error("ERR no such key");
    if (!std::holds_alternative<Type>(it->second.element))
      throw std::runtime_error(
          "WRONGTYPE Operation against a key holding the wrong kind of value");
    size_t mem = EntryMemory(it->first, it->second);
    Type temp = std::get<Type>(std::move(it->second.element));
    current_memory_ -= mem;
    data_.erase(it);
    return temp;
  }

  void Delete(const std::string &key) {
    auto it = data_.find(key);
    if (it == data_.end()) return;
    current_memory_ -= EntryMemory(it->first, it->second);
    data_.erase(it);
  }

  template <typename Type>
  bool IsExist(const std::string &key) {
    auto it = data_.find(key);
    if (it == data_.end()) return false;
    if (it->second.ttl > 0 && now_ > 0 && now_ >= it->second.ttl) {
      current_memory_ -= EntryMemory(it->first, it->second);
      data_.erase(it);
      return false;
    }
    if (!std::holds_alternative<Type>(it->second.element))
      throw std::runtime_error(
          "WRONGTYPE Operation against a key holding the wrong kind of value");
    return true;
  }

  bool IsKey(const std::string &key) { return data_.contains(key); }

  std::string Type(const std::string &key) {
    auto it = data_.find(key);
    if (it == data_.end()) return "none";
    return std::visit(
        Overloaded{[](String &) -> std::string { return "string"; },
                   [](List &) -> std::string { return "list"; },
                   [](Set &) -> std::string { return "set"; },
                   [](GeoIndex &) -> std::string { return "geoindex"; },
                   [](std::monostate &) -> std::string { return "none"; }},
        it->second.element);
  }

  std::vector<std::string> Keys() {
    std::vector<std::string> keys;
    keys.reserve(data_.size());
    for (auto &[k, v] : data_) keys.push_back(k);
    return keys;
  }

  void Clear() {
    data_.clear();
    current_memory_ = 0;
  }

  size_t Size() { return data_.size(); }

  void SetTTL(const std::string &key, const int &seconds) {
    auto it = data_.find(key);
    if (it == data_.end()) throw std::runtime_error("ERR no such key");
    time_t exp = std::time(nullptr) + seconds;
    it->second.ttl = exp;
    expiry_queue_.push({exp, key});
    ttl_enabled_ = true;
  }

  int GetTTL(const std::string &key) const {
    auto it = data_.find(key);
    if (it == data_.end()) return -2;
    if (it->second.ttl == 0) return -1;
    return std::max(0, (int)(it->second.ttl - std::time(nullptr)));
  }

  bool RemoveTTL(const std::string &key) {
    auto it = data_.find(key);
    if (it == data_.end() || it->second.ttl == 0) return false;
    it->second.ttl = 0;
    return true;
  }

  bool IsEnabledTTL() const { return ttl_enabled_; }

  void EnableTTL() { ttl_enabled_ = true; }

  void CleanExpired(time_t now = std::time(nullptr)) {
    while (!expiry_queue_.empty() && expiry_queue_.top().first <= now) {
      auto [exp_time, key] = expiry_queue_.top();
      expiry_queue_.pop();
      auto it = data_.find(key);
      if (it != data_.end() && it->second.ttl == exp_time) {
        current_memory_ -= EntryMemory(it->first, it->second);
        data_.erase(it);
      }
    }
  }

  size_t MemoryUsage(const std::string &key) {
    auto it = data_.find(key);
    if (it == data_.end()) return 0;
    return EntryMemory(it->first, it->second);
  }

  size_t TotalMemory() { return current_memory_; }

  void SetMaxMemory(const uint64_t &bytes) {
    max_memory_ = bytes;
    max_memory_set_ = (bytes != 0);
  }

  uint64_t GetMaxMemory() const { return max_memory_; }

  void SetNow(time_t now) { now_ = now; }

 private:
  template <typename... Ts>
  struct Overloaded : Ts... {
    using Ts::operator()...;
  };

  struct Value {
    std::variant<std::monostate, String, GeoIndex, List, Set> element;
    time_t ttl = 0;
  };

  template <typename T>
  static size_t EstimateValueSize(const T &val) {
    if constexpr (std::is_same_v<T, String>)
      return val.MemoryUsage();
    else if constexpr (std::is_same_v<T, List>)
      return val.MemoryUsage();
    else if constexpr (std::is_same_v<T, Set>)
      return val.MemoryUsage();
    else if constexpr (std::is_same_v<T, GeoIndex>)
      return val.MemoryUsage();
    return 0;
  }

  static size_t EntryMemory(const std::string &key, const Value &v) {
    return key.size() + 64 +
           std::visit(
               Overloaded{
                   [](const String &s) -> size_t { return s.MemoryUsage(); },
                   [](const List &l) -> size_t { return l.MemoryUsage(); },
                   [](const Set &s) -> size_t { return s.MemoryUsage(); },
                   [](const GeoIndex &g) -> size_t { return g.MemoryUsage(); },
                   [](const std::monostate &) -> size_t { return 0; }},
               v.element);
  }

  std::unordered_map<std::string, Value> data_;
  uint64_t max_memory_ = 0;
  bool max_memory_set_ = false;
  bool ttl_enabled_ = false;

  std::priority_queue<std::pair<time_t, std::string>,
                      std::vector<std::pair<time_t, std::string>>,
                      std::greater<>>
      expiry_queue_;

  time_t now_ = 0;
  size_t current_memory_ = 0;
};
