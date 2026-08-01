#pragma once
#include <string>
#include <unordered_set>
#include <vector>

/**
 * @brief Unordered collection of strings
 *
 * Implement commands of family SADD/SREM/SMEMBERS. Duplicates are ignored while
 * inserting. Order is not guaranteed.
 */
class Set {
 public:
  Set() = default;

  /**
   * @brief Constructs a Set from a vector of elements, collapsing duplicates.
   * @param set Elements to seed the set with (lvalue overload).
   */
  Set(std::vector<std::string>& set) : set_(set.begin(), set.end()) {}

  /**
   * @brief Constructs a Set from a vector of elements, collapsing duplicates.
   * @param set Elements to seed the set with (rvalue overload).
   */
  Set(std::vector<std::string>&& set) : set_(set.begin(), set.end()) {}

  /**
   * @brief SADD semantics: inserts elements, ignoring ones already present.
   * @param elems Elements to add.
   */
  void Add(const std::vector<std::string>& elems) {
    set_.insert(elems.begin(), elems.end());
  }

  /**
   * @brief Inserts all members of another Set into this one (union-in-place).
   * @param other Set whose members to add.
   */
  void Add(const Set& other) {
    set_.insert(other.set_.begin(), other.set_.end());
  }

  /**
   * @brief SREM semantics: removes elements if present.
   * @param elems Elements to remove.
   */
  void Remove(const std::vector<std::string>& elems) {
    for (auto& e : elems) set_.erase(e);
  }

  /**
   * @brief SISMEMBER semantics.
   * @param elem Element to test.
   * @return true if @p elem is a member of the set.
   */
  bool IsMember(const std::string& elem) const { return set_.count(elem) > 0; }

  /**
   * @brief SINTER semantics.
   * @param other Set to intersect with.
   * @return New Set containing elements present in both sets.
   */
  Set Intersect(const Set& other) const {
    Set result;
    for (auto& e : set_) {
      if (other.IsMember(e)) result.set_.insert(e);
    }
    return result;
  }

  /**
   * @brief SDIFF semantics.
   * @param other Set to subtract.
   * @return New Set containing elements present in this set but not in
   * @p other.
   */
  Set Diff(const Set& other) const {
    Set result;
    for (auto& e : set_) {
      if (!other.IsMember(e)) result.set_.insert(e);
    }
    return result;
  }

  /**
   * @brief Returns the underlying member collection.
   * @return Reference to the underlying unordered_set.
   */
  const std::unordered_set<std::string>& GetSet() const { return set_; }

  /**
   * @brief Redis-CLI style representation, e.g. "(a, b, c)".
   * @return Parenthesized, comma-separated string of all members.
   */
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

  /**
   * @brief SCARD semantics.
   * @return Number of members in the set.
   */
  size_t Size() const { return set_.size(); }

  /**
   * @brief Approximate memory footprint, used for maxmemory accounting.
   * @return Size in bytes.
   */
  size_t MemoryUsage() const {
    size_t total = sizeof(std::unordered_set<std::string>);
    for (auto& s : set_) total += s.size() + sizeof(std::string) + 32;
    return total;
  }

 private:
  std::unordered_set<std::string> set_;
};
