//
// Created by cheri on 10/29/18.
//

#pragma once

#include <memory>
#include <fstream>
#include <algorithm>
#include <string>

#include "token.h"
#include "state.h"
#include "lexer_exception.h"

class Lexer {
 public:
  Lexer(const std::string&);
  ~Lexer();

  std::unique_ptr<TokenBase> next();
  bool is_end() { return is_end_; }

 private:
  LexerStates states;
  tok::KeywordHelper keywordHelper;
  std::ifstream file;

  void error_handler(int state);
  std::unique_ptr<TokenBase> get_token(int);
  
  bool is_end_;
  int line_, column_;
  std::string str_token;
  char prev_symbol;
};
