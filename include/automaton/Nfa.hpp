#pragma once

#include "AutomatonInclude.hpp"

#include "Dfa.hpp"

#ifdef GTEST
#define protected public
#define private public
#endif  // GTEST

namespace automaton {
class Nfa {
 public:
  using Terminal = char;
  using Terminals = std::set<Terminal>;
  using UnorderedTerminals = std::unordered_set<Terminal>;

  using TransTable = std::unordered_map<Terminal, States>;
  using NfaTable = std::unordered_map<StateId, TransTable>;

 private:
  NfaTable table_;
  StateId s_;
  States f_;

  Terminals terminals_{GetTerminals(table_)};
  States states_{GetStates(table_, s_, f_)};

 public:
  template<typename NfaTableType, typename StatesType>
  Nfa(NfaTableType &&table, StateId s, StatesType &&f) :
      table_(std::forward<NfaTableType>(table)),
      s_(s),
      f_(std::forward<StatesType>(f)) {}

  size_t StateSize() const {
    return states_.size();
  }

  const Terminals &GetTerminals() const {
    return terminals_;
  }

  template<typename Container>
  bool IsAccepted(const Container &container) {
    return IsAccepted(container.begin(), container.end());
  }

  bool IsAccepted(const char *str) {
    std::string_view string_view{str};
    return IsAccepted(string_view.begin(), string_view.end());
  }

  template<typename ForwardIterator>
  bool IsAccepted(ForwardIterator fist, ForwardIterator last) {
    std::set<std::pair<StateId, ForwardIterator>> memo;
    std::function<bool(StateId cur_state_id, ForwardIterator it)>
        dfs =
        [
            &dfs,
            &table = table_,
            &f = f_,
            &last,
            &memo
        ](StateId cur_state_id, ForwardIterator it) {
          if (memo.find(std::make_pair(cur_state_id, it)) != memo.end()) return false;
          if (it == last) {
            if (f.find(cur_state_id) != f.end()) { return true; }
            return false;
          }
          if (table.find(cur_state_id) == table.end()) {
            memo.insert(std::make_pair(cur_state_id, it));
            return false;
          }
          TransTable &trans_table = table.find(cur_state_id)->second;

          if (trans_table.find(*it) == trans_table.end()) {
            memo.insert(std::make_pair(cur_state_id, it));
            return false;
          }
          States &next_states = trans_table.find(*it)->second;

          for (const auto &next_state_id : next_states) {
            ForwardIterator next_it = it;
            next_it++;
            auto res = dfs(next_state_id, next_it);
            if (res) return true;
          }
          memo.insert(std::make_pair(cur_state_id, it));
          return false;
        };
    return dfs(s_, fist);
  }

  Dfa ToDfa() const {
    return ToDfa(*this);
  }

 private:

  static Terminals GetTerminals(const NfaTable &nfa_table) {
    Terminals res;
    for (const auto &[_, trans_table] : nfa_table) {
      for (auto &[terminal, _1] : trans_table) {
        res.insert(terminal);
      }
    }
    return res;
  }

  static States GetStates(const NfaTable &nfa_table, StateId s, const States &f) {
    States res;
    for (const auto &[state_id, trans_table] : nfa_table) {
      res.insert(state_id);
      for (const auto &[_, states] : trans_table) {
        for (const auto &next_state_id : states) {
          res.insert(next_state_id);
        }
      }
    }

    res.insert(s);
    res.insert(f.begin(), f.end());
    return res;
  }

  static Dfa ToDfa(const Nfa &nfa) {
    std::map<States, TransTable> subset_table; // subset -> {terminal, states}

    std::queue<States> q;
    std::set<States> subsets;

    q.push({nfa.s_});
    subsets.insert({nfa.s_});

    while (!q.empty()) {
      States cur = q.front();
      q.pop();
      if (subset_table.find(cur) == subset_table.end()) {
        subset_table[cur] = TransTable();
      }
      TransTable &cur_trans_table = subset_table[cur];

      for (const auto &state : cur) {
        if (nfa.table_.find(state) == nfa.table_.end()) continue;
        const TransTable &trans_table = nfa.table_.find(state)->second;
        for (auto &[terminal, states] : trans_table) {
          cur_trans_table[terminal].insert(states.begin(), states.end());
        }
      }

      for (const auto &[terminal, states] : cur_trans_table) {
        if (subsets.find(states) == subsets.end()) {
          subsets.insert(states);
          q.push(states);
        }
      }
    }

    std::map<States, StateId> new_ids; // subset states -> new id
    StateId new_id = 0;
    for (const auto &states : subsets) {
      new_ids[states] = new_id++;
    }

    Dfa::DfaTable dfa_table;
    for (const auto &subset : subsets) {
      dfa_table[new_ids[subset]] = {};
      auto &cur_trans_table = dfa_table[new_ids[subset]];

      if (subset_table.find(subset) == subset_table.end()) continue;
      auto trans_table = subset_table[subset];

      for (const auto &[terminal, next_states] : trans_table) {
        cur_trans_table[terminal] = new_ids[next_states];
      }
    }

    States f_states;
    for (const auto &subset : subsets) {
      for (const auto &f_id : nfa.f_) {
        if (subset.find(f_id) != subset.end()) {
          f_states.insert(new_ids[subset]);
          break;
        }
      }
    }

    return Dfa(std::move(dfa_table), new_ids[{nfa.s_}], std::move(f_states));
  }
};

}



