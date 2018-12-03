#pragma once

#include <fstream>

#include "node.h"

namespace pr {


class Visitor {
 public:
  virtual void visit(Variable&) = 0;
  virtual void visit(Literal&) = 0;
  virtual void visit(BinaryOperation&) = 0;
  virtual void visit(UnaryOperation&) = 0;
  virtual void visit(ArrayAccess&) = 0;
  virtual void visit(RecordAccess&) = 0;
  virtual void visit(FunctionCall&) = 0;
  virtual void visit(AssignmentStmt&) = 0;
  virtual void visit(FunctionCallStmt&) = 0;
  virtual void visit(BlockStmt&) = 0;
  virtual void visit(IfStmt&) = 0;
  virtual void visit(WhileStmt&) = 0;
  virtual void visit(ForStmt&) = 0;
  virtual void visit(BreakStmt&) = 0;
  virtual void visit(ContinueStmt&) = 0;
};

class PrintVisitor : public Visitor {
 public:
  explicit PrintVisitor(const std::string& out);

  void visit(Variable&) override;
  void visit(Literal&) override;
  void visit(BinaryOperation&) override;
  void visit(UnaryOperation&) override;
  void visit(ArrayAccess&) override;
  void visit(RecordAccess&) override;
  void visit(FunctionCall&) override;
  void visit(AssignmentStmt&) override;
  void visit(FunctionCallStmt&) override;
  void visit(BlockStmt&) override;
  void visit(IfStmt&) override;
  void visit(WhileStmt&) override;
  void visit(ForStmt&) override;
  void visit(BreakStmt&) override;
  void visit(ContinueStmt&) override;

 private:
  std::ofstream out;
  int depth;
  void print(const std::string&);
};

} // namespace pr