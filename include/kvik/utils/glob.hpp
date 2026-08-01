#pragma once

#include <string_view>

inline bool MatchClass(std::string_view pattern, size_t start, char c,
                size_t& out_end) {
  bool negate = false;
  size_t i = start;
  if (i < pattern.size() && (pattern[i] == '^' || pattern[i] == '!')) {
    negate = true;
    ++i;
  }
  bool found = false;
  while (i < pattern.size() && pattern[i] != ']') {
    if (pattern[i] == '\\' && i + 1 < pattern.size()) {
      if (pattern[i + 1] == c) found = true;
      i += 2;
    } else if (i + 2 < pattern.size() && pattern[i + 1] == '-' &&
               pattern[i + 2] != ']') {
      if (c >= pattern[i] && c <= pattern[i + 2]) found = true;
      i += 3;
    } else {
      if (pattern[i] == c) found = true;
      ++i;
    }
  }
  out_end = (i < pattern.size()) ? i + 1 : i;
  return found != negate;
}

inline bool GlobMatch(std::string_view pattern, std::string_view str) {
  size_t p = 0, s = 0;
  size_t star_p = std::string_view::npos, star_s = 0;
  while (s < str.size()) {
    if (p < pattern.size() && pattern[p] == '*') {
      star_p = p++;
      star_s = s;
      continue;
    }
    if (p < pattern.size() && pattern[p] == '?') {
      ++p;
      ++s;
      continue;
    }
    if (p < pattern.size() && pattern[p] == '[') {
      size_t end = 0;
      if (MatchClass(pattern, p + 1, str[s], end)) {
        p = end;
        ++s;
        continue;
      }
    } else if (p < pattern.size() && pattern[p] == '\\' &&
               p + 1 < pattern.size() && pattern[p + 1] == str[s]) {
      p += 2;
      ++s;
      continue;
    } else if (p < pattern.size() && pattern[p] != '*' &&
               pattern[p] == str[s]) {
      ++p;
      ++s;
      continue;
    }
    if (star_p != std::string_view::npos) {
      p = star_p + 1;
      s = ++star_s;
    } else {
      return false;
    }
  }
  while (p < pattern.size() && pattern[p] == '*') ++p;
  return p == pattern.size();
}