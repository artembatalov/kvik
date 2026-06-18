#include "include/kvik/engine/database.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

TEST(DatabaseParser, GoodString) {
  DatabaseParser parser;
  std::string cmd = "SET key value";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"SET", "key", "value"};
  EXPECT_EQ(verify[0], res[0]);
  EXPECT_EQ(verify[1], res[1]);
  EXPECT_EQ(verify[2], res[2]);
}

TEST(DatabaseParser, FirstLetterLowered) {
  DatabaseParser parser;
  std::string cmd = "sET key value";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"SET", "key", "value"};
  EXPECT_EQ(verify[0], res[0]);
  EXPECT_EQ(verify[1], res[1]);
  EXPECT_EQ(verify[2], res[2]);
}

TEST(DatabaseParser, AllLowered) {
  DatabaseParser parser;
  std::string cmd = "set key value";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"SET", "key", "value"};
  EXPECT_EQ(verify[0], res[0]);
  EXPECT_EQ(verify[1], res[1]);
  EXPECT_EQ(verify[2], res[2]);
}

TEST(DatabaseParser, ExitParsed) {
  DatabaseParser parser;
  std::string cmd = "EXIT";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"EXIT"};
  EXPECT_EQ(verify[0], res[0]);
}

TEST(DatabaseParser, DoubleShifts) {
  DatabaseParser parser;
  std::string cmd = "SET  key  value";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"SET", "key", "value"};
  EXPECT_EQ(verify[0], res[0]);
  EXPECT_EQ(verify[1], res[1]);
  EXPECT_EQ(verify[2], res[2]);
}

TEST(DatabaseParser, TrippleShifts) {
  DatabaseParser parser;
  std::string cmd = "SET   key   value";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"SET", "key", "value"};
  EXPECT_EQ(verify[0], res[0]);
  EXPECT_EQ(verify[1], res[1]);
  EXPECT_EQ(verify[2], res[2]);
}

TEST(DatabaseParser, RandomShifts) {
  DatabaseParser parser;
  std::string cmd = " SET key   value ";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"SET", "key", "value"};
  EXPECT_EQ(verify[0], res[0]);
  EXPECT_EQ(verify[1], res[1]);
  EXPECT_EQ(verify[2], res[2]);
}

TEST(DatabaseParser, RandomShiftsAndLoweredCommand) {
  DatabaseParser parser;
  std::string cmd = " set key   value ";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"SET", "key", "value"};
  EXPECT_EQ(verify[0], res[0]);
  EXPECT_EQ(verify[1], res[1]);
  EXPECT_EQ(verify[2], res[2]);
}

TEST(DatabaseParser, EmptyCommand) {
  DatabaseParser parser;
  std::string cmd = "";
  EXPECT_THROW(parser.Parse(cmd), std::runtime_error);
}

TEST(DatabaseParser, OnlyShifts) {
  DatabaseParser parser;
  std::string cmd = "   ";
  EXPECT_THROW(parser.Parse(cmd), std::runtime_error);
}

TEST(DatabaseParser, MemoryUsage) {
  DatabaseParser parser;
  std::string cmd = "MEMORY USAGE key";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"MEMORY USAGE", "key"};
  EXPECT_EQ(res[0], verify[0]);
  EXPECT_EQ(res[1], verify[1]);
}

TEST(DatabaseParser, ConfigGet) {
  DatabaseParser parser;
  std::string cmd = "CONFIG GET maxmemory";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"CONFIG GET", "maxmemory"};
  EXPECT_EQ(res[0], verify[0]);
  EXPECT_EQ(res[1], verify[1]);
}

TEST(DatabaseParser, ConfigSet) {
  DatabaseParser parser;
  std::string cmd = "CONFIG SET maxmemory 120";
  std::vector<std::string> res = parser.Parse(cmd);
  std::vector<std::string> verify = {"CONFIG SET", "maxmemory", "120"};
  EXPECT_EQ(res[0], verify[0]);
  EXPECT_EQ(res[1], verify[1]);
  EXPECT_EQ(res[2], verify[2]);
}