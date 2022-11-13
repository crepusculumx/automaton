#pragma once

#include <cstddef>

#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <string>
#include <sstream>

#include <algorithm>

namespace automaton {
using StateId = size_t;
using States = std::set<StateId>;
using UnorderedStates = std::unordered_set<StateId>;
}