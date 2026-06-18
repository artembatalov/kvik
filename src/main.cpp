#include "../include/kvik/engine/database.hpp"
#include <cctype>
#include <cstdint>
#include <ctime>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

uint64_t ParseMemoryArg(const std::string &s) {
  if (s.empty())
    return 0;
  size_t i = s.size() - 1;
  while (i > 0 && !std::isdigit(s[i]))
    i--;
  uint64_t val = std::stoull(s.substr(0, i + 1));
  std::string suffix = s.substr(i + 1);
  for (auto &c : suffix)
    c = std::tolower(c);
  if (suffix == "kb")
    return val * 1024;
  if (suffix == "mb")
    return val * 1024 * 1024;
  if (suffix == "gb")
    return val * 1024 * 1024 * 1024;
  return val;
}

void EventHandling(Database &db) {
  std::string cmd;
  while (std::getline(std::cin, cmd)) {
    if (cmd == "EXIT")
      return;
    try {
      std::cout << db.ProcessCommand(cmd);
    } catch (std::exception &e) {
      std::cerr << "(error) " << e.what() << "\n";
    }
  }
}

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  try {
    uint64_t max_memory = 0;
    if (argc > 3 || argc == 2) {
      throw std::runtime_error("Wrong number of arguments!");
    }
    if (argc == 3 && std::string(argv[1]) == "--maxmemory") {
      max_memory = ParseMemoryArg(argv[2]);
    }
    Database db(max_memory);
    EventHandling(db);
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  }
}