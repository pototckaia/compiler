#pragma once

#include "node.h"
#include "lexerBuffer.h"

namespace pr {

using ptr_Token = std::unique_ptr<tok::TokenBase>;

class Parser {
 public:
  explicit Parser(const std::string&);

  ptr_Expr parse();

 private:
  lx::LexerBuffer lexer;
  // increasing priority for binary operator
  std::array<std::list<tok::TokenType>, 5> priority;
  int priorityLeftUnary;
  int priorityAccess;

  ListExpr parseListExpression();
  ListExpr parseActualParameter();

  ptr_Expr parseFactor();
  ptr_Expr parseAccess(int p);
  ptr_Expr parseUnaryOperator(int p);
  ptr_Expr parseBinaryOperator(int priority = 0);

  ptr_Expr getExprByPriority(int p = 0);

  void require(tok::TokenType);
  void notRequire(tok::TokenType);
  bool require(std::list<tok::TokenType>&);
};

} // namespace pr