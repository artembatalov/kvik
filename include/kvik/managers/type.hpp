#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../engine/data.hpp"

class DatabaseTypeManager {
 public:
  using Args = std::vector<std::string>;
  using Commands =
      std::unordered_map<std::string, std::function<std::string(const Args &)>>;

  DatabaseTypeManager(DatabaseData &data) : data_(data) {}

  bool IsCommand(std::string cmd) { return commands_.contains(cmd); }

  std::string Execute(Args cmd) { return commands_[cmd[0]](cmd); }

  const Commands &GetCommands() { return commands_; }

 protected:
  Commands commands_;
  DatabaseData &data_;
};