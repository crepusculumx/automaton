#pragma once

#include "AutomatonInclude.hpp"
#include "FaGraph.hpp"

#ifdef GTEST
#define protected public
#define private public
#endif  // GTEST

namespace automaton {

class Dfa {
 public:
  using Terminal = char;
  using Terminals = std::set<Terminal>;
  using UnorderedTerminals = std::unordered_set<Terminal>;

  using TransTable = std::unordered_map<Terminal, StateId>;
  using DfaTable = std::unordered_map<StateId, TransTable>;

 private:
  DfaTable table_;
  StateId s_;
  States f_;

  Terminals terminals_{GetTerminals(table_)};
  States states_{GetStates(table_, s_, f_)};

 public:
  template<typename DfaTableType, typename StatesType>
  Dfa(DfaTableType &&dfa_table, StateId s, StatesType &&t):
      table_(std::forward<DfaTableType>(dfa_table)),
      s_(s),
      f_(std::forward<StatesType>(t)) {}

  [[nodiscard]] std::string ToString() const {
    return ToString(terminals_);
  }

  template<typename TerminalsType>
  [[nodiscard]] std::string ToString(const TerminalsType &terminals) const {
    std::stringstream string_stream;

    // head
    string_stream << "\t\t"; // 6 blank
    for (const auto &terminal : terminals) {
      string_stream << "\t" << terminal;
    }
    string_stream << std::endl;

    // for each line
    for (const auto &state_id : states_) {


      // is start and final
      if (state_id == s_) { string_stream << "(s)"; }
      string_stream << "\t";
      if (f_.find(state_id) != f_.end()) { string_stream << "(e)"; }
      string_stream << "\t";
      // state id
      string_stream << "q" << state_id;

      // for each terminal
      for (const auto &terminal : terminals) {
        string_stream << "\t";

        if (table_.find(state_id) != table_.end()
            && table_.find(state_id)->second.find(terminal) != table_.find(state_id)->second.end()) {
          const TransTable &trans_table = table_.find(state_id)->second;
          string_stream << "q" << trans_table.find(terminal)->second;
        } else {
          string_stream << "#";
        }
      }

      string_stream << std::endl;
    }

    return string_stream.str();
  }

  [[nodiscard]] size_t StateSize() const {
    return table_.size();
  }

  [[nodiscard]] const States &GetStates() const {
    return states_;
  }

  [[nodiscard]] const Terminals &GetTerminals() const {
    return terminals_;
  }

  template<typename Container>
  [[nodiscard]] bool IsAccepted(const Container &container) const {
    return IsAccepted(container.cbegin(), container.cend());
  }

  template<typename ForwardIterator>
  [[nodiscard]] bool IsAccepted(ForwardIterator fist, ForwardIterator last) const {
    StateId cur_state = s_;
    for (auto it = fist; it != last; it++) {
      if (table_.find(cur_state) == table_.end()) return false;
      const auto &trans_table = table_.find(cur_state)->second;

      if (trans_table.find(*it) == trans_table.end()) return false;
      cur_state = trans_table.find(*it)->second;
    }
    return (f_.find(cur_state) != f_.end());
  }

  [[nodiscard]] Dfa Minimize() const {
    return Hopcroft(RemoveUnreachableState(*this));
  }

  /*
   * Rename state id in bfs order.
   */
  [[nodiscard]] Dfa ReorderStates() const {
    std::unordered_map<StateId, StateId> new_id_table; // old id -> new id
    std::unordered_map<StateId, StateId> old_id_table; // new id -> old id
    DfaTable dfa_table;

    new_id_table[s_] = 0;
    old_id_table[0] = s_;

    StateId free_id = 1;
    for (int cur_id = 0; cur_id < free_id; cur_id++) {
      auto old_id = old_id_table[cur_id];
      TransTable new_trans_table;

      if (table_.find(old_id) == table_.end()) continue;

      const TransTable &old_trans_table = table_.find(old_id)->second;
      auto terminals = GetTerminals < UnorderedTerminals > (old_trans_table);

      for (const auto &terminal : terminals) {
        StateId next_id = old_trans_table.find(terminal)->second;
        if (new_id_table.find(next_id) == new_id_table.end()) {
          new_id_table[next_id] = free_id;
          old_id_table[free_id] = next_id;
          ++free_id;
        }
        new_trans_table.insert(std::make_pair(terminal, new_id_table[next_id]));
      }

      dfa_table.insert(std::make_pair(cur_id, new_trans_table));
    }

    States f;
    for (const auto &state_id : f_) {
      f.insert(new_id_table[state_id]);
    }

    return {dfa_table, new_id_table[s_], f};
  }

