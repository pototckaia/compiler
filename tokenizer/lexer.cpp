#include "lexer.h"


Lexer::Lexer(const std::string & filename) : is_end_(false), line_(1), column_(1) {
  file.open(filename, std::ifstream::in);
}

Lexer::~Lexer() {
  file.close();
}

void Lexer::error_handler(int state) {
  if (state == states.ill_char) {
    throw IllegalCharacter(line_, column_, prev_symbol);
  } else if (state == states.ill_str) {
    throw IllegalStringConstant(line_, column_, str_token);
  }
}

std::unique_ptr<TokenBase> Lexer::get_token(int cur_state)  {
  std::unique_ptr<TokenBase> token;
  tok::TokenType t = states.to_token_type(cur_state);

  if (t == tok::TokenType::String) {
    token = std::make_unique<ConstString>(line_, column_, t, str_token);
  } else if (t == tok::TokenType::Double) {
    token = std::make_unique<Double>(line_, column_, t, str_token);
  } else if (t == tok::TokenType::IntBase10 || t == tok::TokenType::IntBase2 ||
             t == tok::TokenType::IntBase16 || t == tok::TokenType::IntBase8) {
    token = std::make_unique<Integer>(line_, column_, t, str_token);
  } else if (states.is_id_or_key(cur_state)) {
    std::string val(str_token);
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    if (keywordHelper.is_keyword(val)) {
      token = std::make_unique<Keyword>(line_, column_, t,
                                        keywordHelper.get_keyword(val), str_token);
    } else {
      token = std::make_unique<OperatorOrId>(line_, column_, t, val, str_token);
    }
  } else if (t == tok::TokenType::Id) {
    std::string val(str_token);
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    token = std::make_unique<OperatorOrId>(line_, column_, t, val, str_token);
  } else {
    token = std::make_unique<OperatorOrId>(line_, column_, t, str_token, str_token);
  }

  return token;
}


std::unique_ptr<TokenBase> Lexer::next() {
  if (is_end_) {
    throw std::out_of_range("конец файла");
  }

  int cur_state = states.start_state();
  str_token = "";
  prev_symbol = '\0';

  int rcount = column_;
  int plcount = rcount;

  char c = file.get();
  while(!states.is_final(cur_state = states.move(cur_state, c))) {
    if (c == '\n') {
      ++line_;
      column_ = rcount = 1;
    }

    if (states.is_skip(cur_state, c)) {
      cur_state = states.start_state();
      ++column_;
    } else {
      str_token += c;
    }

    prev_symbol = c;
    c = file.get();
    ++rcount;
  }

  error_handler(cur_state);

  is_end_ = states.is_eof(states.move(states.start_state(), c));

  if (states.is_putback(cur_state)) {
    file.putback(c);
//    --rcount;
//    if (c == '\n') { --line_; }
  } else {
    str_token += c;
  }

  auto token = get_token(cur_state);

  column_ = rcount;

  return token;
}