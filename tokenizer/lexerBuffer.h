#pragma once

#include <memory>
#include <list>
#include <string>

#include "token.h"
#include "lexer.h"

namespace lx {

class LexerBuffer {
 public:
  explicit LexerBuffer(const std::string& fileName);

  Token next();
  const Token& get();
  void push_back(Token token);
  LexerBuffer& operator++();

 private:
  Lexer lexer;
  std::list<Token> buffer;
};

} // namespace lexer