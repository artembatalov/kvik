#pragma once
#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

/// Pi, used for degree-to-radian conversion in Haversine().
inline constexpr double kPi = 3.14159265358979323846;

/**
 * @brief Geospatial index mapping member names to (longitude, latitude)
 * points.
 *
 * Backs the GEOADD/GEOPOS/GEODIST/GEOSEARCH family of commands. Reported as
 * type "geoindex" (Redis/Valkey report "zset" for the equivalent structure).
 */
class GeoIndex {
 public:
  /// A longitude/latitude pair, in degrees.
  struct Point {
    double lon, lat;
  };

  /// One hit from Search(): a member name and its distance from the query
  /// point, in meters.
  struct SearchResult {
    std::string member;
    double dist_m;
  };

  GeoIndex() = default;

  /**
   * @brief GEOADD semantics: inserts or overwrites a member's position.
   * @param lon Longitude, in degrees.
   * @param lat Latitude, in degrees.
   * @param member Member name.
   */
  void Add(double lon, double lat, const std::string& member) {
    points_[member] = {lon, lat};
  }

  /**
   * @brief GEOPOS semantics.
   * @param member Member to look up.
   * @return The member's Point, or std::nullopt if it is not indexed.
   */
  std::optional<Point> Pos(const std::string& member) const {
    auto it = points_.find(member);
    if (it == points_.end()) return std::nullopt;
    return it->second;
  }

  /**
   * @brief GEODIST semantics: great-circle distance between two members.
   * @param m1 First member.
   * @param m2 Second member.
   * @return Distance in meters, or std::nullopt if either member is not
   * indexed.
   */
  std::optional<double> Dist(const std::string& m1,
                             const std::string& m2) const {
    auto p1 = Pos(m1);
    auto p2 = Pos(m2);
    if (!p1 || !p2) return std::nullopt;
    return Haversine(p1->lat, p1->lon, p2->lat, p2->lon);
  }

  /**
   * @brief GEOSEARCH-style radius query around a point.
   * @param lon Query longitude, in degrees.
   * @param lat Query latitude, in degrees.
   * @param radius_m Search radius, in meters.
   * @param asc Sort ascending by distance if true, descending if false.
   * @param count Maximum number of results to return; non-positive means
   * unlimited.
   * @return Members within @p radius_m of the query point, sorted by
   * distance.
   */
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

  /**
   * @brief Checks whether a member is indexed.
   * @param member Member to test.
   * @return true if @p member has a stored position.
   */
  bool HasMember(const std::string& member) const {
    return points_.count(member) > 0;
  }

  /**
   * @brief Returns the underlying member-to-position map.
   * @return Reference to the underlying unordered_map.
   */
  const std::unordered_map<std::string, Point>& GetPoints() const {
    return points_;
  }

  /**
   * @brief ZCARD-equivalent for the geo index.
   * @return Number of indexed members.
   */
  size_t Size() const { return points_.size(); }

  /**
   * @brief Approximate memory footprint, used for maxmemory accounting.
   * @return Size in bytes.
   */
  size_t MemoryUsage() const {
    size_t total = sizeof(std::unordered_map<std::string, Point>);
    for (auto& [k, v] : points_) total += k.size() + sizeof(Point) + 32;
    return total;
  }

  /**
   * @brief Converts a distance in meters into another unit.
   *
   * Inverse of FromUnit().
   * @param dist_m Distance, in meters.
   * @param unit Target unit: "km", "mi", or "ft"; anything else (including
   * "m") is returned unchanged.
   * @return Distance expressed in @p unit.
   */
  static double ToMeters(double dist_m, const std::string& unit) {
    if (unit == "km") return dist_m / 1000.0;
    if (unit == "mi") return dist_m / 1609.344;
    if (unit == "ft") return dist_m / 0.3048;
    return dist_m;
  }

  /**
   * @brief Converts a distance expressed in a given unit into meters.
   *
   * Inverse of ToMeters().
   * @param val Distance value, expressed in @p unit.
   * @param unit Source unit: "km", "mi", or "ft"; anything else (including
   * "m") is treated as meters already.
   * @return Distance in meters.
   */
  static double FromUnit(double val, const std::string& unit) {
    if (unit == "km") return val * 1000.0;
    if (unit == "mi") return val * 1609.344;
    if (unit == "ft") return val * 0.3048;
    return val;
  }

 private:
  /**
   * @brief Great-circle distance between two lat/lon points (haversine
   * formula), using an Earth radius of 6372800 m.
   * @param lat1 Latitude of the first point, in degrees.
   * @param lon1 Longitude of the first point, in degrees.
   * @param lat2 Latitude of the second point, in degrees.
   * @param lon2 Longitude of the second point, in degrees.
   * @return Distance in meters.
   */
  static double Haversine(double lat1, double lon1, double lat2, double lon2) {
    const double r = 6372800.0;
    double dlat = (lat2 - lat1) * kPi / 180.0;
    double dlon = (lon2 - lon1) * kPi / 180.0;
    lat1 = lat1 * kPi / 180.0;
    lat2 = lat2 * kPi / 180.0;
    double a = std::sin(dlat / 2) * std::sin(dlat / 2) +
               std::cos(lat1) * std::cos(lat2) * std::sin(dlon / 2) *
                   std::sin(dlon / 2);
    return r * 2.0 * std::asin(std::sqrt(a));
  }

  std::unordered_map<std::string, Point> points_;
};
