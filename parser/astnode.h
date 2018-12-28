#pragma once

#include <memory>
#include <list>

#include "token.h"

namespace pr {

class ASTNode;
class Expression;
class ASTNodeStmt;

using ptr_Node = std::shared_ptr<pr::ASTNode>;

using ptr_Token = std::unique_ptr<tok::TokenBase>;
using ptr_Expr = std::unique_ptr<pr::Expression>;
using ptr_Stmt = std::unique_ptr<pr::ASTNodeStmt>;

using ListExpr = std::list<ptr_Expr>;
using ListStmt = std::list<ptr_Stmt>;

class Visitor;

class ASTNode {
 public:
  virtual ~ASTNode() = default;
  virtual void accept(Visitor&) = 0;

  ASTNode() = default;
  ASTNode(int line, int column) : line(line), column(column) {}
  ASTNode(const pr::ptr_Token& t) : line(t->getLine()), column(t->getColumn()) {}

  void setDeclPoint(const tok::ptr_Token& t) {
    line = t->getLine();
    column = t->getColumn();
  }

  int line, column;
};

} // pr

class Symbol;
class SymVar;
class SymType;
class FunctionSignature;
using ptr_Symbol = std::shared_ptr<Symbol>;
using ptr_Var = std::shared_ptr<SymVar>;
using ptr_Type = std::shared_ptr<SymType>;

class Symbol : public pr::ASTNode {
 public:
  Symbol() : ASTNode(-1, -1) {}
  Symbol(int line, int column) : ASTNode(line, column) {}
  Symbol(const std::string& n) : ASTNode(-1, -1), name(n) {}
  Symbol(const tok::ptr_Token& t)
    :  ASTNode(t), name(t->getValueString()){}

  virtual bool isForward() const { return false; }
  std::string name;
};

class SymFun : public Symbol {
 public:
  using Symbol::Symbol;
  SymFun(const tok::ptr_Token& t, std::shared_ptr<FunctionSignature> f)
    : Symbol(t), signature(std::move(f)) {}

  virtual bool isEmbedded() const { return true; }
  std::shared_ptr<FunctionSignature> signature;
};

class SymVar : public Symbol {
 public:
  using Symbol::Symbol;
  SymVar(std::string name, ptr_Type t) : Symbol(name), type(std::move(t)) {}
  SymVar(const tok::ptr_Token& n, ptr_Type t)
    : Symbol(n), type(std::move(t)) {}

  ptr_Type type;
  pr::ptr_Expr defaultValue;
};
