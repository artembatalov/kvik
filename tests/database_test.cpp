#include "include/kvik/engine/database.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

TEST(DatabaseString, SetGet) {
  Database db;
  db.ProcessCommand("SET key hello");
  EXPECT_EQ(db.ProcessCommand("GET key"), "\"hello\"\n");
}

TEST(DatabaseString, GetMissing) {
  Database db;
  EXPECT_EQ(db.ProcessCommand("GET missing"), "(nil)\n");
}

TEST(DatabaseString, Strlen) {
  Database db;
  db.ProcessCommand("SET key hello");
  EXPECT_EQ(db.ProcessCommand("STRLEN key"), "(integer) 5\n");
}

TEST(DatabaseString, Append) {
  Database db;
  db.ProcessCommand("SET key hello");
  db.ProcessCommand("APPEND key world");
  EXPECT_EQ(db.ProcessCommand("GET key"), "\"helloworld\"\n");
}

TEST(DatabaseString, AppendNew) {
  Database db;
  db.ProcessCommand("APPEND newkey val");
  EXPECT_EQ(db.ProcessCommand("GET newkey"), "\"val\"\n");
}

TEST(DatabaseList, LPush) {
  Database db;
  db.ProcessCommand("LPUSH mylist a");
  db.ProcessCommand("LPUSH mylist b");
  EXPECT_EQ(db.ProcessCommand("LLEN mylist"), "(integer) 2\n");
}

TEST(DatabaseList, RPush) {
  Database db;
  db.ProcessCommand("RPUSH mylist a b c");
  EXPECT_EQ(db.ProcessCommand("LLEN mylist"), "(integer) 3\n");
  EXPECT_EQ(db.ProcessCommand("LINDEX mylist 0"), "\"a\"\n");
  EXPECT_EQ(db.ProcessCommand("LINDEX mylist 2"), "\"c\"\n");
}

TEST(DatabaseList, LLen) {
  Database db;
  EXPECT_EQ(db.ProcessCommand("LLEN missing"), "(integer) 0\n");
}

TEST(DatabaseList, LRange) {
  Database db;
  db.ProcessCommand("RPUSH mylist a b c d");
  std::string r = db.ProcessCommand("LRANGE mylist 0 -1");
  EXPECT_NE(r.find("a"), std::string::npos);
  EXPECT_NE(r.find("d"), std::string::npos);
}

TEST(DatabaseList, LPop) {
  Database db;
  db.ProcessCommand("RPUSH mylist a b c");
  EXPECT_EQ(db.ProcessCommand("LPOP mylist"), "\"a\"\n");
  EXPECT_EQ(db.ProcessCommand("LLEN mylist"), "(integer) 2\n");
}

TEST(DatabaseList, RPop) {
  Database db;
  db.ProcessCommand("RPUSH mylist a b c");
  EXPECT_EQ(db.ProcessCommand("RPOP mylist"), "\"c\"\n");
}

TEST(DatabaseList, LIndex) {
  Database db;
  db.ProcessCommand("RPUSH mylist x y z");
  EXPECT_EQ(db.ProcessCommand("LINDEX mylist 1"), "\"y\"\n");
  EXPECT_EQ(db.ProcessCommand("LINDEX mylist -1"), "\"z\"\n");
}

TEST(DatabaseList, LSet) {
  Database db;
  db.ProcessCommand("RPUSH mylist a b c");
  db.ProcessCommand("LSET mylist 1 X");
  EXPECT_EQ(db.ProcessCommand("LINDEX mylist 1"), "\"X\"\n");
}

TEST(DatabaseList, LInsert) {
  Database db;
  db.ProcessCommand("RPUSH mylist a b c");
  db.ProcessCommand("LINSERT mylist BEFORE b X");
  EXPECT_EQ(db.ProcessCommand("LINDEX mylist 1"), "\"X\"\n");
}

TEST(DatabaseSet, SAdd) {
  Database db;
  EXPECT_EQ(db.ProcessCommand("SADD myset a b c"), "(integer) 3\n");
  EXPECT_EQ(db.ProcessCommand("SADD myset a"), "(integer) 0\n");
}

TEST(DatabaseSet, SRem) {
  Database db;
  db.ProcessCommand("SADD myset a b c");
  EXPECT_EQ(db.ProcessCommand("SREM myset a b"), "(integer) 2\n");
  EXPECT_EQ(db.ProcessCommand("SCARD myset"), "(integer) 1\n");
}

