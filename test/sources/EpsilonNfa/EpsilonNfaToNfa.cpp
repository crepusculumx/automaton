#include "test.h"

TEST(EpsilonNfaToNfa, Random) {
  std::vector<Dfa::Terminal> terminals = {'a', 'b', 'c', 'd'};

  for (int i = 0; i < 1000; ++i) {
    auto epsilon_nfa = FaGenerator::RandomEpsilonNfa(terminals, 5);
    auto nfa = epsilon_nfa.ToNfa();
    if (!IsEquivalentEnumeration(epsilon_nfa, nfa)) {

    }
    EXPECT_TRUE(IsEquivalentEnumeration(epsilon_nfa, nfa)) << std::endl;
  }
}
