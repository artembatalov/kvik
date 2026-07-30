#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "../managers/type.hpp"
#include "../types/list.hpp"

class ListManager : public DatabaseTypeManager {
 public:
  using Args = std::vector<std::string>;

  ListManager(DatabaseData &data) : DatabaseTypeManager(data) {
    commands_ = {

        {"LPUSH",
         [&](const Args &args) -> std::string {
           if (args.size() < 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'lpush' command");
           }
           if (!data_.IsExist<List>(args[1])) {
             data_.Add<List>(args[1], List{});
           }
           auto &list = data_.Get<List>(args[1]);
           list.PushLeft(
               std::vector<std::string>(args.begin() + 2, args.end()));
           return "(integer) " + std::to_string(list.Size()) + "\n";
         }},

        {"RPUSH",
         [&](const Args &args) -> std::string {
           if (args.size() < 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'rpush' command");
           }
           if (!data_.IsExist<List>(args[1])) {
             data_.Add<List>(args[1], List{});
           }
           auto &list = data_.Get<List>(args[1]);
           list.PushRight(
               std::vector<std::string>(args.begin() + 2, args.end()));
           return "(integer) " + std::to_string(list.Size()) + "\n";
         }},

        {"LPOP",
         [&](const Args &args) -> std::string {
           if (args.size() < 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'lpop' command");
           }
           if (!data_.IsExist<List>(args[1])) return "(nil)\n";
           int count = 1;
           if (args.size() >= 3) {
             count = ParseInt(args[2]);
             if (count < 0) {
               throw std::runtime_error(
                   "ERR value is out of range, must be positive");
             }
           }
           auto &list = data_.Get<List>(args[1]);
           auto popped = list.PopLeft(count);
           bool empty = (list.Size() == 0);
           if (empty) data_.Delete(args[1]);
           if (popped.empty()) return "(nil)\n";
           if (args.size() < 3) return "\"" + popped[0] + "\"\n";
           return FormatList(popped);
         }},

        {"RPOP",
         [&](const Args &args) -> std::string {
           if (args.size() < 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'rpop' command");
           }
           if (!data_.IsExist<List>(args[1])) return "(nil)\n";
           int count = 1;
           if (args.size() >= 3) {
             count = ParseInt(args[2]);
             if (count < 0) {
               throw std::runtime_error(
                   "ERR value is out of range, must be positive");
             }
           }
           auto &list = data_.Get<List>(args[1]);
           auto popped = list.PopRight(count);
           bool empty = (list.Size() == 0);
           if (empty) data_.Delete(args[1]);
           if (popped.empty()) return "(nil)\n";
           if (args.size() < 3) return "\"" + popped[0] + "\"\n";
           return FormatList(popped);
         }},

        {"LLEN",
         [&](const Args &args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'llen' command");
           }
           if (!data_.IsExist<List>(args[1])) return "(integer) 0\n";
           return "(integer) " +
                  std::to_string(data_.Get<List>(args[1]).Size()) + "\n";
         }},

        {"LRANGE",
         [&](const Args &args) -> std::string {
           if (args.size() != 4) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'lrange' command");
           }
           if (!data_.IsExist<List>(args[1])) return "(empty array)\n";
           auto elems = data_.Get<List>(args[1]).Range(ParseInt(args[2]),
                                                       ParseInt(args[3]));
           if (elems.empty()) return "(empty array)\n";
           return FormatList(elems);
         }},

        {"LINDEX",
         [&](const Args &args) -> std::string {
           if (args.size() != 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'lindex' command");
           }
           if (!data_.IsExist<List>(args[1])) return "(nil)\n";
           try {
             return "\"" + data_.Get<List>(args[1]).Index(ParseInt(args[2])) +
                    "\"\n";
           } catch (const std::runtime_error &e) {
             if (std::string(e.what()) == "ERR index out of range")
               return "(nil)\n";
             throw;
           }
         }},

        {"LSET",
         [&](const Args &args) -> std::string {
           if (args.size() != 4) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'lset' command");
           }
           if (!data_.IsExist<List>(args[1])) {
             throw std::runtime_error("ERR no such key");
           }
           data_.Get<List>(args[1]).Set(ParseInt(args[2]), args[3]);
           return "OK\n";
         }},

        {"LINSERT", [&](const Args &args) -> std::string {
           if (args.size() != 5) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'linsert' command");
           }
           if (!data_.IsExist<List>(args[1])) return "(integer) 0\n";
           std::string dir = args[2];
           std::transform(dir.begin(), dir.end(), dir.begin(), ::toupper);
           if (dir != "BEFORE" && dir != "AFTER") {
             throw std::runtime_error("ERR syntax error");
           }
           bool before = (dir == "BEFORE");
           auto &list = data_.Get<List>(args[1]);
           bool ok = list.Insert(args[3], args[4], before);
           if (!ok) return "(integer) -1\n";
           return "(integer) " + std::to_string(list.Size()) + "\n";
         }}};
  }

 private:
  static int ParseInt(const std::string &s) {
    try {
      return std::stoi(s);
    } catch (...) {
      throw std::runtime_error("ERR value is not an integer or out of range");
    }
  }

  static std::string FormatList(const std::vector<std::string> &elems) {
    std::string res;
    res.reserve(elems.size() * 16);
    for (int i = 0; i < (int)elems.size(); i++) {
      res += std::to_string(i + 1);
      res += ") \"";
      res += elems[i];
      res += "\"\n";
    }
    return res;
  }
};
