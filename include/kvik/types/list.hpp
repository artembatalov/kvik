#pragma once
#include <algorithm>
#include <cstddef>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Ordered, index-addressable sequence of strings.
 *
 * Backs the LPUSH/RPUSH/LPOP/RPOP/LRANGE/LINDEX/LSET/LINSERT family of
 * commands. Backed by std::deque so both-end operations are O(1) amortized.
 */
class List {
 public:
  List() = default;

  /**
   * @brief Constructs a List from an initial ordered sequence of elements.
   * @param list Initial elements, head to tail.
   */
  List(const std::vector<std::string>& list)
      : list_(list.begin(), list.end()) {}

  /**
   * @brief Redis-CLI style representation, e.g. "[a b c]".
   * @return Bracketed, space-separated string of all elements.
   */
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

  /**
   * @brief LPUSH semantics: pushes elements to the head one at a time.
   *
   * Elements later in @p other end up closer to the head, matching Redis'
   * one-at-a-time LPUSH behavior (e.g. `LPUSH key a b c` yields `[c, b, a,
   * ...]`).
   * @param other Elements to push, in command-argument order.
   */
  void PushLeft(const std::vector<std::string>& other) {
    list_.insert(list_.begin(), other.rbegin(), other.rend());
  }

  /**
   * @brief RPUSH semantics: appends elements to the tail in order.
   * @param other Elements to push, in command-argument order.
   */
  void PushRight(const std::vector<std::string>& other) {
    list_.insert(list_.end(), other.begin(), other.end());
  }

  /**
   * @brief LPOP semantics: removes up to @p count elements from the head.
   * @param count Maximum number of elements to pop; clamped to the current
   * size.
   * @return Removed elements, head to tail (in original list order).
   */
  std::vector<std::string> PopLeft(int count) {
    count = std::min(count, (int)list_.size());
    std::vector<std::string> result(list_.begin(), list_.begin() + count);
    list_.erase(list_.begin(), list_.begin() + count);
    return result;
  }

  /**
   * @brief RPOP semantics: removes up to @p count elements from the tail.
   * @param count Maximum number of elements to pop; clamped to the current
   * size.
   * @return Removed elements ordered tail-first (last element first), matching
   * Redis' RPOP-with-count output order.
   */
  std::vector<std::string> PopRight(int count) {
    count = std::min(count, (int)list_.size());
    std::vector<std::string> result(list_.end() - count, list_.end());
    list_.erase(list_.end() - count, list_.end());
    std::reverse(result.begin(), result.end());
    return result;
  }

  /**
   * @brief LRANGE semantics.
   * @param start Start index, inclusive; negative counts from the tail (-1 is
   * the last element).
   * @param stop Stop index, inclusive; negative counts from the tail.
   * @return Elements in [start, stop] after clamping, or an empty vector if
   * the resulting range is invalid.
   */
  std::vector<std::string> Range(int start, int stop) const {
    int n = (int)list_.size();
    if (start < 0) start = n + start;
    if (stop < 0) stop = n + stop;
    start = std::max(0, start);
    stop = std::min(n - 1, stop);
    if (start > stop) return {};
    return std::vector<std::string>(list_.begin() + start,
                                    list_.begin() + stop + 1);
  }

  /**
   * @brief LINDEX semantics.
   * @param idx Index; negative counts from the tail (-1 is the last element).
   * @return Reference to the element at @p idx.
   * @throws std::runtime_error if @p idx is out of range after normalization.
   */
  const std::string& Index(int idx) const {
    int n = (int)list_.size();
    if (idx < 0) idx = n + idx;
    if (idx < 0 || idx >= n) throw std::runtime_error("ERR index out of range");
    return list_[idx];
  }

  /**
   * @brief LSET semantics.
   * @param idx Index to overwrite; negative counts from the tail.
   * @param value New value for the element at @p idx.
   * @throws std::runtime_error if @p idx is out of range after normalization.
   */
  void Set(int idx, const std::string& value) {
    int n = (int)list_.size();
    if (idx < 0) idx = n + idx;
    if (idx < 0 || idx >= n) throw std::runtime_error("ERR index out of range");
    list_[idx] = value;
  }

  /**
   * @brief LINSERT semantics: inserts a value next to the first occurrence of
   * a pivot element.
   * @param pivot Value to search for.
   * @param value Value to insert.
   * @param before If true, insert immediately before the pivot; otherwise
   * immediately after.
   * @return true if the pivot was found and the insertion happened, false
   * otherwise.
   */
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

  /**
   * @brief LLEN semantics.
   * @return Number of elements in the list.
   */
  size_t Size() const { return list_.size(); }

  /**
   * @brief Approximate memory footprint, used for maxmemory accounting.
   * @return Size in bytes.
   */
  size_t MemoryUsage() const {
    size_t total = sizeof(std::deque<std::string>);
    for (auto& s : list_) total += s.size() + sizeof(std::string);
    return total;
  }

 private:
  std::deque<std::string> list_;
};
