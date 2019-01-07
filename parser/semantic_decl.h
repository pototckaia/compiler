#pragma once

#include <memory>
#include "astnode.h"
#include "token.h"
#include "table_symbol.h"
#include "symbol_type.h"
#include "symbol_var.h"
#include "symbol_fun.h"

class SemanticDecl {
 public:
  SemanticDecl();

  ptr_Expr parseFunctionCall(const tok::ptr_Token&, ptr_Expr, ListExpr);

  ptr_Type parseSimpleType(tok::ptr_Token t);
  ptr_Type parseArrayType(tok::ptr_Token, StaticArray::BoundsType, ptr_Type);
  ptr_Type parseRecordType(tok::ptr_Token declPoint,
                           std::list<std::pair<std::unique_ptr<tok::ListToken>,
                           ptr_Type>> listVar);
  ptr_Type parsePointer(tok::ptr_Token declPoint, tok::ptr_Token, bool isCanForwardType);
  ptr_Type parseOpenArray(tok::ptr_Token declPoint, ptr_Type);

  std::shared_ptr<FunctionSignature>
  parseFunctionSignature(int line, int column, ListParam, ptr_Type returnType);

  ListParam parseFormalParamSection(TableSymbol<ptr_Var>&, ParamSpec, tok::ListToken, ptr_Type);

  void parseTypeDecl(ptr_Token, ptr_Type);
  void parseTypeDeclEnd();
  void parseConstDecl(const tok::ptr_Token& decl, ptr_Expr);

  void parseVariableDecl(tok::ListToken, ptr_Type, bool isGlobal);
  void parseVariableDecl(tok::ListToken id, ptr_Type, ptr_Expr, bool isGlobal);

  void parseFunctionForward(const tok::ptr_Token& decl, std::shared_ptr<FunctionSignature>);
  void parseFunctionDeclBegin(std::shared_ptr<FunctionSignature>);
  void parseFunctionDeclEnd(const tok::ptr_Token& decl, std::shared_ptr<FunctionSignature>, ptr_Stmt);

  std::shared_ptr<MainFunction> parseMainBlock(ptr_Stmt body);

 private:
  StackTable stackTable;
};