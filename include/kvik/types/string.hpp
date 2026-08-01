#pragma once
#include <cstddef>
#include <string>

/**
 * @brief Plain string value.
 *
 * Backs the SET/GET/APPEND/STRLEN family of commands. Thin wrapper around
 * std::string with no knowledge of keys or TTL.
 */
class String {
 public:
  /**
   * @brief Constructs a String holding the given value.
   * @param string Initial value to store.
   */
  String(std::string string) : str_(std::move(string)) {}

  /**
   * @brief Returns the stored value.
   * @return Reference to the stored string.
   */
  const std::string& GetString() const { return str_; }

  /**
   * @brief STRLEN semantics.
   * @return Length of the stored string, in bytes.
   */
  size_t Size() const { return str_.size(); }

  /**
   * @brief APPEND semantics: appends to the end of the stored value.
   * @param other String to append.
   */
  void Append(const std::string& other) { str_ += other; }

  /**
   * @brief Approximate memory footprint, used for maxmemory accounting.
   * @return Size in bytes.
   */
  size_t MemoryUsage() const { return str_.size() + sizeof(std::string); }

 private:
  std::string str_;
};
