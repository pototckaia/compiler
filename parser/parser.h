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

  std::unique_ptr<pr::ASTNode> parseFactor();
  std::unique_ptr<pr::ASTNode> parseTerm();
  std::unique_ptr<pr::ASTNode> parseExpr();
};

} // namespace pr