#pragma once

#include <memory>
#include <list>

#include "token.h"

class ASTNode;
class Expression;
class ASTNodeStmt;
class Tables;

using ptr_Node = std::shared_ptr<ASTNode>;

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
  // TODO move to cpp
  ASTNode(int line, int column) : line(line), column(column) {}
  ASTNode(const Token& t) : line(t.getLine()), column(t.getColumn()) {}

  void setDeclPoint(const Token& t) {
    line = t.getLine();
    column = t.getColumn();
  }

  int getLine(){ return line; }
  int getColumn() { return column; }

 private:
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
  using ASTNode::ASTNode;
  // TODO move to cpp
  Symbol(const std::string& n) : name(n) {}
  Symbol(const std::string& n, int line, int column)
    : ASTNode(line, column), name(n) {}
  Symbol(const Token& t)
    :  ASTNode(t),
       name(t.getString()) {}

  // TODO remove
  virtual bool isForward() const { return false; }

  const auto& getName() { return name; }
  void setName(std::string s) { name = std::move(s); }

 protected:
  std::string name;
};

class SymFun : public Symbol {
 public:
  using Symbol::Symbol;
  using ptr_Sign = std::shared_ptr<FunctionSignature>;

  // TODO move to cpp
  SymFun(const Token& t, ptr_Sign f)
    : Symbol(t), signature(std::move(f)) {}

  // TODO remove
  virtual bool isEmbedded() const { return true; }
  virtual void setLabel(const std::string& s) { label = s; }
  virtual std::string getLabel() { return label; }
  virtual ptr_Sign& getSignature();
  virtual ptr_Stmt& getBody() { throw std::logic_error("get Body"); };
  virtual Tables& getTable() {throw std::logic_error("get Table"); };

 protected:
  ptr_Sign signature;
  std::string label;
};

class SymVar : public Symbol {
 public:
  using Symbol::Symbol;
  // TODO move to cpp
  SymVar(std::string name, ptr_Type t) : Symbol(std::move(name)), type(std::move(t)) {}
  SymVar(const Token& n, ptr_Type t)
    : Symbol(n), type(std::move(t)) {}

  // TODO remove
  virtual uint64_t size() const;
  virtual void setOffset(uint64_t) = 0;
  virtual uint64_t getOffset() = 0;

  const auto& getType() { return type; }

 protected:
  ptr_Type type;
  // TODO do use const default value
  //ptr_Expr defaultValue;
};
