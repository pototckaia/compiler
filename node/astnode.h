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
class SymFun;
class FunctionSignature;

class ParamVar;
class ForwardType;
class ForwardFunction;
class Const;

class Tables;

using ptr_Node = std::shared_ptr<ASTNode>;
using ptr_Symbol = std::shared_ptr<Symbol>;
using ptr_Var = std::shared_ptr<SymVar>;
using ptr_Type = std::shared_ptr<SymType>;
using ptr_Fun = std::shared_ptr<SymFun>;
using ptr_Sign = std::shared_ptr<FunctionSignature>;
using ptr_Const = std::shared_ptr<Const>;

using ptr_Expr = std::unique_ptr<Expression>;
using ptr_Stmt = std::unique_ptr<ASTNodeStmt>;

using ListExpr = std::list<ptr_Expr>;
using ListStmt = std::list<ptr_Stmt>;
using ListParam = std::list<std::shared_ptr<ParamVar>>;

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
  using ASTNode::ASTNode;
  Symbol(const std::string& n);
  Symbol(const Token& t);

    // TODO remove
  virtual bool isForward() const { return false; }

  bool isAnonymous() const { return name.empty(); }

  const std::string& getSymbolName() const { return name; }
  void setSymbolName(std::string n) { name = std::move(n); }

 private:
  std::string name;
};

class SymFun : public Symbol {
 public:
  using Symbol::Symbol;
  SymFun(const Token& t, ptr_Sign f);

  // todo remove
  virtual bool isEmbedded() const { return true; }

	// todo remove virtual
	virtual void setLabel(const std::string& s) { label = s; }
	virtual std::string getLabel() { return label; }

	// todo remove virtual
	virtual ptr_Sign& getSignature() { return signature; }
	// todo remove
	virtual ptr_Stmt& getBody() { throw std::logic_error("get Body"); };
	virtual Tables& getTable() {throw std::logic_error("get Table"); };

 protected:
  ptr_Sign signature;
  std::string label;
};

class SymVar : public Symbol {
 public:
  //using Symbol::Symbol;
  SymVar(ptr_Type t);
  SymVar(std::string name, ptr_Type t);
  SymVar(const Token& n, ptr_Type t);

    // todo remove
  virtual uint64_t size() const;
  virtual void setOffset(uint64_t) = 0;
  virtual uint64_t getOffset() = 0;

  auto& getVarType() { return type; }

 protected:
	ptr_Type type;
};
