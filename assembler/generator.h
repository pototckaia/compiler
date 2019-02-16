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
  void visit(Cast&) override;
  void visit(AssignmentStmt&) override;

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
  void visit(ForwardFunction&) override;
  void visit(Function&) override;

  void visit(FunctionCallStmt&) override;
  void visit(FunctionCall&) override;

  void visit(MainFunction&) override;
  void visit_function(SymFun&);

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

  // for functionCallStmt
  bool isSkipResult = false;
  uint64_t sizeParam = 0;

  // for lvalue
  bool need_lvalue;
  void visit_lvalue(Expression&);

   const std::unordered_map<TokenType, Instruction> arith_i = {
    {TokenType::Plus, ADD},
    {TokenType::AssignmentWithPlus, ADD},
    {TokenType::Minus, SUB},
    {TokenType::AssignmentWithMinus, SUB},
    {TokenType::Asterisk, IMUL},
    {TokenType::AssignmentWithAsterisk, IMUL},
    {TokenType::Div, IDIV},
  };
  const std::unordered_map<TokenType, Instruction> arith_d = {
    {TokenType::Plus, ADDSD},
    {TokenType::AssignmentWithPlus, ADDSD},
    {TokenType::Minus, SUBSD},
    {TokenType::AssignmentWithMinus, SUBSD},
    {TokenType::Asterisk, MULSD},
    {TokenType::AssignmentWithAsterisk, MULSD},
    {TokenType::Slash, DIVSD},
    {TokenType::AssignmentWithSlash, DIVSD}
  };
  const std::unordered_map<TokenType, Instruction> cmp_i = {
    {TokenType::Equals, SETE},
    {TokenType::NotEquals, SETNE},
    {TokenType::StrictGreater, SETG},
    {TokenType::GreaterOrEquals, SETGE},
    {TokenType::StrictLess, SETL},
    {TokenType::LessOrEquals, SETLE},
  };
  const std::unordered_map<TokenType, Instruction> cmp_d = {
    {TokenType::Equals, SETE},
    {TokenType::NotEquals, SETNE},
    {TokenType::StrictGreater, SETA},
    {TokenType::GreaterOrEquals, SETAE},
    {TokenType::StrictLess, SETB},
    {TokenType::LessOrEquals, SETBE},
  };

  // for break
  std::stack<std::pair<std::string, std::string>> loop;

  // for variable  - local, global, parameter, function, constant
  Operand buf_var_name;
  StackTable stackTable;

  // for write -> type param
  ListExpr syscall_params;
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
