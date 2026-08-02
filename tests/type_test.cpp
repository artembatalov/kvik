#include <gtest/gtest.h>

#include <stdexcept>

#include "kvik/engine/database.hpp"

TEST(TypeString, GetString) {
  String s("hello");
  EXPECT_EQ(s.GetString(), "hello");
}

TEST(TypeString, Size) {
  String s("hello");
  EXPECT_EQ(s.Size(), 5u);
}

TEST(TypeString, Append) {
  String s("hello");
  s.Append(" world");
  EXPECT_EQ(s.GetString(), "hello world");
}

TEST(TypeList, PushLeftSingle) {
  List l;
  l.PushLeft({"a"});
  EXPECT_EQ(l.Index(0), "a");
  EXPECT_EQ(l.Size(), 1u);
}

TEST(TypeList, PushLeftMultiple) {
  List l;
  l.PushLeft({"a", "b", "c"});
  EXPECT_EQ(l.Index(0), "c");
  EXPECT_EQ(l.Index(1), "b");
  EXPECT_EQ(l.Index(2), "a");
}

TEST(TypeList, PushRight) {
  List l;
  l.PushRight({"a", "b", "c"});
  EXPECT_EQ(l.Index(0), "a");
  EXPECT_EQ(l.Index(2), "c");
}

TEST(TypeList, PopLeft) {
  List l({"a", "b", "c"});
  auto popped = l.PopLeft(2);
  EXPECT_EQ(popped[0], "a");
  EXPECT_EQ(popped[1], "b");
  EXPECT_EQ(l.Size(), 1u);
  EXPECT_EQ(l.Index(0), "c");
}

TEST(TypeList, PopRight) {
  List l({"a", "b", "c"});
  auto popped = l.PopRight(1);
  EXPECT_EQ(popped[0], "c");
  EXPECT_EQ(l.Size(), 2u);
}

TEST(TypeList, RangePositive) {
  List l({"a", "b", "c", "d"});
  auto r = l.Range(1, 2);
  EXPECT_EQ(r.size(), 2u);
  EXPECT_EQ(r[0], "b");
  EXPECT_EQ(r[1], "c");
}

TEST(TypeList, RangeNegative) {
  List l({"a", "b", "c", "d"});
  auto r = l.Range(0, -1);
  EXPECT_EQ(r.size(), 4u);
  auto r2 = l.Range(-2, -1);
  EXPECT_EQ(r2[0], "c");
  EXPECT_EQ(r2[1], "d");
}

TEST(TypeList, Index) {
  List l({"a", "b", "c"});
  EXPECT_EQ(l.Index(0), "a");
  EXPECT_EQ(l.Index(-1), "c");
}

TEST(TypeList, IndexOutOfRange) {
  List l({"a"});
  EXPECT_THROW(l.Index(5), std::runtime_error);
}

TEST(TypeList, Set) {
  List l({"a", "b", "c"});
  l.Set(1, "X");
  EXPECT_EQ(l.Index(1), "X");
}

TEST(TypeList, InsertBefore) {
  List l({"a", "b", "c"});
  bool ok = l.Insert("b", "X", true);
  EXPECT_TRUE(ok);
  EXPECT_EQ(l.Index(1), "X");
  EXPECT_EQ(l.Index(2), "b");
}

TEST(TypeList, InsertAfter) {
  List l({"a", "b", "c"});
  bool ok = l.Insert("b", "X", false);
  EXPECT_TRUE(ok);
  EXPECT_EQ(l.Index(2), "X");
}

TEST(TypeList, InsertMissingPivot) {
  List l({"a", "b"});
  bool ok = l.Insert("z", "X", true);
  EXPECT_FALSE(ok);
  EXPECT_EQ(l.Size(), 2u);
}

TEST(TypeSet, Add) {
  Set s;
  s.Add({"a", "b", "c"});
  EXPECT_EQ(s.Size(), 3u);
}

TEST(TypeSet, AddDuplicates) {
  Set s({"a", "b"});
  s.Add({"b", "c"});
  EXPECT_EQ(s.Size(), 3u);
}

TEST(TypeSet, Remove) {
  Set s({"a", "b", "c"});
  s.Remove({"b"});
  EXPECT_EQ(s.Size(), 2u);
  EXPECT_FALSE(s.IsMember("b"));
}

TEST(TypeSet, IsMember) {
  Set s({"a", "b"});
  EXPECT_TRUE(s.IsMember("a"));
  EXPECT_FALSE(s.IsMember("z"));
}

TEST(TypeSet, Intersect) {
  Set s1({"a", "b", "c"});
  Set s2({"b", "c", "d"});
  Set result = s1.Intersect(s2);
  EXPECT_EQ(result.Size(), 2u);
  EXPECT_TRUE(result.IsMember("b"));
  EXPECT_TRUE(result.IsMember("c"));
}

TEST(TypeSet, Diff) {
  Set s1({"a", "b", "c"});
  Set s2({"b", "c"});
  Set result = s1.Diff(s2);
  EXPECT_EQ(result.Size(), 1u);
  EXPECT_TRUE(result.IsMember("a"));
}

TEST(TypeSet, GetString) {
  Set s({"a"});
  std::string str = s.GetString();
  EXPECT_NE(str.find("a"), std::string::npos);
}

TEST(TypeGeoIndex, AddAndPos) {
  GeoIndex g;
  g.Add(13.361389, 38.115556, "Palermo");
  auto pos = g.Pos("Palermo");
  ASSERT_TRUE(pos.has_value());
  EXPECT_NEAR(pos->lon, 13.361389, 1e-4);
  EXPECT_NEAR(pos->lat, 38.115556, 1e-4);
}

TEST(TypeGeoIndex, PosNotFound) {
  GeoIndex g;
  auto pos = g.Pos("nobody");
  EXPECT_FALSE(pos.has_value());
}

TEST(TypeGeoIndex, Dist) {
  GeoIndex g;
  g.Add(13.361389, 38.115556, "Palermo");
  g.Add(15.087269, 37.502669, "Catania");
  auto d = g.Dist("Palermo", "Catania");
  ASSERT_TRUE(d.has_value());
  EXPECT_NEAR(*d, 166274.0, 1000.0);
}

TEST(TypeGeoIndex, DistUnknownMember) {
  GeoIndex g;
  g.Add(13.361389, 38.115556, "Palermo");
  auto d = g.Dist("Palermo", "nobody");
  EXPECT_FALSE(d.has_value());
}

TEST(TypeGeoIndex, Search) {
  GeoIndex g;
  g.Add(13.361389, 38.115556, "Palermo");
  g.Add(15.087269, 37.502669, "Catania");
  auto results = g.Search(13.361389, 38.115556, 200000.0, true, 0);
  EXPECT_EQ(results.size(), 2u);
  EXPECT_EQ(results[0].member, "Palermo");
}

TEST(TypeGeoIndex, SearchCount) {
  GeoIndex g;
  g.Add(13.361389, 38.115556, "Palermo");
  g.Add(15.087269, 37.502669, "Catania");
  auto results = g.Search(13.361389, 38.115556, 200000.0, true, 1);
  EXPECT_EQ(results.size(), 1u);
}

TEST(TypeGeoIndex, UnitConversion) {
  double d_m = 1000.0;
  EXPECT_NEAR(GeoIndex::ToMeters(d_m, "km"), 1.0, 1e-6);
  EXPECT_NEAR(GeoIndex::ToMeters(d_m, "m"), 1000.0, 1e-6);
}