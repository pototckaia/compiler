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

  std::unique_ptr<Token> next();

  const std::unique_ptr<Token>& get();

  void push_back(std::unique_ptr<Token> token);

  LexerBuffer& operator++();

 private:
  Lexer lexer;
  std::list<std::unique_ptr<Token>> buffer;
};

} // namespace lexer