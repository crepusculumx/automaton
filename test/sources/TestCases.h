#pragma once

#include "automaton/Automaton.hpp"

namespace automaton {

/**
 * Test case.
 * Dfa for valid number.
 * https://leetcode.com/problems/valid-number/
 */
Dfa ValidNumberDfa() {
  Dfa::DfaTable dfa_table;
  for (int i = 0; i < 9; ++i) {
    dfa_table[i] = {};
  }

  auto add_digital = [&dfa_table](StateId u, StateId v) {
    for (char c = '0'; c <= '9'; c++) {
      dfa_table[u][c] = v;
    }
  };

  auto add_operator = [&dfa_table](StateId u, StateId v) {
    dfa_table[u]['+'] = v;
    dfa_table[u]['-'] = v;
  };

  auto add_e = [&dfa_table](StateId u, StateId v) {
    dfa_table[u]['e'] = v;
    dfa_table[u]['E'] = v;
  };

  auto add_space = [&dfa_table](StateId u, StateId v) {
    dfa_table[u][' '] = v;
  };

  auto add_dot = [&dfa_table](StateId u, StateId v) {
    dfa_table[u]['.'] = v;
  };

  add_space(0, 0);
  add_digital(0, 1);
  add_dot(0, 2);
  add_operator(0, 3);

  add_digital(1, 1);
  add_dot(1, 4);
  add_e(1, 5);
  add_space(1, 8);

  add_digital(2, 4);

  add_digital(3, 1);
  add_dot(3, 2);

  add_digital(4, 4);
  add_e(4, 5);
  add_space(4, 8);

  add_digital(5, 7);
  add_operator(5, 6);

  add_digital(6, 7);

  add_digital(7, 7);
  dfa_table[7][' '] = 8;
  add_space(7, 8);

  add_space(8, 8);

  return {dfa_table, 0, States{1, 4, 7, 8}};
}

}



