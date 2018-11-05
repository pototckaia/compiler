#pragma once

#include <map>
#include <set>

#include "token_type.h"


class LexerStates {
 public:
  LexerStates();
  int move(int, char) const;
  tok::TokenType to_token_type(int state) const;

  bool is_final(int state) const;
  bool is_skip(int state, char c) const;
  bool is_putback(int state) const;
  bool is_eof(int state) const { return eof_ == state; }
  bool is_id_or_key(int state) const {return state == -1; }
  int start_state() { return start_; }

  const int ill_str = -71;
  const int ill_char = -77;
 private:
  const int start_ = 0;
  const int whitespace_ = 33;
  const int eof_ = -38;

  std::map<int, tok::TokenType> final_state_;
  const std::set<int> not_putback_state_;

  const int state_table_[33][128];
  const int size_state_table_ = 33;

  inline void initFinishState();
};
