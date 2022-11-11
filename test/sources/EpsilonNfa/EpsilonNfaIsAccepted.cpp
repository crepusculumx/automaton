#include "test.h"

TEST(EpsilonNfaIsAccepted, case1) {
  //0*1*2*
  EpsilonNfa::EpsilonNfaTable table = {
      {0, {
          {{'0', {0}}},
          {1}
      }},
      {1, {
          {{'1', {1}}},
          {2}
      }
      },
      {2, {
          {{'2', {2}}},
          {}
      }
      }
  };

  EpsilonNfa epsilon_nfa{table, 0, States{2}};

  EXPECT_TRUE(epsilon_nfa.IsAccepted(""));
  EXPECT_TRUE(epsilon_nfa.IsAccepted("0"));
  EXPECT_TRUE(epsilon_nfa.IsAccepted("012"));
  EXPECT_TRUE(epsilon_nfa.IsAccepted("0012"));
  EXPECT_TRUE(epsilon_nfa.IsAccepted("12"));

  EXPECT_FALSE(epsilon_nfa.IsAccepted("abc"));
  EXPECT_FALSE(epsilon_nfa.IsAccepted("01220"));
}