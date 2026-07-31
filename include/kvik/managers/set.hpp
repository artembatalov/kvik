#pragma once
#include <string>
#include <vector>

#include "../managers/type.hpp"
#include "../types/set.hpp"

class SetManager : public DatabaseTypeManager {
 public:
  using Args = std::vector<std::string>;

  SetManager(DatabaseData &data) : DatabaseTypeManager(data) {
    commands_ = {

        {"SADD",
         [&](const Args &args) -> std::string {
           if (args.size() < 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'sadd' command");
           }
           if (!data_.IsExist<Set>(args[1])) {
             data_.Add<Set>(args[1], Set{});
           }
           auto s = data_.Modify<Set>(args[1]);
           size_t before = s->Size();
           s->Add(std::vector<std::string>(args.begin() + 2, args.end()));
           return "(integer) " + std::to_string(s->Size() - before) + "\n";
         }},

        {"SREM",
         [&](const Args &args) -> std::string {
           if (args.size() < 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'srem' command");
           }
           size_t after, before;
           if (!data_.IsExist<Set>(args[1])) return "(integer) 0\n";
           {
             auto s = data_.Modify<Set>(args[1]);
             before = s->Size();
             s->Remove(std::vector<std::string>(args.begin() + 2, args.end()));
             after = s->Size();
           }
           if (after == 0) data_.Delete(args[1]);
           return "(integer) " + std::to_string(before - after) + "\n";
         }},

        {"SISMEMBER",
         [&](const Args &args) -> std::string {
           if (args.size() != 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'sismember' command");
           }
           if (!data_.IsExist<Set>(args[1])) return "(integer) 0\n";
           return data_.View<Set>(args[1]).IsMember(args[2]) ? "(integer) 1\n"
                                                             : "(integer) 0\n";
         }},

        {"SMEMBERS",
         [&](const Args &args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'smembers' command");
           }
           if (!data_.IsExist<Set>(args[1])) return "(empty set)\n";
           auto &members = data_.View<Set>(args[1]).GetSet();
           return FormatSet(members);
         }},

        {"SCARD",
         [&](const Args &args) -> std::string {
           if (args.size() != 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'scard' command");
           }
           if (!data_.IsExist<Set>(args[1])) return "(integer) 0\n";
           return "(integer) " +
                  std::to_string(data_.View<Set>(args[1]).Size()) + "\n";
         }},

        {"SUNION",
         [&](const Args &args) -> std::string {
           if (args.size() < 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'sunion' command");
           }
           Set sunion;
           for (int i = 1; i < (int)args.size(); i++) {
             if (data_.IsExist<Set>(args[i]))
               sunion.Add(data_.View<Set>(args[i]));
           }
           auto &members = sunion.GetSet();
           if (members.empty()) return "(empty set)\n";
           return FormatSet(members);
         }},

        {"SINTER",
         [&](const Args &args) -> std::string {
           if (args.size() < 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'sinter' command");
           }
           if (!data_.IsExist<Set>(args[1])) return "(empty set)\n";
           Set result = data_.View<Set>(args[1]);
           for (int i = 2; i < (int)args.size(); i++) {
             if (!data_.IsExist<Set>(args[i])) return "(empty set)\n";
             result = result.Intersect(data_.View<Set>(args[i]));
           }
           auto &members = result.GetSet();
           if (members.empty()) return "(empty set)\n";
           return FormatSet(members);
         }},

        {"SDIFF",
         [&](const Args &args) -> std::string {
           if (args.size() < 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'sdiff' command");
           }
           if (!data_.IsExist<Set>(args[1])) return "(empty set)\n";
           Set result = data_.View<Set>(args[1]);
           for (int i = 2; i < (int)args.size(); i++) {
             if (data_.IsExist<Set>(args[i]))
               result = result.Diff(data_.View<Set>(args[i]));
           }
           auto &members = result.GetSet();
           if (members.empty()) return "(empty set)\n";
           return FormatSet(members);
         }},

        {"SMOVE", [&](const Args &args) -> std::string {
           if (args.size() != 4) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'smove' command");
           }
           bool src_exists = data_.IsExist<Set>(args[1]);
           bool dest_exists = data_.IsExist<Set>(args[2]);
           if (!src_exists) return "(integer) 0\n";
           size_t src_size_after;
           {
             auto src = data_.Modify<Set>(args[1]);
             if (!src->IsMember(args[3])) return "(integer) 0\n";
             src->Remove({args[3]});
             src_size_after = src->Size();
           }
           if (!dest_exists) {
             data_.Add<Set>(args[2], Set{});
           }
           data_.Modify<Set>(args[2])->Add({args[3]});
           if (src_size_after == 0 && args[1] != args[2]) {
             data_.Delete(args[1]);
           }
           return "(integer) 1\n";
         }}};
  }

 private:
  static std::string FormatSet(const std::unordered_set<std::string> &members) {
    std::string res;
    res.reserve(members.size() * 16);
    int i = 1;
    for (auto &m : members) {
      res += std::to_string(i++);
      res += ") \"";
      res += m;
      res += "\"\n";
    }
    return res;
  }
};
