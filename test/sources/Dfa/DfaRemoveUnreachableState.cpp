#include "test.h"

TEST(DfaRemoveUnrachableState, TotalSubset) {
  std::vector<Dfa::Terminal> v_terminals = {'a', 'b', 'c', 'd'};
  Dfa::Terminals s_terminals = {'a', 'b', 'c', 'd'};

  for (int i = 0; i < 10000; ++i) {
    auto dfa = FaGenerator::RandomDfa(v_terminals, 5);
    auto min_dfa = Dfa::RemoveUnreachableState(dfa);

    EXPECT_TRUE(IsEquivalentEnumeration(dfa, min_dfa))
              << dfa.ReorderStates().ToString(s_terminals)
              << min_dfa.ReorderStates().ToString(s_terminals)
              << std::endl;
  }
}