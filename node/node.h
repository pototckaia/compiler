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

  // todo remove
  virtual std::string getVarName() { return ""; }

  auto& getNodeType() { return type; }
  auto& getEmbeddedFunction() { return embeddedFunction; }
  void setNodeType(ptr_Type t) { type = std::move(t); }
  void setEmbeddedFunction(ptr_Fun e) { embeddedFunction = std::move(e); }

 protected:
  ptr_Type type;
  // todo remove this
  ptr_Fun embeddedFunction;
};

class Variable : public Expression {
 public:
  Variable(const Token&);
  Variable(const Token&, ptr_Type);

  const auto& getSubToken() const { return name; }
  // todo remove virtual
  std::string getVarName() override { return name.getString(); }

  void accept(Visitor&) override;

 private:
  Token name;
};

class Literal : public Expression {
 public:
  Literal(const Token&);
  // todo move to cpp
  Literal(const Token& v, ptr_Type t) : Expression(std::move(t)), value(v) {};

  const auto& getValue() const { return value; }
  void accept(Visitor&) override;

 private:
  Token value;
};


class BinaryOperation : public Expression {
 public:
  BinaryOperation(const Token&, ptr_Expr, ptr_Expr);

  const auto& getLeft() const { return left; }
  const auto& getRight() const { return right; }
  const auto& getOpr() const { return opr; }

  void accept(Visitor&) override ;
// todo private
  Token opr;
  ptr_Expr left;
  ptr_Expr right;
};

class UnaryOperation : public Expression {
 public:
  UnaryOperation(Token, ptr_Expr);

  const auto& getOpr() const { return opr; }
  const auto& getExpr() const { return expr; }

  void accept(Visitor&) override;
// todo private
  Token opr;
  ptr_Expr expr;
};

class ArrayAccess : public Expression {
 public:
  ArrayAccess(const Token& d, ptr_Expr name, ListExpr i);

  const auto& getName() const { return nameArray; }
  const auto& getListIndex() const { return listIndex; };

  void accept(Visitor&) override;
  // todo private
  ptr_Expr nameArray;
  ListExpr listIndex;
};

class FunctionCall : public Expression {
 public:
  FunctionCall() = default;
  FunctionCall(const Token& d, ptr_Expr, ListExpr);

  const auto& getName() const { return nameFunction; }
  const auto& getParam() const { return listParam; };

  void accept(Visitor&) override;
// todo private
  ptr_Expr nameFunction;
  ListExpr listParam;
};

class Cast : public Expression {
 public:
  Cast(ptr_Type to, ptr_Expr expr);
  // todo why this?
  Cast(FunctionCall);
  void accept(Visitor& v) override;
  // todo private
  ptr_Expr expr;
};

class RecordAccess : public Expression {
 public:
  RecordAccess(const Token& d, ptr_Expr, Token);

  auto& getRecord() const { return record; }
  auto& getField() const { return field; };

  void accept(Visitor&) override;
// todo private
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
  const auto& getThen() const { return then_stmt; }
  const auto& getElse() const { return else_stmt; }

  void accept(Visitor&) override;
// todo private
  ptr_Expr condition;
  ptr_Stmt then_stmt;
  ptr_Stmt else_stmt = nullptr;
};

class LoopStmt : public ASTNodeStmt {};

class WhileStmt : public LoopStmt {
 public:
  WhileStmt(ptr_Expr, ptr_Stmt);

  const auto& getCondition() const { return condition; }
  const auto& getBlock() const { return block; }

  void accept(Visitor&) override;
// todo private
  ptr_Expr condition;
  ptr_Stmt block;
};

class ForStmt : public LoopStmt {
 public:
  ForStmt(std::unique_ptr<Variable>,
          ptr_Expr, ptr_Expr, bool,
          ptr_Stmt);

  auto& getVar() const { return var; }
  const auto& getLow() const { return low; }
  const auto& getHigh() const { return high; }
  bool getDirect() const { return direct; }
  const auto& getBlock() const { return block; }

  void accept(Visitor&) override;
// todo private
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
