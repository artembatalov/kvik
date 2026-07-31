#pragma once
#include <string>
#include <unordered_set>
#include <vector>

class Set {
 public:
  Set() = default;

  Set(std::vector<std::string>& set) : set_(set.begin(), set.end()) {}

  Set(std::vector<std::string>&& set) : set_(set.begin(), set.end()) {}


  void Add(const std::vector<std::string>& elems) {
    set_.insert(elems.begin(), elems.end());
  }

  void Add(const Set& other) {
    set_.insert(other.set_.begin(), other.set_.end());
  }

  void Remove(const std::vector<std::string>& elems) {
    for (auto& e : elems) set_.erase(e);
  }

  bool IsMember(const std::string& elem) const { return set_.count(elem) > 0; }

  Set Intersect(const Set& other) const {
    Set result;
    for (auto& e : set_) {
      if (other.IsMember(e)) result.set_.insert(e);
    }
    return result;
  }

  Set Diff(const Set& other) const {
    Set result;
    for (auto& e : set_) {
      if (!other.IsMember(e)) result.set_.insert(e);
    }
    return result;
  }

  const std::unordered_set<std::string>& GetSet() const { return set_; }

  std::string GetString() const {
    std::string res;
    res.reserve(set_.size() * 10 + 2);
    res += "(";
    bool first = true;
    for (auto& x : set_) {
      if (!first) res += ", ";
      res += x;
      first = false;
    }
    res += ")";
    return res;
  }

  size_t Size() const { return set_.size(); }

  size_t MemoryUsage() const {
    size_t total = sizeof(std::unordered_set<std::string>);
    for (auto& s : set_) total += s.size() + sizeof(std::string) + 32;
    return total;
  }

 private:
  std::unordered_set<std::string> set_;
};
