//
// Created by crepusculumx on 22-11-10.
//

#include "test.h"

TEST(NfaToDfa, Random) {
  std::vector<Dfa::Terminal> terminals = {'a', 'b', 'c', 'd'};

  for (int i = 0; i < 100000; ++i) {
    auto nfa = FaGenerator::RandomNfa(terminals, 5);
    auto dfa = nfa.ToDfa();

    EXPECT_TRUE(IsEquivalentEnumeration(nfa, dfa))
//              << nfa.ToString(terminals)
              << dfa.ToString(terminals)
              << std::endl;
  }
}