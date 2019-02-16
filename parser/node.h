#pragma once

#include <string>
#include <memory>
#include <list>

#include "token.h"
#include "astnode.h"

class Expression : public virtual ASTNode {
 public:
  Expression() = default;
  Expression(ptr_Type t);

  // TODO: replace
	virtual std::string getVarName() { return ""; }

	auto& getType() { return type; }
	void setType(const ptr_Type& type_) { type = type_; }
	auto& getEmbeddedFunction() { return embeddedFunction; }
	void setEmbeddedFunction(const std::shared_ptr<SymFun>& e) { embeddedFunction = e; }

 protected:
	ptr_Type type;

  // TODO: replace
	std::shared_ptr<SymFun> embeddedFunction;
};

class Variable : public Expression {
 public:
  Variable(const Token&);
  Variable(const Token&, ptr_Type);

  // TODO rename
  const auto& getName() const { return name; }
  std::string getVarName() override { return name.getString(); }
  void accept(Visitor&) override;

 private:
  Token name;
};


class Literal : public Expression {
 public:
  Literal(const Token&);
	Literal(const Token& v, ptr_Type t);

	// TODO rename
  const auto& getValue() const { return value; }
  void accept(Visitor&) override;

 private:
  Token value;
};


class BinaryOperation : public Expression {
 public:
  BinaryOperation(const Token&, ptr_Expr, ptr_Expr);

  // TODO rename
  auto& getLeft() { return left; }
  const auto& getRight() const { return right; }
  const auto& getOpr() const { return opr; }

  void setRight(ptr_Expr r) { right = std::move(r); }
	void setLeft(ptr_Expr r) { left = std::move(r); }

  void accept(Visitor&) override ;

 private:
  Token opr;
  ptr_Expr left;
  ptr_Expr right;
};

class UnaryOperation : public Expression {
 public:
  UnaryOperation(const Token&, ptr_Expr);

  // TODO rename
  const auto& getOpr() const { return opr; }
  auto& getExpr() { return expr; }

  void accept(Visitor&) override;

 private:
  Token opr;
  ptr_Expr expr;
};

class ArrayAccess : public Expression {
 public:
  ArrayAccess(const Token&, ptr_Expr, ListExpr);

  // TODO rename
  auto& getName() { return nameArray; }
  const auto& getListIndex() const { return listIndex; };

  void accept(Visitor&) override;

 private:
  ptr_Expr nameArray;
  ListExpr listIndex;
};

class FunctionCall : public Expression {
 public:
  FunctionCall() = default;
  FunctionCall(const Token&, ptr_Expr, ListExpr);

  // TODO replace
  auto& getName() { return nameFunction; }
  // TODO do cast in parser
  ListExpr& getParam() { return listParam; };
  void setParam(const ListExpr& exp) { listParam = exp; }

  void accept(Visitor&) override;

 private:
  ptr_Expr nameFunction;
  ListExpr listParam;
};

class Cast : public Expression {
 public:
  Cast(ptr_Type to, ptr_Expr expr);
  // TODO why this??
  Cast(FunctionCall);

  void accept(Visitor& v) override;
	const auto& getExpr() { return expr; }

 private:
  ptr_Expr expr;
};

class RecordAccess : public Expression {
 public:
  RecordAccess(const Token&, ptr_Expr, Token);

  auto& getRecord() { return record; }
  const auto& getField() const { return field; };

  void accept(Visitor&) override;

 private:
  ptr_Expr record;
  Token field;
};

class ASTNodeStmt : public virtual ASTNode {};

class AssignmentStmt : public ASTNodeStmt, public BinaryOperation {
 public:
  using BinaryOperation::BinaryOperation;
  void accept(Visitor&) override;
};

class FunctionCallStmt : public ASTNodeStmt {
 public:
  FunctionCallStmt(ptr_Expr);

  const auto& getFunctionCall() { return functionCall; }

  void accept(Visitor&) override;

 private:
  ptr_Expr functionCall;
};

class BlockStmt : public ASTNodeStmt {
 public:
  BlockStmt(ListStmt);

  const auto& getBlock() const { return stmts; }

  void accept(Visitor&) override;

 private:
  ListStmt stmts;
};


class IfStmt : public ASTNodeStmt {
 public:
  IfStmt(ptr_Expr, ptr_Stmt);
  IfStmt(ptr_Expr, ptr_Stmt, ptr_Stmt);

  const auto& getCondition() const { return condition; }
  void setCondition(ptr_Expr c) { condition = std::move(c); }
  const auto& getThen() const { return then_stmt; }
  const auto& getElse() const { return else_stmt; }

  void accept(Visitor&) override;

 private:
  ptr_Expr condition;
  ptr_Stmt then_stmt;
  ptr_Stmt else_stmt = nullptr;
};

class LoopStmt : public ASTNodeStmt {};

class WhileStmt : public LoopStmt {
 public:
  WhileStmt(ptr_Expr, ptr_Stmt);

  const auto& getCondition() const { return condition; }
	void setCondition(ptr_Expr c) { condition = std::move(c); }
  const auto& getBlock() const { return block; }

  void accept(Visitor&) override;

 private:
  ptr_Expr condition;
  ptr_Stmt block;
};

class ForStmt : public LoopStmt {
 public:
  ForStmt(std::unique_ptr<Variable>,
          ptr_Expr, ptr_Expr, bool,
          ptr_Stmt);

  const auto& getVar() const { return var; }
  const auto& getLow() const { return low; }
  const auto& getHigh() const { return high; }
  bool getDirect() const { return direct; }
  const auto& getBlock() const { return block; }

  void accept(Visitor&) override;

 private:
  std::unique_ptr<Variable> var;
  ptr_Stmt block;
  ptr_Expr low;
  ptr_Expr high;
  bool direct;
};

class BreakStmt : public ASTNodeStmt {
 public:
  BreakStmt() = default;

  void accept(Visitor&) override;
};

class ContinueStmt : public ASTNodeStmt {
 public:
  ContinueStmt() = default;

  void accept(Visitor&) override;
};
