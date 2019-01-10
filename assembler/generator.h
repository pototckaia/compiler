#pragma once

#include <sstream>
#include "visitor.h"
#include "opcode.h"
#include <stack>

class AsmGenerator : public Visitor {
 public:
  AsmGenerator(const std::string&);
  ~AsmGenerator() override = default;

  void visit(Variable&) override;
  void visit(Literal&) override;

  void visit(BinaryOperation&) override;
  void visit_arithmetic(BinaryOperation&);
  void visit_cmp(BinaryOperation&);
  void visit_logical(BinaryOperation&);

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

 public:
  std::ofstream asm_file;
  StackTable stackTable;

  bool need_lvalue;
  void visit_lvalue(Expression&);

  std::stack<std::pair<std::string, std::string>> loop;

  Operand buf_var_name;
  ptr_Type syscall_param_type;
  bool last_param;

  std::stringstream buf_string;
  void clear_buf_string();
  std::string add_string(const std::string& value);

  const std::string label_main = "main";
  // for syscall
  const std::string label_fmt_int = "fmt_int";
  const std::string label_fmt_double = "fmt_double";
  const std::string label_fmt_char = "fmt_char";
  const std::string label_fmt_new_line = "fmt_new_line";
};


class AsmGlobalDecl : public Visitor {
 public:
  AsmGlobalDecl(std::ostream& a) : a(a) {}

  void visit(Int&) override;
  void visit(Double&) override;
  void visit(Char&) override;
  void visit(TPointer&) override;
  void visit(Boolean&) override;

  void visit(Alias&) override;
  void visit(Pointer&) override;
  void visit(StaticArray&) override;
  void visit(Record&) override;
  void visit(FunctionSignature&) override;

  void visit(GlobalVar&) override;

  // for fmt decl
  void visit(std::string name, std::string value);
  void visit(std::string name, int value);


 private:
  std::ostream& a;
};
