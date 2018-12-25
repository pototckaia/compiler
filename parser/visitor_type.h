#pragma once

#include "visitor.h"
#include "symbol_type.h"

using namespace pr;

class CheckLvalue : public Visitor {
 public:
  CheckLvalue() : lvalue(true) {}
  bool isLvalue() { return lvalue; }

  void visit(Literal&) override;
  void visit(Variable&) override;
  void visit(BinaryOperation&) override;
  void visit(UnaryOperation&) override;
  void visit(ArrayAccess&) override;
  void visit(RecordAccess&) override;
  void visit(FunctionCall&) override;
  void visit(StaticCast&) override;

 private:
  bool lvalue;
};

class CheckTypeExpressionBase : public Visitor {
 public:
  CheckTypeExpressionBase(std::string s) : errorMes(std::move(s)) {}

  void visit(Int&) override;
  void visit(Double&) override;
  void visit(Char&) override;
  void visit(TPointer&) override;
  void visit(Boolean&) override;
  void visit(String&) override;

  void visit(Alias&) override;
  void visit(Pointer&) override;
  void visit(StaticArray&) override;
  void visit(OpenArray&) override;
  void visit(Record&) override;
  void visit(FunctionSignature&) override;
  void visit(ForwardType&) override;

 private:
  std::string errorMes;
};

class CheckTypeArrayAccess : public CheckTypeExpressionBase {
 public:
  CheckTypeArrayAccess(ArrayAccess& a)
  : CheckTypeExpressionBase("Array access to type \"" + a.getName()->typeExpression->name + "\" not valid"),
    arrayAccess(a),
    sizeBounds(a.getListIndex().size()) {}

  void visit(Pointer&) override;
  void visit(StaticArray&) override;
  void visit(OpenArray&) override;

 private:
  ArrayAccess& arrayAccess;
  int sizeBounds;
};

class CheckTypeRecordAccess : public CheckTypeExpressionBase {
 public:
  CheckTypeRecordAccess(RecordAccess& a)
    : CheckTypeExpressionBase("Record access to type \"" + a.getRecord()->typeExpression->name + "\" not valid"),
      recordAccess(a) {}

  void visit(Record&) override;

 private:
  RecordAccess& recordAccess;
};

class CheckTypeFunctionCall : public CheckTypeExpressionBase {
 public:
  CheckTypeFunctionCall(FunctionCall& f)
    : CheckTypeExpressionBase("Except function or procedure but find " + f.getName()->typeExpression->name),
      f(f) {}

  void visit(FunctionSignature& f) override;
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
  void visit(Exit&) override;

 private:
  FunctionCall& f;
};

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



 private:
  StackTable stackTable;

  bool implicitCastInt(BinaryOperation& b);
  bool checkTypePlusMinus(BinaryOperation& b);

};
