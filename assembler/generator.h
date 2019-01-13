#pragma once

#include <sstream>
#include "visitor.h"
#include "opcode.h"
#include <stack>
#include <unordered_map>

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

  void visit(StaticArray&) override;
  void visit(OpenArray&) override;
  void visit(Pointer&) override;
  void visit(Alias&) override;

  void visit(LocalVar&) override;
  void visit(GlobalVar&) override;
  void visit(ParamVar&) override;
  void visit(Const&) override;

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

  // for lvalue
  bool need_lvalue;
  void visit_lvalue(Expression&);

   const std::unordered_map<tok::TokenType, Instruction> arith_i = {
    {tok::TokenType::Plus, ADD},
    {tok::TokenType::AssignmentWithPlus, ADD},
    {tok::TokenType::Minus, SUB},
    {tok::TokenType::AssignmentWithMinus, SUB},
    {tok::TokenType::Asterisk, IMUL},
    {tok::TokenType::AssignmentWithAsterisk, IMUL},
    {tok::TokenType::Div, IDIV},
  };
  const std::unordered_map<tok::TokenType, Instruction> arith_d = {
    {tok::TokenType::Plus, ADDSD},
    {tok::TokenType::AssignmentWithPlus, ADDSD},
    {tok::TokenType::Minus, SUBSD},
    {tok::TokenType::AssignmentWithMinus, SUBSD},
    {tok::TokenType::Asterisk, MULSD},
    {tok::TokenType::AssignmentWithAsterisk, MULSD},
    {tok::TokenType::Slash, DIVSD},
    {tok::TokenType::AssignmentWithSlash, DIVSD}
  };
  const std::unordered_map<tok::TokenType, Instruction> cmp_i = {
    {tok::TokenType::Equals, SETE},
    {tok::TokenType::NotEquals, SETNE},
    {tok::TokenType::StrictGreater, SETG},
    {tok::TokenType::GreaterOrEquals, SETGE},
    {tok::TokenType::StrictLess, SETL},
    {tok::TokenType::LessOrEquals, SETLE},
  };
  const std::unordered_map<tok::TokenType, Instruction> cmp_d = {
    {tok::TokenType::Equals, SETE},
    {tok::TokenType::NotEquals, SETNE},
    {tok::TokenType::StrictGreater, SETA},
    {tok::TokenType::GreaterOrEquals, SETAE},
    {tok::TokenType::StrictLess, SETB},
    {tok::TokenType::LessOrEquals, SETBE},
  };

  // for break
  std::stack<std::pair<std::string, std::string>> loop;

  // for variable  - local, global, parameter, function, constant
  Operand buf_var_name;
  StackTable stackTable;

  // for write -> type param
  ptr_Type syscall_param_type;
  bool last_param;

  // generator string constant
  std::stringstream buf_string;
  void clear_buf_string();
  std::string add_string(const std::string& value);

  // for array access
  ListExpr bounds;

  void pushRvalue(ptr_Type&);

  // global main
  const std::string label_main = "main";
  // for write and read
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
