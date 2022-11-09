#include "test.h"

TEST(DfaMinimization, Random) {
  std::vector<Dfa::Terminal> terminals = {'a', 'b', 'c', 'd'};

  for (int i = 0; i < 10000; ++i) {
    auto dfa = FaGenerator::RandomDfa(terminals, 5);
    auto min_dfa = dfa.Minimize();

    EXPECT_TRUE(IsEquivalentEnumeration(dfa, min_dfa))
              << dfa.ToString(terminals)
              << min_dfa.ToString(terminals)
              << std::endl;
  }
}