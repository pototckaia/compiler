#pragma once

#include "node.h"
#include "lexerBuffer.h"

namespace pr {

class Parser {
 public:
  explicit Parser(const std::string&);

  std::unique_ptr<pr::ASTNode> parse();

 private:
  lx::LexerBuffer lexer;
  // increasing priority
  std::array<std::list<tok::TokenType>, 2> priority;

  std::unique_ptr<pr::ASTNode> parseFactor();
  std::unique_ptr<pr::ASTNode> parseTerm(int priority = 0);
  std::unique_ptr<pr::ASTNode> getTermOrFactor(int p = 0);

  void require(tok::TokenType);
  void requireNotNull();
  bool require(std::list<tok::TokenType>&);
};

} // namespace pr