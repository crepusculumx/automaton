#include "test.h"

TEST(ReToPostfixExpression, case1) {
  EXPECT_EQ(Re::ToPostfixExpression("a"), "a");
  EXPECT_EQ(Re::ToPostfixExpression("a+b"), "ab+");
  EXPECT_EQ(Re::ToPostfixExpression("ab"), "ab#");
  EXPECT_EQ(Re::ToPostfixExpression("a*"), "a*");

  EXPECT_EQ(Re::ToPostfixExpression("a*b"), "a*b#");
}