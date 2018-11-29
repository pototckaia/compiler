#pragma once

#include <string>
#include <memory>
#include <list>

#include "token.h"

namespace pr {

class ASTNode;
class ASTNodeStmt;

using ptr_Expr = std::unique_ptr<pr::ASTNode>;
using ListExpr = std::list<ptr_Expr>;
using ptr_Stmt = std::unique_ptr<pr::ASTNodeStmt>;

class Visitor;

class ASTNode {
 public:
  ASTNode() = default;
  virtual ~ASTNode() = default;

  virtual void accept(Visitor&) = 0;
};


class Variable : public ASTNode {
 public:
  Variable(std::unique_ptr<tok::TokenBase>);

  const auto& getName() const { return name; }

  void accept(Visitor&) override;

 private:
  std::unique_ptr<tok::TokenBase> name;
};


class Literal : public ASTNode {
 public:
  Literal(std::unique_ptr<tok::TokenBase>);

  const auto& getValue() const { return value; }

  void accept(Visitor&) override;

 private:
  std::unique_ptr<tok::TokenBase> value;
};


class BinaryOperation : public ASTNode {
 public:
  BinaryOperation(std::unique_ptr<tok::TokenBase>,
                  std::unique_ptr<ASTNode>,
                  std::unique_ptr<ASTNode>);

  const auto& getLeft() const { return left; }
  const auto& getRight() const { return right; }
  const auto& getOpr() const { return opr; }

  void accept(Visitor&) override ;

 private:
  std::unique_ptr<tok::TokenBase> opr;
  std::unique_ptr<ASTNode> left;
  std::unique_ptr<ASTNode> right;
};

class UnaryOperation : public ASTNode {
 public:
  explicit UnaryOperation(std::unique_ptr<tok::TokenBase>, std::unique_ptr<ASTNode>);

  const auto& getOpr() const { return opr; }
  const auto& getExpr() const { return expr; }

  void accept(Visitor&) override;

 private:
  std::unique_ptr<tok::TokenBase> opr;
  std::unique_ptr<ASTNode> expr;
};

class ArrayAccess : public ASTNode {
 public:
  ArrayAccess(ptr_Expr, ListExpr);

  const auto& getName() const { return nameArray; }
  const auto& getListIndex() const { return listIndex; };

  void accept(Visitor&) override;

 private:
  ptr_Expr nameArray;
  ListExpr listIndex;
};

class FunctionCall : public ASTNode {
 public:
  FunctionCall(ptr_Expr, ListExpr);

  const auto& getName() const { return nameFunction; }
  const auto& getParam() const { return listParam; };

  void accept(Visitor&) override;

 private:
  ptr_Expr nameFunction;
  ListExpr listParam;
};

class RecordAccess : public ASTNode {
 public:
  RecordAccess(ptr_Expr, std::unique_ptr<tok::TokenBase>);

  const auto& getRecord() const { return record; }
  const auto& getField() const { return field; };

  void accept(Visitor&) override;

 private:
  ptr_Expr record;
  std::unique_ptr<tok::TokenBase> field;
};

class ASTNodeStmt : public ASTNode {};

class AssignmentStmt : public ASTNodeStmt, public BinaryOperation {
 public:
  AssignmentStmt(std::unique_ptr<tok::TokenBase>,
                 ptr_Expr left, ptr_Expr right);

  void accept(Visitor&) override;
};

//class BlockStmt : public ASTNodeStmt {
// public:
//
// private:
//  std::list<ptr_Stmt> stmts;
//};
//
//class IfStmt : public ASTNodeStmt {
// public:
//
// private:
//  ptr_Expr condition;
//  ptr_Stmt then_stmt;
//  ptr_Stmt else_stmt;
//};
//
//class WhileStmt : public ASTNodeStmt {
// public:
//
// private:
//  ptr_Expr condition;
//  ptr_Stmt block;
//};
//
//class ForStmt : public ASTNodeStmt {
// public:
//
// private:
//  std::unique_ptr<Variable> var;
//  ptr_Expr low;
//  ptr_Expr high;
//  ptr_Stmt block;
//  bool direct;
//};
//
//class BreakStmt : public ASTNodeStmt {
// public:
// private:
//  ptr_Stmt cycle;
//};
//
//class ContinueStmt : public ASTNodeStmt {
// public:
// private:
//  ptr_Stmt cycle;
//};

} // namespace pr