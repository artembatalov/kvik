#pragma once
#include <string>
#include <vector>

#include "../managers/type.hpp"
#include "../types/string.hpp"

class StringManager : public DatabaseTypeManager {
 public:
  using Args = std::vector<std::string>;

  StringManager(DatabaseData &data) : DatabaseTypeManager(data) {
    commands_ = {

        {"SET",
         [&](const Args &args) -> std::string {
           if (args.size() != 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'set' command");
           }
           data_.Add<String>(args[1], String(args[2]));
           return "OK\n";
         }},

        {"GET",
         [&](const Args &args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'get' command");
           }
           if (!data_.IsExist<String>(args[1])) return "(nil)\n";
           return "\"" + data_.View<String>(args[1]).GetString() + "\"\n";
         }},

        {"STRLEN",
         [&](const Args &args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'strlen' command");
           }
           if (!data_.IsExist<String>(args[1])) return "(integer) 0\n";
           return "(integer) " +
                  std::to_string(data_.View<String>(args[1]).Size()) + "\n";
         }},

        {"APPEND", [&](const Args &args) -> std::string {
           if (args.size() != 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'append' command");
           }
           if (data_.IsExist<String>(args[1])) {
             auto s = data_.Modify<String>(args[1]);
             s->Append(args[2]);
             return "(integer) " + std::to_string(s->Size()) + "\n";
           }
           data_.Add<String>(args[1], String(args[2]));
           return "(integer) " + std::to_string(args[2].size()) + "\n";
         }}};
  }
};
