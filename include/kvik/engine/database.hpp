#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../managers/database.hpp"
#include "../managers/geoindex.hpp"
#include "../managers/list.hpp"
#include "../managers/set.hpp"
#include "../managers/string.hpp"
#include "../managers/type.hpp"
#include "data.hpp"
#include "parser.hpp"

class Database {
 public:
  using Args = std::vector<std::string>;

  using Commands =
      std::unordered_map<std::string, std::function<std::string(const Args&)>>;

  Database(uint64_t max_memory = 0) : data_(DatabaseData(max_memory)) {
    managers_.push_back(std::make_unique<DatabaseManager>(data_));
    managers_.push_back(std::make_unique<StringManager>(data_));
    managers_.push_back(std::make_unique<SetManager>(data_));
    managers_.push_back(std::make_unique<ListManager>(data_));
    managers_.push_back(std::make_unique<GeoIndexManager>(data_));

    for (auto& ptr : managers_) {
      for (auto& [cmd, fnc] : ptr->GetCommands()) {
        commands_[cmd] = fnc;
      }
    }
  }

  std::string ProcessCommand(const std::string& cmd) {
    Args parsed = parser_.Parse(cmd);
    auto found_it = commands_.find(parsed[0]);
    if (found_it == commands_.end()) {
      throw std::runtime_error("ERR unknown command '" + parsed[0] + "'");
    }
    if (data_.IsEnabledTTL()) {
      data_.CleanExpired();
    }
    return found_it->second(parsed);
  }

 private:
  std::vector<std::unique_ptr<DatabaseTypeManager>> managers_;
  DatabaseData data_;
  Commands commands_;

  DatabaseParser parser_;
};