#pragma once

#include "node.h"
#include "lexerBuffer.h"

namespace pr {

using ptr_Token = std::unique_ptr<tok::TokenBase>;

class Parser {
 public:
  explicit Parser(const std::string&);

  ptr_Node parseProgram();
  ptr_Expr parseExpression();


 private:
  lx::LexerBuffer lexer;
  // increasing priority for operator
  std::array<std::list<tok::TokenType>, 5> priority;
  int priorityLeftUnary;
  int priorityAccess;
  std::list<tok::TokenType> assigment;

  bool isInsideLoop = false;

  ListExpr parseListExpression();
  ListExpr parseActualParameter();

  ptr_Expr parseFactor();
  ptr_Expr parseAccess(int p);
  ptr_Expr parseUnaryOperator(int p);
  ptr_Expr parseBinaryOperator(int priority = 0);

  ptr_Expr getExprByPriority(int p = 0);

  ptr_Stmt parseStatement();
  ptr_Stmt parseCompound();
  ptr_Stmt parseIf();
  ptr_Stmt parseWhile();
  ptr_Stmt parseFor();

  ptr_Stmt parseMainBlock();
//  ptr_Stmt parseMainDecl();
//  ptr_Stmt parseFunctionDecl();


  void require(tok::TokenType);
  void require(const std::list<tok::TokenType>& listType, const std::string&);
  bool match(const std::list<tok::TokenType>& listType);
};

} // namespace pr