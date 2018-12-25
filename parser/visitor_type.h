#pragma once

#include "visitor.h"
#include "symbol_type.h"

using namespace pr;

class CheckType : public Visitor {
 public:
  CheckType(StackTable s);

  void visit(Literal&) override;
  void visit(Variable&) override;
  void visit(BinaryOperation&) override;
  void visit(UnaryOperation&) override;
  void visit(ArrayAccess&) override;
  void visit(RecordAccess&) override;
  void visit(FunctionCall&) override;
  void visit(StaticCast&) override;

  void visit(AssignmentStmt&) override;
  void visit(FunctionCallStmt&) override;
  void visit(BlockStmt&) override;
  void visit(IfStmt&) override;
  void visit(WhileStmt&) override;
  void visit(ForStmt&) override;

  void visit(ForwardFunction&) override;
  void visit(Function&) override;
  void visit(MainFunction&) override;
  void visit(Read&) override;
  void visit(Write&) override;
  void visit(Trunc&) override;
  void visit(Round&) override;
  void visit(Succ&) override;
  void visit(Prev&) override;
  void visit(Chr&) override;
  void visit(Ord&) override;
  void visit(High&) override;
  void visit(Low&) override;


 private:
  StackTable stackTable;

  bool implicitCastInt(BinaryOperation& b);
  bool checkTypePlusMinus(BinaryOperation& b);

};
