#pragma once
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "../managers/type.hpp"
#include "../types/geoindex.hpp"

class GeoIndexManager : public DatabaseTypeManager {
 public:
  using Args = std::vector<std::string>;

  GeoIndexManager(DatabaseData& data) : DatabaseTypeManager(data) {
    commands_ = {

        {"GEOADD",
         [&](const Args& args) -> std::string {
           if (args.size() < 5 || (args.size() - 2) % 3 != 0) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'geoadd' command");
           }
           if (!data_.IsExist(args[1])) {
             data_.Add<GeoIndex>(args[1], GeoIndex{});
           }
           if (!data_.IsMatch<GeoIndex>(args[1])) {
             throw std::runtime_error("ERR type error");
           }
           auto geo = data_.Modify<GeoIndex>(args[1]);
           int added = 0;
           for (int i = 2; i < (int)args.size(); i += 3) {
             double lon, lat;
             try {
               lon = std::stod(args[i]);
               lat = std::stod(args[i + 1]);
             } catch (...) {
               throw std::runtime_error("ERR value is not a valid float");
             }
             bool exists = geo->HasMember(args[i + 2]);
             geo->Add(lon, lat, args[i + 2]);
             if (!exists) added++;
           }
           return "(integer) " + std::to_string(added) + "\n";
         }},

        {"GEOPOS",
         [&](const Args& args) -> std::string {
           if (args.size() < 3) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'geopos' command");
           }
           std::string res;
           res.reserve((args.size() - 2) * 48);
           bool exists = data_.IsExist(args[1]);
           if (exists && !data_.IsMatch<GeoIndex>(args[1])) {
             throw std::runtime_error("WRONGTYPE");
           }
           for (int i = 2; i < (int)args.size(); i++) {
             int idx = i - 1;
             if (!exists) {
               res += std::to_string(idx);
               res += ") (nil)\n";
               continue;
             }
             auto pos = data_.View<GeoIndex>(args[1]).Pos(args[i]);
             if (!pos) {
               res += std::to_string(idx);
               res += ") (nil)\n";
             } else {
               res += std::to_string(idx);
               res += ") 1) \"";
               res += FormatDouble(pos->lon);
               res += "\"\n   2) \"";
               res += FormatDouble(pos->lat);
               res += "\"\n";
             }
           }
           return res;
         }},

        {"GEODIST",
         [&](const Args& args) -> std::string {
           if (args.size() < 4) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'geodist' command");
           }
           if (!data_.IsExist(args[1])) return "(nil)\n";
           if (!data_.IsMatch<GeoIndex>(args[1])) {
             throw std::runtime_error("WRONGTYPE");
           }
           auto& geo = data_.View<GeoIndex>(args[1]);
           auto dist_m = geo.Dist(args[2], args[3]);
           if (!dist_m) return "(nil)\n";
           std::string unit = (args.size() >= 5) ? args[4] : "m";
           std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
           double converted = GeoIndex::ToMeters(*dist_m, unit);
           return "\"" + FormatDouble(converted) + "\"\n";
         }},

        {"GEOSEARCH",
         [&](const Args& args) -> std::string {
           return GeoSearchImpl(args, "");
         }},

        {"GEOSEARCHSTORE", [&](const Args& args) -> std::string {
           if (args.size() < 2) {
             throw std::runtime_error(
                 "ERR wrong number of arguments for 'geosearchstore' command");
           }
           std::string dest = args[1];
           Args search_args = args;
           search_args.erase(search_args.begin() + 1);
           return GeoSearchImpl(search_args, dest);
         }}};
  }

 private:
  static std::string FormatDouble(double v) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(4) << v;
    return ss.str();
  }

  std::string GeoSearchImpl(const Args& args, const std::string& store_dest) {
    if (args.size() < 8) {
      throw std::runtime_error(
          "ERR wrong number of arguments for 'geosearch' command");
    }
    if (!data_.IsExist(args[1])) return "(empty array)\n";
    if (!data_.IsMatch<GeoIndex>(args[1])) {
      throw std::runtime_error("WRONGTYPE");
    }
    double lon, lat, radius;
    try {
      lon = std::stod(args[3]);
      lat = std::stod(args[4]);
      radius = std::stod(args[6]);
    } catch (...) {
      throw std::runtime_error("ERR value is not a valid float");
    }
    std::string unit = args[7];
    std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
    double radius_m = GeoIndex::FromUnit(radius, unit);

    bool asc = true;
    int count = 0;
    for (int i = 8; i < (int)args.size(); i++) {
      std::string opt = args[i];
      std::transform(opt.begin(), opt.end(), opt.begin(), ::toupper);
      if (opt == "ASC")
        asc = true;
      else if (opt == "DESC")
        asc = false;
      else if (opt == "COUNT" && i + 1 < (int)args.size()) {
        try {
          count = std::stoi(args[++i]);
        } catch (...) {
          throw std::runtime_error(
              "ERR value is not an integer or out of range");
        }
      }
    }

    auto& src = data_.View<GeoIndex>(args[1]);
    auto results = src.Search(lon, lat, radius_m, asc, count);
    if (results.empty()) return "(empty array)\n";

    if (!store_dest.empty()) {
      GeoIndex dest_geo;
      for (auto& r : results) {
        auto pos = src.Pos(r.member);
        if (pos) dest_geo.Add(pos->lon, pos->lat, r.member);
      }
      data_.Add<GeoIndex>(store_dest, dest_geo);
      return "(integer) " + std::to_string(results.size()) + "\n";
    }

    std::string res;
    res.reserve(results.size() * 24);
    for (int i = 0; i < (int)results.size(); i++) {
      res += std::to_string(i + 1);
      res += ") \"";
      res += results[i].member;
      res += "\"\n";
    }
    return res;
  }
};
