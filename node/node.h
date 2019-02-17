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

  auto& getSubToken() { return name; }
  // todo remove virtual
  std::string getVarName() override { return name.getString(); }

  void accept(Visitor&) override;

 private:
  Token name;
};

class Literal : public Expression {
 public:
  Literal(const Token&);
  Literal(const Token& v, ptr_Type t);

  auto& getSubToken() { return value; }

  void accept(Visitor&) override;

 private:
  Token value;
};


class BinaryOperation : public Expression {
 public:
  BinaryOperation(const Token&, ptr_Expr, ptr_Expr);

  auto& getSubLeft() { return left; }
  auto& getSubRight() { return right; }
  auto& getOp() { return op; }

  void setSubRight(ptr_Expr r) { right = std::move(r); }
  void setSubLeft(ptr_Expr l) { left = std::move(l); }

  void accept(Visitor&) override ;

 private:
  Token op;
  ptr_Expr left;
  ptr_Expr right;
};

class UnaryOperation : public Expression {
 public:
  UnaryOperation(const Token&, ptr_Expr);

  auto& getOp() { return op; }
  auto& getSubNode() { return expr; }

  void accept(Visitor&) override;

 private:
  Token op;
  ptr_Expr expr;
};

class ArrayAccess : public Expression {
 public:
  ArrayAccess(const Token&, ptr_Expr, ListExpr);

  auto& getSubNode() { return nameArray; }
  auto& getListIndex() { return listIndex; };

  void accept(Visitor&) override;

 private:
  ptr_Expr nameArray;
  ListExpr listIndex;
};

class FunctionCall : public Expression {
 public:
  FunctionCall() = default;
  FunctionCall(const Token&, ptr_Expr, ListExpr);

  auto& getSubNode() { return nameFunction; }
  auto& getListParam() { return listParam; };
  // todo why this?
  void setListParam(ListExpr e) { listParam = std::move(e); }

  void accept(Visitor&) override;

 private:
  ptr_Expr nameFunction;
  ListExpr listParam;
};

class Cast : public Expression {
 public:
  Cast(ptr_Type to, ptr_Expr expr);
  // todo why this?
  Cast(FunctionCall);

  void accept(Visitor& v) override;

  auto& getSubNode() { return expr; }

 private:
  ptr_Expr expr;
};

class RecordAccess : public Expression {
 public:
  RecordAccess(const Token&, ptr_Expr, Token);

  auto& getSubNode() { return record; }
  auto& getField() { return field; };

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

  auto& getFunctionCall() { return functionCall; }

  void accept(Visitor&) override;

 private:
  ptr_Expr functionCall;
};

class BlockStmt : public ASTNodeStmt {
 public:
  BlockStmt(ListStmt);

  auto& getBlock() { return stmts; }

  void accept(Visitor&) override;

 private:
  ListStmt stmts;
};


class IfStmt : public ASTNodeStmt {
 public:
  IfStmt(ptr_Expr, ptr_Stmt);
  IfStmt(ptr_Expr, ptr_Stmt, ptr_Stmt);

  auto& getCondition() { return condition; }
  auto& getThen() { return then_stmt; }
  auto& getElse() { return else_stmt; }

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

  auto& getCondition() { return condition; }
  auto& getBlock() { return block; }

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

  auto& getVar() { return var; }
  auto& getLow() { return low; }
  auto& getHigh() { return high; }
  bool getDirect() { return direct; }
  auto& getBlock() { return block; }

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
