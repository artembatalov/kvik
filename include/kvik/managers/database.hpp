#pragma once

#include <string>
#include <vector>

#include "../engine/data.hpp"
#include "../managers/type.hpp"
#include "../utils/glob.hpp"

class DatabaseManager : public DatabaseTypeManager {
 public:
  using Args = std::vector<std::string>;

  DatabaseManager(DatabaseData& data) : DatabaseTypeManager(data) {
    commands_ = {

        {"TYPE",
         [&](const Args& args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'type' command");
           }
           return data_.Type(args[1]) + "\n";
         }},

        {"DEL",
         [&](const Args& args) -> std::string {
           if (args.size() < 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'del' command");
           }
           int deleted = 0;
           for (int i = 1; i < (int)args.size(); i++) {
             if (data_.IsKey(args[i])) {
               data_.Delete(args[i]);
               deleted++;
             }
           }
           return "(integer) " + std::to_string(deleted) + "\n";
         }},

        {"EXISTS",
         [&](const Args& args) -> std::string {
           if (args.size() < 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'exists' command");
           }
           int count = 0;
           for (int i = 1; i < (int)args.size(); i++) {
             if (data_.IsKey(args[i])) count++;
           }
           return "(integer) " + std::to_string(count) + "\n";
         }},

        {"KEYS",
         [&](const Args& args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'keys' command");
           }
           auto all_keys = data_.Keys();
           std::vector<std::string> matched;
           matched.reserve(all_keys.size());
           for (auto& k : all_keys) {
             if (GlobMatch(args[1], k)) matched.push_back(k);
           }
           if (matched.empty()) return "(empty array)\n";
           std::string res;
           res.reserve(matched.size() * 16);
           for (int i = 0; i < (int)matched.size(); i++) {
             res += std::to_string(i + 1);
             res += ") \"";
             res += matched[i];
             res += "\"\n";
           }
           return res;
         }},

        {"FLUSHDB",
         [&](const Args&) -> std::string {
           data_.Clear();
           return "OK\n";
         }},

        {"CONFIG SET",
         [&](const Args& args) -> std::string {
           if (args.size() != 3 || args[1] != "maxmemory") {
             throw std::runtime_error("ERR syntax error");
           }
           data_.SetMaxMemory(ParseMemory(args[2]));
           return "OK\n";
         }},

        {"CONFIG GET",
         [&](const Args& args) -> std::string {
           if (args.size() != 2 || args[1] != "maxmemory") {
             throw std::runtime_error("ERR syntax error");
           }
           return "1) \"maxmemory\"\n2) \"" +
                  std::to_string(data_.GetMaxMemory()) + "\"\n";
         }},

        {"DBSIZE",
         [&](const Args&) -> std::string {
           return "(integer) " + std::to_string(data_.Size()) + "\n";
         }},

        {"MEMORY USAGE",
         [&](const Args& args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'memory|usage' command");
           }
           if (!data_.IsKey(args[1])) return "(nil)\n";
           return "(integer) " + std::to_string(data_.MemoryUsage(args[1])) +
                  "\n";
         }},

        {"EXPIRE",
         [&](const Args& args) -> std::string {
           if (args.size() != 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'expire' command");
           }
           if (!data_.IsKey(args[1])) return "(integer) 0\n";
           int seconds;
           try {
             seconds = std::stoi(args[2]);
           } catch (...) {
             throw std::runtime_error(
                 "ERR value is not an integer or out of range");
           }
           data_.SetTTL(args[1], seconds);
           return "(integer) 1\n";
         }},

        {"TTL",
         [&](const Args& args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'ttl' command");
           }
           return "(integer) " + std::to_string(data_.GetTTL(args[1])) + "\n";
         }},

        {"PERSIST", [&](const Args& args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'persist' command");
           }
           return data_.RemoveTTL(args[1]) ? "(integer) 1\n" : "(integer) 0\n";
         }}};
  }

 private:
  static uint64_t ParseMemory(const std::string& s) {
    if (s.empty()) return 0;
    size_t i = s.size() - 1;
    while (i > 0 && !std::isdigit(s[i])) i--;
    uint64_t val;
    try {
      val = std::stoull(s.substr(0, i + 1));
    } catch (...) {
      throw std::runtime_error("ERR value is not an integer or out of range");
    }
    std::string suffix = s.substr(i + 1);
    for (auto& c : suffix) c = std::tolower(c);
    if (suffix == "kb") return val * 1024;
    if (suffix == "mb") return val * 1024 * 1024;
    if (suffix == "gb") return val * 1024 * 1024 * 1024;
    return val;
  }
};
