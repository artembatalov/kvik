#include <gtest/gtest.h>

#include "kvik/utils/glob.hpp"

TEST(GlobMatch, ExactMatchNoWildcards) {
  EXPECT_TRUE(GlobMatch("hello", "hello"));
  EXPECT_FALSE(GlobMatch("hello", "world"));
  EXPECT_FALSE(GlobMatch("hello", "hell"));
}

TEST(GlobMatch, EmptyPatternAndString) {
  EXPECT_TRUE(GlobMatch("", ""));
  EXPECT_FALSE(GlobMatch("", "a"));
  EXPECT_FALSE(GlobMatch("a", ""));
  EXPECT_TRUE(GlobMatch("*", ""));
}

TEST(GlobMatch, StarMatchesAnySequence) {
  EXPECT_TRUE(GlobMatch("*", "anything"));
  EXPECT_TRUE(GlobMatch("user:*", "user:123"));
  EXPECT_TRUE(GlobMatch("user:*", "user:"));
  EXPECT_FALSE(GlobMatch("user:*", "session:123"));
  EXPECT_TRUE(GlobMatch("*:cache", "user:cache"));
  EXPECT_TRUE(GlobMatch("*mid*", "left_mid_right"));
}

TEST(GlobMatch, ConsecutiveStars) {
  EXPECT_TRUE(GlobMatch("**", "anything"));
  EXPECT_TRUE(GlobMatch("a**b", "ab"));
  EXPECT_TRUE(GlobMatch("a**b", "axxxb"));
}

TEST(GlobMatch, StarRequiresBacktrack) {
  EXPECT_TRUE(GlobMatch("a*b*c", "axbxxxc"));
  EXPECT_FALSE(GlobMatch("a*b*c", "axbxxx"));
  EXPECT_TRUE(GlobMatch("*abc", "xxabc"));
  EXPECT_TRUE(GlobMatch("*abc*abc", "abcabcabc"));
}

TEST(GlobMatch, QuestionMarkMatchesExactlyOneChar) {
  EXPECT_TRUE(GlobMatch("h?llo", "hello"));
  EXPECT_TRUE(GlobMatch("h?llo", "hallo"));
  EXPECT_FALSE(GlobMatch("h?llo", "hllo"));
  EXPECT_FALSE(GlobMatch("h?llo", "heello"));
}

TEST(GlobMatch, CharacterClass) {
  EXPECT_TRUE(GlobMatch("[abc]", "a"));
  EXPECT_TRUE(GlobMatch("[abc]", "b"));
  EXPECT_FALSE(GlobMatch("[abc]", "d"));
  EXPECT_TRUE(GlobMatch("key[123]", "key2"));
}

TEST(GlobMatch, CharacterRange) {
  EXPECT_TRUE(GlobMatch("[a-z]", "m"));
  EXPECT_FALSE(GlobMatch("[a-z]", "M"));
  EXPECT_TRUE(GlobMatch("user:[0-9]", "user:7"));
  EXPECT_FALSE(GlobMatch("user:[0-9]", "user:x"));
}

TEST(GlobMatch, NegatedClass) {
  EXPECT_TRUE(GlobMatch("[^abc]", "d"));
  EXPECT_FALSE(GlobMatch("[^abc]", "a"));
  EXPECT_TRUE(GlobMatch("[!abc]", "z"));
}

TEST(GlobMatch, EscapedLiteralCharacters) {
  EXPECT_TRUE(GlobMatch("2plus2\\*4", "2plus2*4"));
  EXPECT_FALSE(GlobMatch("2plus2\\*4", "2plus28884"));
  EXPECT_TRUE(GlobMatch("what\\?", "what?"));
}

TEST(GlobMatch, CaseSensitive) {
  EXPECT_FALSE(GlobMatch("User", "user"));
  EXPECT_TRUE(GlobMatch("User", "User"));
}