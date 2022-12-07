#pragma once

#include "AutomatonInclude.hpp"
#include "Nfa.hpp"

#ifdef GTEST
#define protected public
#define private public
#endif  // GTEST

namespace automaton {

class EpsilonNfa {
 public:
  using Terminal = char;
  using Terminals = std::set<Terminal>;
  using UnorderedTerminals = std::unordered_set<Terminal>;

  using TerminalTransTable = std::unordered_map<Terminal, States>;
  using EpsilonTransTable = std::set<StateId>;

  struct TransTable {
    TerminalTransTable terminal_trans_table;
    EpsilonTransTable epsilon_trans_table;
  };

  using EpsilonNfaTable = std::unordered_map<StateId, TransTable>;
  using ClosureTable = std::unordered_map<StateId, UnorderedStates>;

 private:
  EpsilonNfaTable table_;
  StateId s_;
  States f_;

  Terminals terminals_{GetTerminals(table_)};
  States states_{GetStates(table_, s_, f_)};

  ClosureTable closure_table_{GetClosureTable(table_, states_)};

 public:
  template<typename EpsilonNfaTableType, typename StatesType>
  EpsilonNfa(EpsilonNfaTableType &&table, StateId s, StatesType &&f)
      : table_(std::forward<EpsilonNfaTableType>(table)),
        s_(s),
        f_(std::forward<StatesType>(f)) {}

  Nfa ToNfa() const {
    return ToNfa(*this);
  }

  const Terminals &GetTerminals() const {
    return terminals_;
  }

  const States &GetStates() const {
    return states_;
  }

  size_t StateSize() const {
    return states_.size();
  }

  template<typename Container>
  bool IsAccepted(const Container &container) const {
    return IsAccepted(container.begin(), container.end());
  }

  bool IsAccepted(const char *str) const {
    std::string_view string_view{str};
    return IsAccepted(string_view.begin(), string_view.end());
  }

  template<typename ForwardIterator>
  [[nodiscard]] bool IsAccepted(ForwardIterator fist, ForwardIterator last) const {
    using It = ForwardIterator;
    std::function<bool(StateId cur_states, It it, bool can_epsilon)> dfs = [
        &dfs,
        &last,
        &table = table_,
        &f = f_,
        &closure_table = closure_table_
    ](StateId cur_state, It it, bool can_epsilon) {
      if (it == last) {
        if (f.find(cur_state) != f.end()) { return true; }

        if (closure_table.find(cur_state) == closure_table.end()) { return false; }
        const auto &closure = closure_table.find(cur_state)->second;

        return std::any_of(closure.begin(), closure.end(), [&f](const auto &next_state) {
          return f.find(next_state) != f.end();
        });
      }

      if (table.find(cur_state) == table.end()) { return false; }
      const TransTable &trans_table = table.find(cur_state)->second;

      const auto &terminal_trans_table = trans_table.terminal_trans_table;
      const auto &epsilon_trans_table = trans_table.epsilon_trans_table;

      if (terminal_trans_table.find(*it) != terminal_trans_table.end()) {
        const auto &next_states = terminal_trans_table.find(*it)->second;
        auto next_it = it;
        ++next_it;
        for (const auto &next_state : next_states) {
          auto res = dfs(next_state, next_it, true);
          if (res) { return res; }
        }
      }

      if (can_epsilon) {
        if (closure_table.find(cur_state) == closure_table.end()) { return false; }
        const auto &closure = closure_table.find(cur_state)->second;
        for (const auto &next_state : closure) {
          auto res = dfs(next_state, it, false);
          if (res) { return res; }
        }
      }
      return false;
    };

    return dfs(s_, fist, true);
  }

  static States GetStates(const EpsilonNfaTable &table, StateId s, const States &f) {
    States res;
    for (const auto &[state_id, trans_table] : table) {
      res.insert(state_id);
      for (const auto &[terminal, next_states] : trans_table.terminal_trans_table) {
        res.insert(next_states.begin(), next_states.end());
      }
      for (const auto &next_state : trans_table.epsilon_trans_table) {
        res.insert(next_state);
      }
    }
    res.insert(s);
    res.insert(f.begin(), f.end());
    return res;
  }

