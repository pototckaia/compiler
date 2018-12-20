#pragma once

#include "node.h"
#include "lexerBuffer.h"
#include "symbol.h"

namespace pr {

using ListToken = std::list<ptr_Token>;

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

  StackTable stackTable;

  bool isInsideLoop = false;

  ListExpr parseListExpression();
  ListExpr parseActualParameter();
  ListSymbol parseFormalParameterList();
  ListToken parseListId();

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

  ptr_Symbol parseType(bool isTypeDecl = false);
  ptr_Symbol parseSimpleType();
  ptr_Symbol parseArrayType();
  ptr_Symbol parseRecordType();
  ptr_Symbol parsePointer(bool isTypeDecl = false);
  std::pair<int, int> parseRangeType();
  ptr_Symbol parseParameterType();

  ptr_Symbol parseFunctionSignature(bool isProcedure = false);
  ListSymbol parseFormalParamSection(TableSymbol&);

  ptr_Symbol parseMainBlock();
  void parseDecl(bool isMainBlock = false);
  void parseTypeDecl();
  void parseConstDecl();
  void parseVarDecl();
  void parseVariableDecl();
  void parseFunctionDecl(bool isProcedure = false);


  void require(tok::TokenType);
  void require(const std::list<tok::TokenType>& listType, const std::string&);
  bool match(tok::TokenType);
  bool match(const std::list<tok::TokenType>& listType);
  void requireAndSkip(tok::TokenType);
};

} // namespace pr