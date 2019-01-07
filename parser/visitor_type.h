#pragma once

#include "visitor.h"
#include "symbol_type.h"

using namespace pr;

class LvalueChecker : public Visitor {
 public:
  static bool is(ptr_Expr&);
  LvalueChecker() : lvalue(true) {}
  bool isLvalue() { return lvalue; }

  void visit(Literal&) override;
  void visit(Variable&) override;
  void visit(BinaryOperation&) override;
  void visit(UnaryOperation&) override;
  void visit(ArrayAccess&) override;
  void visit(RecordAccess&) override;
  void visit(FunctionCall&) override;
  void visit(Cast&) override;

 private:
  bool lvalue;
};

class BaseTypeChecker : public Visitor {
 public:
  BaseTypeChecker(std::string s) : errorMes(std::move(s)) {}

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

class ArrayAccessChecker : public BaseTypeChecker {
 public:
  static void make(ArrayAccess&, ptr_Type&);

  ArrayAccessChecker(ArrayAccess& a)
  : BaseTypeChecker(tok::getPoint(a.line, a.column) +
                    "Array access to type \"" + a.getName()->type->name + "\" not valid"),
    arrayAccess(a),
    sizeBounds(a.getListIndex().size()) {}

  void visit(Pointer&) override;
  void visit(StaticArray&) override;
  void visit(OpenArray&) override;

 private:
  ArrayAccess& arrayAccess;
  int sizeBounds;
};

class RecordAccessChecker : public BaseTypeChecker {
 public:
  static void make(RecordAccess&, ptr_Type&);

  RecordAccessChecker(RecordAccess& a)
    : BaseTypeChecker(tok::getPoint(a.line, a.column) +
                      "Record access to type \"" + a.getRecord()->type->name + "\" not valid"),
      recordAccess(a) {}

  void visit(Record&) override;

 private:
  RecordAccess& recordAccess;
};

class FunctionCallChecker : public BaseTypeChecker {
 public:
  static void make(FunctionCall&, const ptr_Symbol&);

  FunctionCallChecker(FunctionCall& f)
    : BaseTypeChecker(tok::getPoint(f.line, f.column) + "Expect function or procedure"), f(f) {}

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

class TypeChecker : public Visitor {
 public:
  TypeChecker(StackTable s);

  void visit(Literal&) override;
  void visit(Variable&) override;
  void visit(BinaryOperation&) override;
  void visit(UnaryOperation&) override;
  void visit(ArrayAccess&) override;
  void visit(RecordAccess&) override;
  void visit(FunctionCall&) override;
  void visit(Cast&) override;

  void visit(AssignmentStmt&) override;
  void visit(FunctionCallStmt&) override;
  void visit(BlockStmt&) override;
  void visit(IfStmt&) override;
  void visit(WhileStmt&) override;
  void visit(ForStmt&) override;

 private:
  StackTable stackTable;
  bool isMustFunctionCall = false;
  bool wasFunctionCall = false;

  bool isImplicitType(ptr_Type&, ptr_Type&);
  bool implicitCast(BinaryOperation& b, bool isAssigment);
  bool checkTypePlusMinus(BinaryOperation& b, bool isAssigment = false);
  bool checkTypeSlashAsterisk(BinaryOperation& b, bool isAssigment = false);
};