TEST(DatabaseSet, SRemDrainsSetToEmpty) {
  Database db;
  db.ProcessCommand("SADD myset a");
  EXPECT_EQ(db.ProcessCommand("SREM myset a"), "(integer) 1\n");
  EXPECT_EQ(db.ProcessCommand("EXISTS myset"), "(integer) 0\n");
  EXPECT_EQ(db.ProcessCommand("DBSIZE"), "(integer) 0\n");
}

TEST(DatabaseSet, SIsMember) {
  Database db;
  db.ProcessCommand("SADD myset a b");
  EXPECT_EQ(db.ProcessCommand("SISMEMBER myset a"), "(integer) 1\n");
  EXPECT_EQ(db.ProcessCommand("SISMEMBER myset z"), "(integer) 0\n");
}

TEST(DatabaseSet, SMembers) {
  Database db;
  db.ProcessCommand("SADD myset a");
  std::string r = db.ProcessCommand("SMEMBERS myset");
  EXPECT_NE(r.find("a"), std::string::npos);
}

TEST(DatabaseSet, SCard) {
  Database db;
  db.ProcessCommand("SADD myset a b c");
  EXPECT_EQ(db.ProcessCommand("SCARD myset"), "(integer) 3\n");
  EXPECT_EQ(db.ProcessCommand("SCARD missing"), "(integer) 0\n");
}

TEST(DatabaseSet, SUion) {
  Database db;
  db.ProcessCommand("SADD s1 a b");
  db.ProcessCommand("SADD s2 b c");
  std::string r = db.ProcessCommand("SUNION s1 s2");
  EXPECT_NE(r.find("a"), std::string::npos);
  EXPECT_NE(r.find("c"), std::string::npos);
}

TEST(DatabaseSet, SInter) {
  Database db;
  db.ProcessCommand("SADD s1 a b c");
  db.ProcessCommand("SADD s2 b c d");
  std::string r = db.ProcessCommand("SINTER s1 s2");
  EXPECT_NE(r.find("b"), std::string::npos);
  EXPECT_NE(r.find("c"), std::string::npos);
  EXPECT_EQ(r.find("a"), std::string::npos);
}

TEST(DatabaseSet, SDiff) {
  Database db;
  db.ProcessCommand("SADD s1 a b c");
  db.ProcessCommand("SADD s2 b c");
  std::string r = db.ProcessCommand("SDIFF s1 s2");
  EXPECT_NE(r.find("a"), std::string::npos);
  EXPECT_EQ(r.find("b"), std::string::npos);
}

TEST(DatabaseSet, SMove) {
  Database db;
  db.ProcessCommand("SADD s1 a b");
  db.ProcessCommand("SADD s2 c");
  EXPECT_EQ(db.ProcessCommand("SMOVE s1 s2 a"), "(integer) 1\n");
  EXPECT_EQ(db.ProcessCommand("SISMEMBER s1 a"), "(integer) 0\n");
  EXPECT_EQ(db.ProcessCommand("SISMEMBER s2 a"), "(integer) 1\n");
}

TEST(DatabaseSet, SMoveDrainsSourceSet) {
  Database db;
  db.ProcessCommand("SADD s1 a");
  db.ProcessCommand("SADD s2 b");
  EXPECT_EQ(db.ProcessCommand("SMOVE s1 s2 a"), "(integer) 1\n");
  EXPECT_EQ(db.ProcessCommand("EXISTS s1"), "(integer) 0\n");
  EXPECT_EQ(db.ProcessCommand("SISMEMBER s2 a"), "(integer) 1\n");
}

TEST(DatabaseGeneral, Type) {
  Database db;
  db.ProcessCommand("SET k v");
  EXPECT_EQ(db.ProcessCommand("TYPE k"), "string\n");
  db.ProcessCommand("RPUSH l a");
  EXPECT_EQ(db.ProcessCommand("TYPE l"), "list\n");
  EXPECT_EQ(db.ProcessCommand("TYPE missing"), "none\n");
}

TEST(DatabaseGeneral, Del) {
  Database db;
  db.ProcessCommand("SET k v");
  EXPECT_EQ(db.ProcessCommand("DEL k"), "(integer) 1\n");
  EXPECT_EQ(db.ProcessCommand("DEL k"), "(integer) 0\n");
}

