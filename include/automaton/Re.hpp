#pragma once

#include <utility>

#include "AutomatonInclude.hpp"

namespace automaton {

class Re {
  std::string regular_expression_;

  Dfa dfa_{ToEpsilonNfa(regular_expression_).ToNfa().ToDfa()};

 public:
  explicit Re(std::string regular_expression) : regular_expression_(std::move(regular_expression)) {}

  bool IsAccepted(std::string_view string_view) {
    return dfa_.IsAccepted(string_view.begin(), string_view.end());
  }

  bool IsAccepted(const char *str) {
    std::string_view string_view{str};
    return dfa_.IsAccepted(string_view.begin(), string_view.end());
  }

  EpsilonNfa ToEpsilonNfa() {
    return ToEpsilonNfa(regular_expression_);
  }

 private:

  static const std::unordered_set<char> operators;

  static bool IsNotOperator(char c) {
    return operators.find(c) == operators.end();
  }

  static std::string AddTimesSign(const std::string &str) {
    if (str.empty()) return str;

    std::string res;
    res.push_back(str[0]);
    for (size_t i = 1; i < str.size(); ++i) {
      if (IsNotOperator(str[i - 1]) && IsNotOperator(str[i]) ||
          IsNotOperator(str[i - 1]) && str[i] == '(' ||
          str[i] == ')' && IsNotOperator(str[i]) ||
          str[i - 1] == ')' && str[i] == '(' ||
          str[i - 1] == '*' && IsNotOperator(str[i]) ||
          str[i - 1] == '*' && str[i] == '('
          ) {
        res.push_back('#');
      }
      res.push_back(str[i]);
    }
    return res;
  }

  static std::string ToPostfixExpression(const std::string &str) {
    auto re = AddTimesSign(str);

    std::stack<char> s;
    std::string res;

    for (const auto &c : re) {
      if (c == '(') { s.push(c); }
      else if (c == ')') {
        while (!s.empty() && s.top() != '(') {
          res.push_back(s.top());
          s.pop();
        }
        // illegal
        if (s.empty() || s.top() != '(') {
          return "";
        }
        s.pop();
      } else if (IsNotOperator(c)) {
        res.push_back(c);
      } else if (c == '*') {
        if (s.empty() || s.top() == '+' || s.top() == '#') { s.push(c); }
        else {
          while ((!s.empty()) && s.top() == '*') {
            res.push_back(s.top());
            s.pop();
          }
          s.push(c);
        }
      } else if (c == '#') {
        if (s.empty() || s.top() == '+') { s.push(c); }
        else {
          while ((!s.empty()) && (s.top() == '*' || s.top() == '#')) {
            res.push_back(s.top());
            s.pop();
          }
          s.push(c);
        }
      } else if (c == '+') {
        if (s.empty()) { s.push(c); }
        else {
          while ((!s.empty()) && (s.top() == '*' || s.top() == '+' || s.top() == '#')) {
            res.push_back(s.top());
            s.pop();
          }
          s.push(c);
        }
      }
    }
    while (!s.empty()) {
      res.push_back(s.top());
      s.pop();
    }
    return res;
  }

  static EpsilonNfa ToEpsilonNfa(const std::string &str) {
    auto pe = ToPostfixExpression(str);
    EpsilonNfa::EpsilonNfaTable table;

    using NfaPart = std::pair<StateId, StateId>;
    std::stack<NfaPart> s; // <s, f>

    StateId free_id = 0;
    auto new_id = [&free_id]() { return free_id++; };

    // a
    auto c_nfa = [&table, &new_id](char c) {
      auto new_s = new_id();
      auto new_f = new_id();

      // c -> b
      if (table.find(new_s) == table.end()) { table[new_s] = {}; }
      if (table[new_s].terminal_trans_table.find(c) == table[new_s].terminal_trans_table.end()) {
        table[new_s].terminal_trans_table[c] = {};
      }
      table[new_s].terminal_trans_table[c].insert(new_f);

      return std::make_pair(new_s, new_f);
    };

    // A#B
    auto time_nfa = [&table](NfaPart a, NfaPart b) {
      auto new_s = a.first;
      auto new_f = b.second;

      // a -> b
      if (table.find(a.second) == table.end()) { table[a.second] = {}; }
      table[a.second].epsilon_trans_table.insert(b.first);

      return std::make_pair(new_s, new_f);
    };

    // A+B
    auto add_nfa = [&table, &new_id](NfaPart a, NfaPart b) {
      auto new_s = new_id();
      auto new_f = new_id();

      // s -> a; s -> b
      table[new_s] = {};
      table[new_s].epsilon_trans_table.insert(a.first);
      table[new_s].epsilon_trans_table.insert(b.first);

      // a -> f
      if (table.find(a.second) == table.end()) { table[a.second] = {}; }
      table[a.second].epsilon_trans_table.insert(new_f);

      // b -> f
      if (table.find(b.second) == table.end()) { table[b.second] = {}; }
      table[b.second].epsilon_trans_table.insert(new_f);

      return std::make_pair(new_s, new_f);
    };

    // A*
    auto closure_nfa = [&table, &new_id](NfaPart a) {
      auto new_n = new_id();

      // n -> a
      if (table.find(new_n) == table.end()) { table[new_n] = {}; }
      table[new_n].epsilon_trans_table.insert(a.first);

      // a-> n
      if (table.find(a.second) == table.end()) { table[a.second] = {}; }
      table[a.second].epsilon_trans_table.insert(new_n);

      return std::make_pair(new_n, new_n);
    };

    for (const auto &c : pe) {
      if (IsNotOperator(c)) { s.push(c_nfa(c)); }
      else if (c == '*') {
        auto t = s.top();
        s.pop();
        s.push(closure_nfa(t));
      } else if (c == '#') {
        // top is in the right. 0#1 -> 01#
        auto t2 = s.top();
        s.pop();
        auto t1 = s.top();
        s.pop();
        s.push(time_nfa(t1, t2));
      } else if (c == '+') {
        // top is in the right. 0+1 -> 01+
        auto t2 = s.top();
        s.pop();
        auto t1 = s.top();
        s.pop();
        s.push(add_nfa(t1, t2));
      }
    }
    if (s.size() != 1) throw std::runtime_error("wrong stack size");
    return {table, s.top().first, States{s.top().second}};
  }
};

const std::unordered_set<char> Re::operators = {'*', '+', '#', '(', ')'};
}