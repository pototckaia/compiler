#pragma once

#include <fstream>

#include "node.h"
#include "table_symbol.h"
#include "symbol_fun.h"
#include "symbol_var.h"
#include "symbol_type.h"

class Visitor {
 public:
  virtual ~Visitor() = default;
  virtual void visit(Variable&) {};
  virtual void visit(Literal&) {};
  virtual void visit(BinaryOperation&)  {};
  virtual void visit(UnaryOperation&)  {};
  virtual void visit(ArrayAccess&) {};
  virtual void visit(RecordAccess&) {};
  virtual void visit(FunctionCall&) {};
  virtual void visit(Cast&) {};

  virtual void visit(AssignmentStmt&) {};
  virtual void visit(FunctionCallStmt&) {};
  virtual void visit(BlockStmt&) {};
  virtual void visit(IfStmt&) {};
  virtual void visit(WhileStmt&) {};
  virtual void visit(ForStmt&) {};
  virtual void visit(BreakStmt&) {};
  virtual void visit(ContinueStmt&) {};

  virtual void visit(Int&) {};
  virtual void visit(Double&) {};
  virtual void visit(Char&) {};
  virtual void visit(Boolean&) {};
  virtual void visit(TPointer&) {};
  virtual void visit(String&) {};
  virtual void visit(Void&) {};

  virtual void visit(Alias&) {};
  virtual void visit(Pointer&) {};
  virtual void visit(StaticArray&) {};
  virtual void visit(OpenArray&) {};
  virtual void visit(Record&) {};
  virtual void visit(FunctionSignature&) {};
  virtual void visit(ForwardType&) {};

  virtual void visit(LocalVar&) {};
  virtual void visit(GlobalVar&) {};
  virtual void visit(ParamVar&) {};
  virtual void visit(Const&) {};

  virtual void visit(ForwardFunction&) {};
  virtual void visit(Function&) {};
  virtual void visit(MainFunction&) {};

  virtual void visit(Read&) {};
  virtual void visit(Write&) {};
  virtual void visit(Trunc&) {};
  virtual void visit(Round&) {};
  virtual void visit(Succ&) {};
  virtual void visit(Prev&) {};
  virtual void visit(Chr&) {};
  virtual void visit(Ord&) {};
  virtual void visit(High&) {};
  virtual void visit(Low&) {};
  virtual void visit(Exit& e) {};

  virtual void visit(TableSymbol<std::shared_ptr<SymType>>&) {};
  virtual void visit(TableSymbol<std::shared_ptr<SymVar>>&) {};
  virtual void visit(TableSymbol<std::shared_ptr<Const>>&) {};
  virtual void visit(TableSymbol<std::shared_ptr<SymFun>>&) {};
  virtual void visit(Tables&) {};
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
  void visit(Cast&) override;

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
  void visit(String&) override;
  void visit(Void&) override;

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
  void visit(Exit&) override;

  void visit(TableSymbol<std::shared_ptr<SymType>>&) override;
  void visit(TableSymbol<std::shared_ptr<SymVar>>&) override;
  void visit(TableSymbol<std::shared_ptr<Const>>&) override;
  void visit(TableSymbol<std::shared_ptr<SymFun>>&) override;
  void visit(Tables&) override;

 private:
  std::ofstream out;
  int depth;
  void print(const std::string&);
  void print(const ptr_Type&);
};