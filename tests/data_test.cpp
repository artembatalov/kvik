#include "include/kvik/engine/database.hpp"
#include "include/kvik/types/list.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <stdexcept>

TEST(DatabaseData, Simple) {
  DatabaseData data(0);
  data.Add<String>(std::string("key"), std::string("value"));
  ASSERT_EQ(data.View<String>("key").GetString(), "value");
}

TEST(DatabaseData, AddAndGet) {
  DatabaseData db;
  db.Add("key", String("hello"));
  EXPECT_EQ(db.View<String>("key").GetString(), "hello");
}

TEST(DatabaseData, GetMissingKeyThrows) {
  DatabaseData db;
  EXPECT_THROW(db.View<String>("missing"), std::runtime_error);
}

TEST(DatabaseData, GetWrongTypeThrows) {
  DatabaseData db;
  db.Add("key", String("hello"));
  EXPECT_THROW(db.View<List>("key"), std::runtime_error);
}

TEST(DatabaseData, DeleteExistingKey) {
  DatabaseData db;
  db.Add("key", String("hello"));
  db.Delete("key");
  EXPECT_FALSE(db.IsKey("key"));
}

TEST(DatabaseData, DeleteMissingKeyNoThrow) {
  DatabaseData db;
  EXPECT_NO_THROW(db.Delete("missing"));
}

TEST(DatabaseData, IsKey) {
  DatabaseData db;
  db.Add("key", String("hello"));
  EXPECT_TRUE(db.IsKey("key"));
  EXPECT_FALSE(db.IsKey("missing"));
}

TEST(DatabaseData, TypeReturnsCorrectType) {
  DatabaseData db;
  db.Add("s", String("hi"));
  db.Add("l", List());
  EXPECT_EQ(db.Type("s"), "string");
  EXPECT_EQ(db.Type("l"), "list");
  EXPECT_EQ(db.Type("missing"), "none");
}

TEST(DatabaseData, ExtractRemovesKey) {
  DatabaseData db;
  db.Add("key", String("hello"));
  String val = db.Extract<String>("key");
  EXPECT_EQ(val.GetString(), "hello");
  EXPECT_FALSE(db.IsKey("key"));
}

TEST(DatabaseData, ExtractMissingKeyThrows) {
  DatabaseData db;
  EXPECT_THROW(db.Extract<String>("missing"), std::runtime_error);
}

TEST(DatabaseData, SizeAndClear) {
  DatabaseData db;
  db.Add("a", String("1"));
  db.Add("b", String("2"));
  EXPECT_EQ(db.Size(), 2);
  db.Clear();
  EXPECT_EQ(db.Size(), 0);
}

TEST(DatabaseData, KeysReturnsAll) {
  DatabaseData db;
  db.Add("a", String("1"));
  db.Add("b", String("2"));
  auto keys = db.Keys();
  EXPECT_EQ(keys.size(), 2);
  EXPECT_TRUE(std::find(keys.begin(), keys.end(), "a") != keys.end());
  EXPECT_TRUE(std::find(keys.begin(), keys.end(), "b") != keys.end());
}

TEST(DatabaseData, OverwriteSameType) {
  DatabaseData db;
  db.Add("key", String("old"));
  db.Add("key", String("new"));
  EXPECT_EQ(db.View<String>("key").GetString(), "new");
  EXPECT_EQ(db.Size(), 1);
}

TEST(DatabaseData, GetTTLNoExpiry) {
  DatabaseData db;
  db.Add("key", String("val"));
  EXPECT_EQ(db.GetTTL("key"), -1);
}

TEST(DatabaseData, GetTTLMissingKey) {
  DatabaseData db;
  EXPECT_EQ(db.GetTTL("missing"), -2);
}

TEST(DatabaseData, SetAndGetTTL) {
  DatabaseData db;
  db.Add("key", String("val"));
  db.SetTTL("key", 60);
  int ttl = db.GetTTL("key");
  EXPECT_GT(ttl, 0);
  EXPECT_LE(ttl, 60);
}

TEST(DatabaseData, CleanExpiredKeepsLiveKeys) {
  DatabaseData db;
  db.Add("live", String("val"));
  db.SetTTL("live", 100);
  db.CleanExpired();
  EXPECT_TRUE(db.IsKey("live"));
}

TEST(DatabaseData, CleanExpiredDoesNothingIfDisabled) {
  DatabaseData db;
  db.Add("key", String("val"));
  db.CleanExpired();
  EXPECT_TRUE(db.IsKey("key"));
}

TEST(DatabaseData, OOMThrowsOnExceedingLimit) {
  DatabaseData db(10);
  EXPECT_THROW(db.Add("key", String("very long string value")),
               std::runtime_error);
}

TEST(DatabaseData, OOMMessageContent) {
  DatabaseData db(10);
  try {
    db.Add("key", String("very long string value"));
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error &e) {
    EXPECT_NE(std::string(e.what()).find("OOM"), std::string::npos);
  }
}

TEST(DatabaseData, OverwriteWithSmallValueNoOOM) {
  DatabaseData db(1024);
  db.Add("key", String("big value here"));
  EXPECT_NO_THROW(db.Add("key", String("x")));
}

TEST(DatabaseData, SetMaxMemoryZeroRemovesLimit) {
  DatabaseData db(10);
  db.SetMaxMemory(0);
  EXPECT_NO_THROW(db.Add("key", String("very long string value")));
}

TEST(DatabaseData, MemoryUsageNonZero) {
  DatabaseData db;
  db.Add("key", String("hello"));
  EXPECT_GT(db.MemoryUsage("key"), 0);
}

TEST(DatabaseData, TotalMemoryNonZero) {
  DatabaseData db;
  db.Add("a", String("hello"));
  db.Add("b", String("world"));
  EXPECT_GT(db.TotalMemory(), 0);
}

TEST(DatabaseData, MemoryUsageMissingKeyZero) {
  DatabaseData db;
  EXPECT_EQ(db.MemoryUsage("missing"), 0);
}

TEST(DatabaseData, MemoryUsageAfterModificationWithoutExpiration) {
  DatabaseData db;
  db.Add<List>("key", List());
  size_t previous_memory = db.TotalMemory();
  {
    auto key = db.Modify<List>("key");
    key->PushRight(std::vector<std::string> {"Bug", "could", "be", "here"});
  }
  EXPECT_NE(previous_memory, db.TotalMemory());
}

TEST(DatabaseData, SimpleTTL) {
  DatabaseData db;
  db.Add<String>("key", String("val"));
  db.EnableTTL();
  db.SetTTL("key", 2);
  sleep(3);
  db.CleanExpired();
  EXPECT_FALSE(db.IsKey("key"));
}

TEST(DatabaseData, ViewExpiresLazilyWithoutCleanExpired) {
  DatabaseData db;
  db.Add<String>("key", String("val"));
  db.EnableTTL();
  db.SetTTL("key", 2);
  sleep(3);
  EXPECT_THROW(db.View<String>("key"), std::runtime_error);
}

TEST(DatabaseData, IsExistExpiresLazilyWithoutCleanExpired) {
  DatabaseData db;
  db.Add<String>("key", String("val"));
  db.EnableTTL();
  db.SetTTL("key", 2);
  sleep(3);
  EXPECT_FALSE(db.IsExist<String>("key"));
}