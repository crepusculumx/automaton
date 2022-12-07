#pragma once

#include "gtest/gtest.h"

#include "automaton/Automaton.hpp"

#include "FaGenerator.h"
#include "TestCases.h"

using namespace automaton;


template<typename Automaton1, typename Automaton2>
bool IsEquivalentEnumeration(Automaton1 a1, Automaton2 a2) {
  auto terminals_1 = a1.GetTerminals();
  auto terminals_2 = a2.GetTerminals();
  decltype(terminals_1) terminals;
  terminals.insert(terminals_1.begin(), terminals_1.end());
  terminals.insert(terminals_2.begin(), terminals_2.end());

  auto max_len = std::max(a1.StateSize(), a2.StateSize());

  std::function<bool(size_t len, std::vector<typename Automaton1::Terminal> &str)> dfs =
      [
          &dfs,
          &a1,
          &a2,
          &max_len,
          &terminals
      ](size_t len, std::vector<typename Automaton1::Terminal> &str) {
        if (len > max_len) return true;
        if (a1.IsAccepted(str) ^ a2.IsAccepted(str)) {
          return false;
        }
        for (const auto &terminal : terminals) {
          str.push_back(terminal);
          auto res = dfs(len + 1, str);
          if (!res) return false;
          str.pop_back();
        }
        return true;
      };

  std::vector<typename Automaton1::Terminal> str;
  return dfs(0, str);
}
