#pragma once

#include "node.h"
#include "lexerBuffer.h"
#include "table_symbol.h"
#include "symbol_type.h"
#include "symbol_var.h"
#include "symbol_fun.h"
#include "semantic_decl.h"

class Parser {
 public:
  explicit Parser(const std::string&);

  ptr_Node parseProgram();
  ptr_Expr parseExpression();


 private:
  lx::LexerBuffer lexer;
  // increasing priority for operator
  std::array<std::list<TokenType>, 5> priority;
  int priorityLeftUnary;
  int priorityAccess;
  std::list<TokenType> assigment;

  SemanticDecl semanticDecl;

  bool isInsideLoop = false;

  ListExpr parseListExpression();
  ListExpr parseActualParameter();
  ListParam parseFormalParameterList();
  std::list<Token> parseListId();

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

  ptr_Type parseType(bool isTypeDecl = false);
  ptr_Type parseSimpleType();
  ptr_Type parseArrayType();
  ptr_Type parseRecordType();
  ptr_Type parsePointer(bool isTypeDecl = false);
  std::pair<int, int> parseRangeType();
  ptr_Type parseParameterType();

  std::shared_ptr<FunctionSignature> parseFunctionSignature(bool isProcedure = false);
  ListParam parseFormalParamSection(TableSymbol<ptr_Var>&);

  std::shared_ptr<MainFunction> parseMainBlock();
  void parseDecl(bool isMainBlock = false);
  void parseTypeDecl();
  void parseConstDecl();
  void parseVarDecl(bool isMainBlock = false);
  void parseVariableDecl(bool isMainBlock = false);
  void parseFunctionDecl(bool isProcedure = false);


  void require(TokenType);
  void require(const std::list<TokenType>& listType, const std::string&);
  bool match(TokenType);
  bool match(const std::list<TokenType>& listType);
  void requireAndSkip(TokenType);
};