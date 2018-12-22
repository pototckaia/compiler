#pragma once

#include "visitor.h"
#include "symbol_type.h"

using namespace pr;

class CheckType : public Visitor {

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
  void visit(StaticCast&) override;

};
