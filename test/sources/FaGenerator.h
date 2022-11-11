#pragma once

#include <random>

#include "automaton/Automaton.hpp"

namespace automaton::FaGenerator {

static std::random_device dev;
static std::default_random_engine e(dev());
static std::uniform_int_distribution<int> u{};

template<typename T>
std::vector<T> RandomSubset(std::vector<T> s) {
  std::shuffle(s.begin(), s.end(), e);
  return {s.begin(), s.begin() + u(e) % s.size()};
}

template<typename ArrContainer, typename SetContainer>
SetContainer RandomSubset(ArrContainer s) {
  std::shuffle(s.begin(), s.end(), e);
  SetContainer set_container;
  auto v_size = u(e) % s.size();
  for (int i = 0; i < v_size; i++) {
    set_container.insert(s[v_size]);
  }
  return set_container;
}

/**
 *
 * @return One random element from s. Use default constructor if s is empty.
 */
template<typename T>
T RandomGetOne(const std::vector<T> &s) {
  if (s.empty()) return T{};
  return s[u(e) % s.size()];
}

Dfa RandomDfa(const std::vector<Dfa::Terminal> &terminals, size_t StateSize) {
  using ArrStates = std::vector<StateId>;
  ArrStates states;
  for (size_t i = 0; i < StateSize; i++) {
    states.push_back(i);
  }

  Dfa::DfaTable dfa_table;

  ArrStates u_states = RandomSubset(states);
  for (const auto &state_id : u_states) {
    dfa_table[state_id] = {};
    auto cur_terminals = RandomSubset(terminals);
    for (const auto &terminal : cur_terminals) {
      dfa_table[state_id][terminal] = RandomGetOne(states);
    }
  }

  return {std::move(dfa_table), RandomGetOne(states), RandomSubset<ArrStates, States>(states)};
}

Nfa RandomNfa(const std::vector<Nfa::Terminal> &terminals, size_t StateSize) {
  using ArrStates = std::vector<StateId>;
  ArrStates states;
  for (size_t i = 0; i < StateSize; i++) {
    states.push_back(i);
  }

  Nfa::NfaTable nfa_table;

  ArrStates u_states = RandomSubset(states);
  for (const auto &state_id : u_states) {
    nfa_table[state_id] = {};
    auto cur_terminals = RandomSubset(terminals);
    for (const auto &terminal : cur_terminals) {
      nfa_table[state_id][terminal] = RandomSubset<ArrStates, States>(states);
    }
  }

  return {std::move(nfa_table), RandomGetOne(states), RandomSubset<ArrStates, States>(states)};
}

}