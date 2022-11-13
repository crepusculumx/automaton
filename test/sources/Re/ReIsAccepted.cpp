#include "test.h"

TEST(ReIsAccepted, Add) {
  Re re{"a+b"};

  EXPECT_TRUE(re.IsAccepted("a"));
  EXPECT_TRUE(re.IsAccepted("b"));

  EXPECT_FALSE(re.IsAccepted(""));
  EXPECT_FALSE(re.IsAccepted("aaa"));
  EXPECT_FALSE(re.IsAccepted("ab"));
  EXPECT_FALSE(re.IsAccepted("123"));
}

TEST(ReIsAccepted, Times) {
  Re re{"ab"};

  EXPECT_TRUE(re.IsAccepted("ab"));

  EXPECT_FALSE(re.IsAccepted(""));
  EXPECT_FALSE(re.IsAccepted("a"));
  EXPECT_FALSE(re.IsAccepted("b"));
  EXPECT_FALSE(re.IsAccepted("abb"));
}

TEST(ReIsAccepted, Closure) {
  Re re{"a*"};

  EXPECT_TRUE(re.IsAccepted(""));
  EXPECT_TRUE(re.IsAccepted("a"));
  EXPECT_TRUE(re.IsAccepted("aaa"));

  EXPECT_FALSE(re.IsAccepted("ab"));
  EXPECT_FALSE(re.IsAccepted("b"));
  EXPECT_FALSE(re.IsAccepted("ba"));
  EXPECT_FALSE(re.IsAccepted("123"));
}

TEST(ReIsAccepted, Case1) {
  Re re{"a*b*"};

  EXPECT_TRUE(re.IsAccepted(""));
  EXPECT_TRUE(re.IsAccepted("a"));
  EXPECT_TRUE(re.IsAccepted("aaa"));
  EXPECT_TRUE(re.IsAccepted("ab"));
  EXPECT_TRUE(re.IsAccepted("abb"));
  EXPECT_TRUE(re.IsAccepted("aabb"));

  EXPECT_FALSE(re.IsAccepted("ba"));
  EXPECT_FALSE(re.IsAccepted("bba"));
  EXPECT_FALSE(re.IsAccepted("123"));
}

TEST(ReIsAccepted, Case2) {
  Re re{"(ab)*"};

  EXPECT_TRUE(re.IsAccepted(""));
  EXPECT_TRUE(re.IsAccepted("ab"));
  EXPECT_TRUE(re.IsAccepted("abab"));

  EXPECT_FALSE(re.IsAccepted("ba"));
  EXPECT_FALSE(re.IsAccepted("bba"));
  EXPECT_FALSE(re.IsAccepted("123"));
}

TEST(ReIsAccepted, Case3) {
  Re re{"(a+b)(c+d)"};

  EXPECT_TRUE(re.IsAccepted("ac"));
  EXPECT_TRUE(re.IsAccepted("ad"));
  EXPECT_TRUE(re.IsAccepted("bc"));
  EXPECT_TRUE(re.IsAccepted("bd"));

  EXPECT_FALSE(re.IsAccepted("ba"));
  EXPECT_FALSE(re.IsAccepted("abc"));
  EXPECT_FALSE(re.IsAccepted("123"));
}

TEST(ReIsAccepted, Case4) {
  Re re{"ab+cd"};

  EXPECT_TRUE(re.IsAccepted("ab"));
  EXPECT_TRUE(re.IsAccepted("cd"));

  EXPECT_FALSE(re.IsAccepted("ba"));
  EXPECT_FALSE(re.IsAccepted("abc"));
  EXPECT_FALSE(re.IsAccepted("123"));
}