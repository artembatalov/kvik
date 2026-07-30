#pragma once
#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class GeoIndex {
 public:
  struct Point {
    double lon, lat;
  };

  struct SearchResult {
    std::string member;
    double dist_m;
  };

  GeoIndex() = default;

  void Add(double lon, double lat, const std::string& member) {
    points_[member] = {lon, lat};
  }

  std::optional<Point> Pos(const std::string& member) const {
    auto it = points_.find(member);
    if (it == points_.end()) return std::nullopt;
    return it->second;
  }

  std::optional<double> Dist(const std::string& m1,
                             const std::string& m2) const {
    auto p1 = Pos(m1);
    auto p2 = Pos(m2);
    if (!p1 || !p2) return std::nullopt;
    return Haversine(p1->lat, p1->lon, p2->lat, p2->lon);
  }

  std::vector<SearchResult> Search(double lon, double lat, double radius_m,
                                   bool asc, int count) const {
    std::vector<SearchResult> results;
    for (auto& [name, pt] : points_) {
      double d = Haversine(lat, lon, pt.lat, pt.lon);
      if (d <= radius_m) results.push_back({name, d});
    }
    if (asc) {
      std::sort(results.begin(), results.end(),
                [](auto& a, auto& b) { return a.dist_m < b.dist_m; });
    } else {
      std::sort(results.begin(), results.end(),
                [](auto& a, auto& b) { return a.dist_m > b.dist_m; });
    }
    if (count > 0 && (int)results.size() > count) results.resize(count);
    return results;
  }

  bool HasMember(const std::string& member) const {
    return points_.count(member) > 0;
  }

  const std::unordered_map<std::string, Point>& GetPoints() const {
    return points_;
  }

  size_t Size() const { return points_.size(); }

  size_t MemoryUsage() const {
    size_t total = sizeof(std::unordered_map<std::string, Point>);
    for (auto& [k, v] : points_) total += k.size() + sizeof(Point) + 32;
    return total;
  }

  static double ToMeters(double dist_m, const std::string& unit) {
    if (unit == "km") return dist_m / 1000.0;
    if (unit == "mi") return dist_m / 1609.344;
    if (unit == "ft") return dist_m / 0.3048;
    return dist_m;
  }

  static double FromUnit(double val, const std::string& unit) {
    if (unit == "km") return val * 1000.0;
    if (unit == "mi") return val * 1609.344;
    if (unit == "ft") return val * 0.3048;
    return val;
  }

 private:
  static double Haversine(double lat1, double lon1, double lat2, double lon2) {
    const double r = 6372800.0;
    double dlat = (lat2 - lat1) * M_PI / 180.0;
    double dlon = (lon2 - lon1) * M_PI / 180.0;
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
               std::cos(lat1) * std::cos(lat2) * std::sin(dlon / 2) *
                   std::sin(dlon / 2);
    return r * 2.0 * std::asin(std::sqrt(a));
  }

  std::unordered_map<std::string, Point> points_;
};
