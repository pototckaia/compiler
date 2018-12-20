#pragma once

#include <fstream>

#include "node.h"
#include "symbol.h"

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

  virtual void visit(Int&) = 0;
  virtual void visit(Double&) = 0;
  virtual void visit(Char&) = 0;
  virtual void visit(Boolean&) = 0;
  virtual void visit(TPointer&) = 0;

  virtual void visit(Alias&) = 0;
  virtual void visit(Pointer&) = 0;
  virtual void visit(StaticArray&) = 0;
  virtual void visit(OpenArray&) = 0;
  virtual void visit(Record&) = 0;
  virtual void visit(FunctionSignature&) = 0;
  virtual void visit(ForwardType&) = 0;

  virtual void visit(LocalVar&) = 0;
  virtual void visit(GlobalVar&) = 0;
  virtual void visit(ParamVar&) = 0;
  virtual void visit(Const&) = 0;

  virtual void visit(ForwardFunction&) = 0;
  virtual void visit(Function&) = 0;
  virtual void visit(MainFunction&) = 0;

  virtual void visit(TableSymbol&) = 0;
  virtual void visit(Tables&) = 0;
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

  void visit(Int&) override;
  void visit(Double&) override;
  void visit(Char&) override;
  void visit(TPointer&) override;
  void visit(Boolean&) override;

  void visit(Alias&) override;
  void visit(Pointer&) override;
  void visit(StaticArray&) override;
  void visit(OpenArray&) override;
  void visit(Record&) override;
  void visit(FunctionSignature&) override;
  void visit(ForwardType&) override;

  void visit(LocalVar&) override;
  void visit(GlobalVar&) override;
  void visit(ParamVar&) override;
  void visit(Const&) override;

  void visit(ForwardFunction&) override;
  void visit(Function&) override;
  void visit(MainFunction&) override;

  void visit(TableSymbol&) override;
  void visit(Tables&) override;

 private:
  std::ofstream out;
  int depth;
  void print(const std::string&);
};

} // namespace pr