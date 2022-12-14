#pragma once

#include "AutomatonInclude.hpp"

namespace automaton {

using UnweightedFaGraph = std::unordered_map<StateId, UnorderedStates>;

[[nodiscard]] UnweightedFaGraph ReverseFaGraph(const UnweightedFaGraph &graph) {
  UnweightedFaGraph res;
  for (const auto &[u, vs] : graph) {
    for (const auto &v : vs) {
      if (res.find(v) == res.end()) res[v] = {};
      res[v].insert(u);
    }
  }
  return res;
}

/**
 * state in s will always be in result, because graph may only stores edges.
 * @tparam StatesContainer States, UnorderedStates
 * @tparam VContainer States, UnorderedStates
 */
template<typename StatesContainer = States, typename VContainer = UnorderedStates>
[[nodiscard]] VContainer GetReachable(const UnweightedFaGraph &graph, const StatesContainer &s) {
  VContainer res;
  std::queue<StateId> q;

  for (const auto &state_id : s) {
    q.push(state_id);
    res.insert(state_id);
  }

  while (!q.empty()) {
    StateId cur = q.front();
    q.pop();

    if (graph.find(cur) == graph.end()) { continue; }
    auto &vs = graph.find(cur)->second;

    for (const auto &v : vs) {
      if (res.find(v) == res.end()) {
        res.insert(v);
        q.push(v);
      }
    }
  }

  return res;
}
}