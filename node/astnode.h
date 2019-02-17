#pragma once

#include <memory>
#include <list>

#include "token.h"

class ASTNode;
class Expression;
class ASTNodeStmt;
class Symbol;
class SymVar;
class SymType;
class FunctionSignature;

class Tables;

using ptr_Node = std::shared_ptr<ASTNode>;
using ptr_Symbol = std::shared_ptr<Symbol>;
using ptr_Var = std::shared_ptr<SymVar>;
using ptr_Type = std::shared_ptr<SymType>;
using ptr_Expr = std::unique_ptr<Expression>;
using ptr_Stmt = std::unique_ptr<ASTNodeStmt>;

using ListExpr = std::list<ptr_Expr>;
using ListStmt = std::list<ptr_Stmt>;

class Visitor;

class ASTNode {
 public:
  ASTNode();
  ASTNode(int line, int column);
  ASTNode(const Token& t);
  virtual ~ASTNode() = default;

  virtual void accept(Visitor&) = 0;

  void setDeclPoint(const Token& t);
  int getDeclLine();
  int getDeclColumn();
  auto& getDeclPoint() { return declPoint; }

 private:
  Token declPoint;
};

class Symbol : public ASTNode {
 public:
	// TODO move to cpp
  Symbol() : ASTNode(-1, -1) {}
  Symbol(int line, int column) : ASTNode(line, column) {}
  Symbol(const std::string& n) : ASTNode(-1, -1), name(n) {}
  Symbol(const Token& t)
    :  ASTNode(t), name(t.getString()){}

    // TODO remove
  virtual bool isForward() const { return false; }

  // todo protected and setter
  std::string name;
};

class SymFun : public Symbol {
 public:
	// todo move to cpp
  using Symbol::Symbol;
  SymFun(const Token& t, std::shared_ptr<FunctionSignature> f)
    : Symbol(t), signature(std::move(f)) {}

  using ptr_Sign = std::shared_ptr<FunctionSignature>;
  // todo remove
  virtual bool isEmbedded() const { return true; }
 // todo protected
  ptr_Sign signature;

  // todo remove
  virtual void setLabel(const std::string& s) { label = s; }
  virtual std::string getLabel() { return label; }
  // todo protected
  std::string label;

  // todo remove
  virtual ptr_Sign& getSignature();
  virtual ptr_Stmt& getBody() { throw std::logic_error("get Body"); };
  virtual Tables& getTable() {throw std::logic_error("get Table"); };
};

class SymVar : public Symbol {
 public:
  using Symbol::Symbol;
  // todo move to cpp
  SymVar(std::string name, ptr_Type t) : Symbol(std::move(name)), type(std::move(t)) {}
  SymVar(const Token& n, ptr_Type t)
    : Symbol(n), type(std::move(t)) {}

    // todo remove
  virtual uint64_t size() const;
  // todo protected
  ptr_Type type;
//  ptr_Expr defaultValue;
  virtual void setOffset(uint64_t) = 0;
  virtual uint64_t getOffset() = 0;
};