  std::string ToRG() {
    std::stringstream string_stream;
    using OrderedTransTable = std::map<Terminal, StateId>;
    using OrderedDfaTable = std::map<StateId, OrderedTransTable>;

    auto to_ordered_table = [](const DfaTable &table) {

      auto to_ordered_transTable = [](const TransTable &trans_table) {
        OrderedTransTable res;
        for (const auto &item : trans_table) {
          res.insert(std::make_pair(item.first, item.second));
        }
        return res;
      };

      OrderedDfaTable res;
      for (const auto &[state, trans_table] : table) {
        res.insert(std::make_pair(state, to_ordered_transTable(trans_table)));
      }
      return res;
    };

    OrderedDfaTable table = to_ordered_table(table_);

    for (const auto &[state, trans_table] : table) {
      // form of q0->0q1
      for (const auto &[terminal, next_state] : trans_table) {
        if (table[next_state].empty()) { continue; }
        string_stream << "q" << state << "->" << terminal << "q" << next_state << std::endl;
      }

      // form of q0->0
      for (const auto &[terminal, next_state] : trans_table) {
        if (f_.find(next_state) == f_.end()) { continue; }
        string_stream << "q" << state << "->" << terminal << std::endl;;
      }
    }
    return string_stream.str();
  }

 private:

  template<typename VContainerType = Terminals>
  [[nodiscard]] static VContainerType GetTerminals(const DfaTable &table) {
    VContainerType terminals;
    for (const auto &item : table) {
      for (const auto &[terminal, _] : item.second) {
        terminals.emplace(terminal);
      }
    }
    return terminals;
  }

  /**
   * get terminals states used in table.
   * @tparam StatesType states type. e.g. States, UnorderedStates
   * @tparam VContainerType terminals type. e.g. Terminals, UnorderedTerminals
   * @param table
   * @param states
   * @return
   */
  template<typename StatesType, typename VContainerType = Terminals>
  [[nodiscard]] static VContainerType GetTerminals(const DfaTable &table, const StatesType &states) {
    VContainerType terminals;
    for (const auto &state_id : states) {
      if (table.find(state_id) == table.end()) { continue; }
      for (const auto &[terminal, _] : table.find(state_id)->second) {
        terminals.emplace(terminal);
      }
    }
    return terminals;
  }

  template<typename VContainerType = Terminals>
  [[nodiscard]] static VContainerType GetTerminals(const TransTable &trans_table) {
    VContainerType res;
    for (const auto &[terminal, _] : trans_table) {
      res.emplace(terminal);
    }
    return res;
  }

  [[nodiscard]] static States GetStates(const DfaTable &table, StateId s, const States &f) {
    States states;
    for (const auto &[state_id, trans_table] : table) {
      states.insert(state_id);
      for (const auto &[terminal, next_state_id] : trans_table) {
        states.insert(next_state_id);
      }
    }
    states.insert(s);
    states.insert(f.begin(), f.end());
    return states;
  }

//  [[nodiscard]] static

  [[nodiscard]] static Dfa RemoveUnreachableState(const Dfa &dfa) {
    auto to_graph = [](const DfaTable &dfa_table) -> UnweightedFaGraph {
      UnweightedFaGraph res;
      for (const auto &[state_id, trans_table] : dfa_table) {
        for (const auto &[terminal, next_state_id] : trans_table) {
          if (res.find(state_id) == res.end()) { res[state_id] = {}; }
          res[state_id].insert(next_state_id);
        }
      }
      return res;
    };

    auto graph = to_graph(dfa.table_);
    auto rev_graph = ReverseFaGraph(graph);

    auto s_reachable = GetReachable(graph, {dfa.s_});
    auto f_reachable = GetReachable(rev_graph, dfa.f_);

    auto unordered_set_intersection = [](const auto a, const auto b) {
      auto unordered_set_intersection = [](const auto &less, const auto &great) {
        UnorderedStates res;
        for (const auto &item : less) {
          if (great.find(item) == great.end())continue;
          res.insert(item);
        }
        return res;
      };
      return a.size() < a.size()
             ? unordered_set_intersection(a, b)
             : unordered_set_intersection(b, a);
    };

    UnorderedStates reachable_states = unordered_set_intersection(s_reachable, f_reachable);

    DfaTable res_dfa_table;
    for (const auto &[state_id, trans_table] : dfa.table_) {
      if (reachable_states.find(state_id) == reachable_states.end())continue;

      TransTable res_trans_table;
      for (const auto &[terminal, next_state_id] : trans_table) {
        if (reachable_states.find(next_state_id) == reachable_states.end())continue;
        res_trans_table.insert(std::make_pair(terminal, next_state_id));
      }
      res_dfa_table.insert(std::make_pair(state_id, res_trans_table));
    }

    // for s, it may not be alive, but it must exist, so use s anyway.

    States f;
    for (const auto &state_id : dfa.f_) {
      if (reachable_states.find(state_id) == reachable_states.end())continue;
      f.insert(state_id);
    }

    return {res_dfa_table, dfa.s_, f};
  }