  static Terminals GetTerminals(const EpsilonNfaTable &table) {
    Terminals res;
    for (const auto &[state_id, trans_table] : table) {
      for (const auto &[terminal, next_states] : trans_table.terminal_trans_table) {
        res.insert(terminal);
      }
    }
    return res;
  }

  static ClosureTable GetClosureTable(const EpsilonNfaTable &table, const States &states) {
    ClosureTable closure_table;

    auto bfs = [
        &table,
        &closure_table
    ](StateId s) {
      std::queue<StateId> q;
      UnorderedStates res;

      q.push(s);
      res.insert(s);

      while (!q.empty()) {
        auto cur = q.front();
        q.pop();
        if (closure_table.find(cur) != closure_table.end()) {
          const auto &closure = closure_table.find(cur)->second;
          res.insert(closure.begin(), closure.end());
          continue;
        }
        if (table.find(cur) == table.end()) { continue; }
        auto &epsilon_trans_table = table.find(cur)->second.epsilon_trans_table;
        for (const auto &id : epsilon_trans_table) {
          if (res.find(id) == res.end()) {
            q.push(id);
            res.insert(id);
          }
        }
      }

      return res;
    };
    for (const auto &state_id : states) {
      if (closure_table.find(state_id) != closure_table.end()) continue;
      closure_table[state_id] = bfs(state_id);
    }
    return closure_table;
  }

  static Nfa ToNfa(const EpsilonNfa &epsilon_nfa) {
    const auto &table = epsilon_nfa.table_;
    const auto &states = epsilon_nfa.states_;
    const auto &closure_table = epsilon_nfa.closure_table_;

    Nfa::NfaTable nfa_table;

    for (const auto &q : states) {
      nfa_table[q] = {};

      if (closure_table.find(q) == closure_table.end()) continue;
      const auto &closure_q = closure_table.find(q)->second;

      // for p in epsilon-CLOSURE(q)
      for (const auto &p : closure_q) {
        // epsilon -> terminal
        std::map<Terminal, UnorderedStates> temp_states;
        if (table.find(p) == table.end()) { continue; }
        const auto &p_terminal_trans_table = table.find(p)->second.terminal_trans_table;

        for (const auto &[terminal, next_states] : p_terminal_trans_table) {
          if (temp_states.find(terminal) == temp_states.end()) { temp_states[terminal] = {}; }
          temp_states[terminal].insert(next_states.begin(), next_states.end());
        }

        // build nfa table
        for (const auto &[terminal, next_states] : temp_states) {
          if (nfa_table[q].find(terminal) == nfa_table[q].end()) { nfa_table[q][terminal] = {}; }
          nfa_table[q][terminal].insert(next_states.begin(), next_states.end());

          // terminal -> epsilon
          for (const auto &state : next_states) {
            if (closure_table.find(state) == closure_table.end()) { continue; }
            const auto &closure = closure_table.find(state)->second;
            nfa_table[q][terminal].insert(closure.begin(), closure.end());
          }
        }
      }
    }

    States f;

    // only epsilon
    auto to_fa_graph = [](const EpsilonNfa &epsilon_nfa) {
      const auto &states = epsilon_nfa.states_;
      const auto &table = epsilon_nfa.table_;

      UnweightedFaGraph res;

      for (const auto &state : states) {
        res[state] = {};
        if (table.find(state) == table.end()) { continue; }
        auto trans_table = table.find(state)->second;
        res[state].insert(trans_table.epsilon_trans_table.begin(), trans_table.epsilon_trans_table.end());
      }

      return res;
    };

    auto graph = to_fa_graph(epsilon_nfa);
    auto reverse_graph = ReverseFaGraph(graph);

    return {std::move(nfa_table), epsilon_nfa.s_, GetReachable<States, States>(reverse_graph, epsilon_nfa.f_)};
  }

};

}