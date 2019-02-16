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

  ptr_Expr parseFunctionCall(const ptr_Token&, ptr_Expr, ListExpr);

  ptr_Type parseSimpleType(ptr_Token t);
  ptr_Type parseArrayType(ptr_Token, StaticArray::BoundsType, ptr_Type);
  ptr_Type parseRecordType(ptr_Token declPoint,
                           std::list<std::pair<std::unique_ptr<ListToken>,
                           ptr_Type>> listVar);
  ptr_Type parsePointer(ptr_Token declPoint, ptr_Token, bool isCanForwardType);
  ptr_Type parseOpenArray(ptr_Token declPoint, ptr_Type);

  std::shared_ptr<FunctionSignature>
  parseFunctionSignature(int line, int column, ListParam, ptr_Type returnType);

  ListParam parseFormalParamSection(TableSymbol<ptr_Var>&, ParamSpec, ListToken, ptr_Type);

  void parseTypeDecl(ptr_Token, ptr_Type);
  void parseTypeDeclEnd();
  void parseConstDecl(const ptr_Token& decl, ptr_Expr);

  void parseVariableDecl(ListToken, ptr_Type, bool isGlobal);
  void parseVariableDecl(ListToken id, ptr_Type, ptr_Expr, bool isGlobal);

  void parseFunctionForward(const ptr_Token& decl, std::shared_ptr<FunctionSignature>);
  void parseFunctionDeclBegin(std::shared_ptr<FunctionSignature>);
  void parseFunctionDeclEnd(const ptr_Token& decl, std::shared_ptr<FunctionSignature>, ptr_Stmt);

  std::shared_ptr<MainFunction> parseMainBlock(ptr_Stmt body);

 private:
  StackTable stackTable;
};