  using SplitId = StateId;
  using Split = States;
  using SplitTable = std::map<StateId, SplitId>;

  [[nodiscard]] static std::map<SplitId, States> SplitOne(
      const DfaTable &dfa_table,
      const SplitTable &split_table,
      const Split &split
  ) {
    // for cur_split split, get terminals they use
    UnorderedTerminals terminals{GetTerminals<Split, UnorderedTerminals>(dfa_table, split)};

    std::map<SplitId, States> new_splits;

    // for each terminal, see if it can make split.
    for (const auto &terminal : terminals) {
      new_splits.clear();

      for (const auto &state_id : split) {

        // cur state may not in table because its out-degree is 0.
        if (dfa_table.find(state_id) == dfa_table.end()) {
          if (new_splits.find(SIZE_MAX) == new_splits.end()) { new_splits[SIZE_MAX] = States(); }
          new_splits[SIZE_MAX].insert(state_id);
          continue;
        }
        const auto &trans_table = dfa_table.find(state_id)->second;

        if (trans_table.find(terminal) == trans_table.end()) {
          // use SIZE_MAX as epsilon state
          if (new_splits.find(SIZE_MAX) == new_splits.end()) { new_splits[SIZE_MAX] = States(); }
          new_splits[SIZE_MAX].insert(state_id);
        } else {
          StateId next_state = trans_table.find(terminal)->second;
          SplitId next_split = split_table.find(next_state)->second;
          if (new_splits.find(next_split) == new_splits.end()) { new_splits[next_split] = States(); }
          new_splits[next_split].insert(state_id);
        }
      }

      // successfully split
      if (new_splits.size() > 1) { break; }
    }
    return new_splits;
  }

  [[nodiscard]] static Dfa Hopcroft(const Dfa &dfa) {
    SplitTable split_table; // for state i, which split its in
    std::set<Split> splits;

    std::queue<Split> work_q;
    std::queue<Split> wait_q;

    SplitId free_split_id = 0; // split id free to use

    // split states into final states and non-final states
    Split final_states, none_final_states;
    for (const auto &state : dfa.states_) {
      dfa.f_.find(state) != dfa.f_.end() ? final_states.insert(state) : none_final_states.insert(state);
    }

    //final states and non-final states may be empty

    auto push_if_not_empty = [&split_table, &free_split_id, &work_q](const auto &states) {
      if (!states.empty()) {
        for (const auto &stateId : states) {
          split_table[stateId] = free_split_id;
        }
        ++free_split_id;
        work_q.push(states);
      }
    };

    push_if_not_empty(final_states);
    push_if_not_empty(none_final_states);

    do {
      bool success_split_flag = false;

      // for each split, try to split more
      while (!work_q.empty()) {
        auto cur_split = std::move(work_q.front());
        work_q.pop();

        // split with size 1 can't be split
        if (cur_split.size() == 1) {
          splits.emplace(std::move(cur_split));
          continue;
        }

        // try to split
        auto new_splits = SplitOne(dfa.table_, split_table, cur_split);

        if (new_splits.size() > 1) {
          success_split_flag = true;
          for (auto &[split_id, states] : new_splits) {
            for (const auto &state_id : states) {
              split_table[state_id] = free_split_id;
            }
            wait_q.emplace(std::move(states));
            free_split_id++;
          }
        } else {
          wait_q.emplace(std::move(cur_split));
        }
      }

      if (!success_split_flag) { break; }
      else { std::swap(work_q, wait_q); }

    } while (true);

    while (!wait_q.empty()) {
      splits.emplace(std::move(wait_q.front()));
      wait_q.pop();
    }

    // creat new min DFA

    DfaTable min_dfa_table;
    StateId min_dfa_s = split_table[dfa.s_];
    States min_dfa_f;

    // construct min_dfa_table
    for (const auto &split : splits) {
      // use the fist state to represent the split.
      // split won't be empty because final states and non-final states may be empty has been checked.
      // split id will be its new state id

      StateId cur_state_id = *(split.begin());
      auto split_id = split_table[cur_state_id];

      if (dfa.table_.find(cur_state_id) == dfa.table_.end()) continue;
      const TransTable &trans_table = dfa.table_.find(cur_state_id)->second;

      TransTable new_trans_table;

      for (const auto &[terminal, state_id] : trans_table) {
        new_trans_table[terminal] = split_table[state_id];
      }
      min_dfa_table.insert(std::make_pair(split_id, std::move(new_trans_table)));
    }

    // construct min_dfa_f
    for (const auto &state_id : dfa.f_) {
      min_dfa_f.insert(split_table[state_id]);
    }

    return {min_dfa_table, min_dfa_s, min_dfa_f};
  }

};

}