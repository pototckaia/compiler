#pragma once

#include <memory>
#include <list>

#include "token.h"

class ASTNode;
class Expression;
class ASTNodeStmt;
class Tables;

using ptr_Node = std::shared_ptr<ASTNode>;

using ptr_Token = std::unique_ptr<TokenBase>;
using ptr_Expr = std::unique_ptr<Expression>;
using ptr_Stmt = std::unique_ptr<ASTNodeStmt>;

using ListExpr = std::list<ptr_Expr>;
using ListStmt = std::list<ptr_Stmt>;

class Visitor;

class ASTNode {
 public:
  virtual ~ASTNode() = default;
  virtual void accept(Visitor&) = 0;

  ASTNode() = default;
  ASTNode(int line, int column) : line(line), column(column) {}
  ASTNode(const ptr_Token& t) : line(t->getLine()), column(t->getColumn()) {}

  void setDeclPoint(const ptr_Token& t) {
    line = t->getLine();
    column = t->getColumn();
  }

  int line, column;
};


class Symbol;
class SymVar;
class SymType;
class FunctionSignature;
using ptr_Symbol = std::shared_ptr<Symbol>;
using ptr_Var = std::shared_ptr<SymVar>;
using ptr_Type = std::shared_ptr<SymType>;

class Symbol : public ASTNode {
 public:
  Symbol() : ASTNode(-1, -1) {}
  Symbol(int line, int column) : ASTNode(line, column) {}
  Symbol(const std::string& n) : ASTNode(-1, -1), name(n) {}
  Symbol(const ptr_Token& t)
    :  ASTNode(t), name(t->getValueString()){}

  virtual bool isForward() const { return false; }
  std::string name;
};

class SymFun : public Symbol {
 public:
  using Symbol::Symbol;
  SymFun(const ptr_Token& t, std::shared_ptr<FunctionSignature> f)
    : Symbol(t), signature(std::move(f)) {}

  using ptr_Sign = std::shared_ptr<FunctionSignature>;
  virtual bool isEmbedded() const { return true; }
  ptr_Sign signature;

  virtual void setLabel(const std::string& s) { label = s; }
  virtual std::string getLabel() { return label; }
  std::string label;

  virtual ptr_Sign& getSignature();
  virtual ptr_Stmt& getBody() { throw std::logic_error("get Body"); };
  virtual Tables& getTable() {throw std::logic_error("get Table"); };
};

class SymVar : public Symbol {
 public:
  using Symbol::Symbol;
  SymVar(std::string name, ptr_Type t) : Symbol(std::move(name)), type(std::move(t)) {}
  SymVar(const ptr_Token& n, ptr_Type t)
    : Symbol(n), type(std::move(t)) {}

  virtual uint64_t size() const;
  ptr_Type type;
  ptr_Expr defaultValue;
  virtual void setOffset(uint64_t) = 0;
  virtual uint64_t getOffset() = 0;
};