TEST(DatabaseGeneral, Exists) {
  Database db;
  db.ProcessCommand("SET k v");
  EXPECT_EQ(db.ProcessCommand("EXISTS k"), "(integer) 1\n");
  EXPECT_EQ(db.ProcessCommand("EXISTS k missing"), "(integer) 1\n");
  EXPECT_EQ(db.ProcessCommand("EXISTS missing"), "(integer) 0\n");
}

TEST(DatabaseGeneral, FlushDB) {
  Database db;
  db.ProcessCommand("SET k v");
  db.ProcessCommand("FLUSHDB");
  EXPECT_EQ(db.ProcessCommand("DBSIZE"), "(integer) 0\n");
}

TEST(DatabaseGeneral, DBSize) {
  Database db;
  EXPECT_EQ(db.ProcessCommand("DBSIZE"), "(integer) 0\n");
  db.ProcessCommand("SET k v");
  EXPECT_EQ(db.ProcessCommand("DBSIZE"), "(integer) 1\n");
}

TEST(DatabaseGeneral, Keys) {
  Database db;
  db.ProcessCommand("SET hello world");
  db.ProcessCommand("SET helo x");
  std::string r = db.ProcessCommand("KEYS *");
  EXPECT_NE(r.find("hello"), std::string::npos);
}

TEST(DatabaseGeneral, ConfigSetGet) {
  Database db;
  db.ProcessCommand("CONFIG SET maxmemory 1mb");
  std::string r = db.ProcessCommand("CONFIG GET maxmemory");
  EXPECT_NE(r.find("maxmemory"), std::string::npos);
}

TEST(DatabaseGeneral, MemoryUsage) {
  Database db;
  db.ProcessCommand("SET k hello");
  std::string r = db.ProcessCommand("MEMORY USAGE k");
  EXPECT_NE(r.find("integer"), std::string::npos);
}

TEST(DatabaseGeneral, TTL) {
  Database db;
  db.ProcessCommand("SET k v");
  EXPECT_EQ(db.ProcessCommand("TTL k"), "(integer) -1\n");
  db.ProcessCommand("EXPIRE k 100");
  std::string ttl = db.ProcessCommand("TTL k");
  EXPECT_NE(ttl.find("integer"), std::string::npos);
}

TEST(DatabaseGeneral, TTLMissing) {
  Database db;
  EXPECT_EQ(db.ProcessCommand("TTL missing"), "(integer) -2\n");
}

TEST(DatabaseGeneral, UnknownCommand) {
  Database db;
  EXPECT_THROW(db.ProcessCommand("UNKNOWNCMD"), std::runtime_error);
}

TEST(DatabaseGeo, GeoAdd) {
  Database db;
  EXPECT_EQ(db.ProcessCommand("GEOADD mygeo 13.361389 38.115556 Palermo"),
            "(integer) 1\n");
  EXPECT_EQ(db.ProcessCommand("GEOADD mygeo 15.087269 37.502669 Catania"),
            "(integer) 1\n");
}

TEST(DatabaseGeo, GeoPos) {
  Database db;
  db.ProcessCommand("GEOADD mygeo 13.361389 38.115556 Palermo");
  std::string r = db.ProcessCommand("GEOPOS mygeo Palermo");
  EXPECT_NE(r.find("13"), std::string::npos);
}

TEST(DatabaseGeo, GeoDist) {
  Database db;
  db.ProcessCommand("GEOADD mygeo 13.361389 38.115556 Palermo");
  db.ProcessCommand("GEOADD mygeo 15.087269 37.502669 Catania");
  std::string r = db.ProcessCommand("GEODIST mygeo Palermo Catania km");
  EXPECT_NE(r.find("166"), std::string::npos);
}

TEST(DatabaseGeo, GeoSearch) {
  Database db;
  db.ProcessCommand("GEOADD mygeo 13.361389 38.115556 Palermo");
  db.ProcessCommand("GEOADD mygeo 15.087269 37.502669 Catania");
  std::string r = db.ProcessCommand(
      "GEOSEARCH mygeo FROMLONLAT 13.361389 38.115556 BYRADIUS 200 km ASC");
  EXPECT_NE(r.find("Palermo"), std::string::npos);
}