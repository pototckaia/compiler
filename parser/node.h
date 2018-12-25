#pragma once

#include <string>
#include <memory>
#include <list>

#include "token.h"
#include "astnode.h"

namespace pr {

class Expression : public ASTNode {
 public:
  Expression() = default;
  Expression(ptr_Type t) : typeExpression(std::move(t)) {}

  ptr_Type typeExpression;
  std::shared_ptr<SymFun> embeddedFunction;
};

class Variable : public Expression {
 public:
  Variable(std::unique_ptr<tok::TokenBase>);

  const auto& getName() const { return name; }

  void accept(Visitor&) override;

 private:
  ptr_Token name;
};


class Literal : public Expression {
 public:
  Literal(std::unique_ptr<tok::TokenBase>);

  const auto& getValue() const { return value; }

  void accept(Visitor&) override;

 private:
  ptr_Token value;
};


class BinaryOperation : public Expression {
 public:
  BinaryOperation(std::unique_ptr<tok::TokenBase>,
                  ptr_Expr,
                  ptr_Expr);

  const auto& getLeft() const { return left; }
  const auto& getRight() const { return right; }
  const auto& getOpr() const { return opr; }

  void accept(Visitor&) override ;

  ptr_Token opr;
  ptr_Expr left;
  ptr_Expr right;
};

class UnaryOperation : public Expression {
 public:
  UnaryOperation(std::unique_ptr<tok::TokenBase>, ptr_Expr);

  const auto& getOpr() const { return opr; }
  const auto& getExpr() const { return expr; }

  void accept(Visitor&) override;

 private:
  ptr_Token opr;
  ptr_Expr expr;
};

class ArrayAccess : public Expression {
 public:
  ArrayAccess(ptr_Expr, ListExpr);

  const auto& getName() const { return nameArray; }
  const auto& getListIndex() const { return listIndex; };

  void accept(Visitor&) override;

 private:
  ptr_Expr nameArray;
  ListExpr listIndex;
};

class FunctionCall : public Expression {
 public:
  FunctionCall(ptr_Expr, ListExpr);

  const auto& getName() const { return nameFunction; }
  const auto& getParam() const { return listParam; };

  void accept(Visitor&) override;

  ptr_Expr nameFunction;
  ListExpr listParam;
};

class StaticCast : public Expression {
 public:
  StaticCast(ptr_Type to, ptr_Expr expr);
  void accept(pr::Visitor& v) override;
  ptr_Expr expr;
};

class RecordAccess : public Expression {
 public:
  RecordAccess(ptr_Expr, std::unique_ptr<tok::TokenBase>);

  const auto& getRecord() const { return record; }
  const auto& getField() const { return field; };

  void accept(Visitor&) override;

 private:
  ptr_Expr record;
  ptr_Token field;
};

class ASTNodeStmt : public ASTNode {};

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

} // namespace pr