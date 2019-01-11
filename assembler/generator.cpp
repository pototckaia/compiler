#include <utility>
#include "generator.h"

#include "opcode.h"

AsmGenerator::AsmGenerator(const std::string& s) : asm_file(s) {}

void AsmGenerator::clear_buf_string() {
  asm_file << buf_string.str();
  buf_string.clear();
}

std::string AsmGenerator::add_string(const std::string& value) {
  AsmGlobalDecl a(buf_string);
  std::string label = getStrName();
  a.visit(label, value);
  return label;
}

void AsmGenerator::visit_lvalue(Expression& n) {
  need_lvalue = true;
  n.accept(*this);
  need_lvalue = false;
}

void AsmGenerator::visit(MainFunction& m) {
  asm_file
    << cmd(Printf) // extern
    << cmd(Scanf) << "\n"
    << cmd(label_main, true); // global main

  AsmGlobalDecl a(asm_file);
  a.visit(label_fmt_int, "%Ld");
  a.visit(label_fmt_double, "%1.16E");
  a.visit(label_fmt_char, "%c");
  a.visit(label_fmt_new_line, 10);
  for (auto& var : m.decl.tableVariable) {
    var.second->accept(a);
  }
  for (auto& var : m.decl.tableFunction) {
    auto label = getLabelName(var.second->name);
    var.second->set_label(label);
  }

  stackTable.push(m.decl);
  asm_file
    << cmd(code)
    << cmd(Label(label_main))
    << Comment("prolog")
    << cmd(PUSH, {RBP})
    << cmd(MOV, {RBP}, {RSP});

  m.body->accept(*this);

  asm_file
    << Comment("epilog")
    << cmd(POP, {RBP})
    << cmd(XOR, {RAX}, {RAX})
    << cmd(RET);

  clear_buf_string();
}

void AsmGenerator::visit(BlockStmt& b) {
  for (auto& e : b.getBlock()) {
    e->accept(*this);
  }
}

void AsmGenerator::visit(FunctionCallStmt& f) {
  f.getFunctionCall()->accept(*this);
}

void AsmGenerator::visit(LocalVar& l) {
  buf_var_name = Operand(EffectiveAddress(RBP, l.offset, true)); // [rbp - offset]
}

void AsmGenerator::visit(GlobalVar& g) {
  buf_var_name = Operand(EffectiveAddress(Label(g.label)), none); // [label]
}

void AsmGenerator::visit(ParamVar& p) {
  buf_var_name = Operand(EffectiveAddress(RBP, (int) p.offset)); // [rbp + offset]
}

void AsmGenerator::visit(Const&) {}

void AsmGenerator::visit(Function& f) {
  buf_var_name = Operand(Label(f.get_label())); // label
}

void AsmGenerator::visit(Variable& v) {
  if (stackTable.isFunction(v.getName()->getValueString())) {
    stackTable.findFunction(v.getName()->getValueString())->accept(*this);
  } else {
    stackTable.findVar(v.getName()->getValueString())->accept(*this);
  }
  if (need_lvalue) {
    asm_file
      << Comment("lvalue variable")
      << cmd(LEA, {RAX}, buf_var_name)
      << cmd(PUSH, {RAX});
  } else {
    asm_file
      << Comment("rvalue variable")
      << cmd(MOV, {RAX}, buf_var_name)
      << cmd(PUSH, {RAX});
  }
}

void AsmGenerator::visit(Literal& l) {
  if (l.type->isInt()) {
    asm_file
      << Comment("int literal")
      << cmd(PUSH, {l.getValue()->getInt()});
  } else if (l.type->isDouble()) {
    asm_file
      << Comment("double literal")
      << cmd(MOV, {RAX}, {l.getValue()->getDouble(), double64})
      << cmd(PUSH, {RAX});
  } else if (l.type->isChar()) {
    asm_file
      << Comment("char literal")
      << cmd(PUSH, {l.getValue()->getValueString()});
  } else if (l.type->isString()) {
    auto label_name = add_string(l.getValue()->getValueString());
    asm_file
      << Comment("string literal")
      << cmd(PUSH, {Label(label_name)});
  } else if (l.getValue()->getTokenType() == tok::TokenType::Nil ||
             l.getValue()->getTokenType() == tok::TokenType::False) {
    asm_file
      << Comment("false or nil literal")
      << cmd(PUSH, {(uint64_t) 0});
  } else if (l.getValue()->getTokenType() == tok::TokenType::True) {
    asm_file
      << Comment("true literal")
      << cmd(PUSH, {(uint64_t) 1});
  }
}

void AsmGenerator::visit_arithmetic(BinaryOperation& b) {
  b.left->accept(*this);
  b.right->accept(*this);
  auto t = b.getOpr()->getTokenType();
  asm_file
    << Comment("arithmetic operation")
    << cmd(POP, {RCX}) // right
    << cmd(POP, {RAX}); // left

  if (b.type->isDouble()) {
    asm_file
      << cmd(MOVQ, {XMM0, none}, {RAX})
      << cmd(MOVQ, {XMM1, none}, {RCX})
      << cmd(arith_d.at(t), {XMM0, none}, {XMM1, none})
      << cmd(MOVQ, {RAX}, {XMM0, none})
      << cmd(PUSH, {RAX});
  } else {
    switch (b.getOpr()->getTokenType()) {
      case tok::TokenType::Plus:
      case tok::TokenType::Minus:
      case tok::TokenType::Asterisk: {
        asm_file << cmd(arith_i.at(t), {RAX}, {RCX});
        break;
      }
      case tok::TokenType::Div: {
        asm_file
          << cmd(XOR, {RDX}, {RDX})
          << cmd(CQO)
          << cmd(arith_i.at(t), {RCX});
        break;
      }
      case tok::TokenType::Mod: {
        asm_file
          << cmd(XOR, {RDX}, {RDX})
          << cmd(CQO)
          << cmd(IDIV, {RCX})
          << cmd(MOV, {RAX}, {RDX});
        break;
      }
      case tok::TokenType::ShiftLeft:
      case tok::TokenType::Shl: {
        asm_file
          << cmd(SHL, {RAX}, {CL, Pref::b});
        break;
      }
      case tok::TokenType::ShiftRight:
      case tok::TokenType::Shr: {
        asm_file
          << cmd(SHR, {RAX}, {CL, Pref::b});
        break;
      }
      default:
        break;
    }
    asm_file << cmd(PUSH, {RAX});
  }
}

void AsmGenerator::visit_cmp(BinaryOperation& b) {
  b.left->accept(*this);
  b.right->accept(*this);
  auto t = b.getOpr()->getTokenType();
  asm_file << Comment("cmp operation");
  if (b.right->type->isDouble()) {
    asm_file
      << cmd(POP, {R11}) // right
      << cmd(POP, {RAX}) // left
      << cmd(MOVQ, {XMM0, none}, {RAX})
      << cmd(MOVQ, {XMM1, none}, {R11})
      << cmd(XOR, {RAX}, {RAX})
      << cmd(COMISD, {XMM0, none}, {XMM1, none})
      << cmd(cmp_d.at(t), {AL, none});
  } else {
    asm_file
      << cmd(POP, {R11}) // right
      << cmd(POP, {RDX}) // left
      << cmd(XOR, {RAX}, {RAX})
      << cmd(CMP, {RDX}, {R11})
      << cmd(cmp_i.at(t), {AL, none});
  }
  asm_file << cmd(PUSH, {RAX});
}

void AsmGenerator::visit_logical(BinaryOperation& b) {
  switch (b.getOpr()->getTokenType()) {
    case tok::TokenType::Xor: {
      b.left->accept(*this);
      b.right->accept(*this);
      asm_file << Comment("xor operation");
      asm_file
        << cmd(POP, {RBX}) // righ
        << cmd(POP, {RAX}) // left
        << cmd(XOR, {RAX}, {RBX})
        << cmd(PUSH, {RAX});
      return;
    }
    case tok::TokenType::And: {
      if (b.type->isInt()) {
        b.left->accept(*this);
        b.right->accept(*this);
        asm_file << Comment("and operation");
        asm_file
          << cmd(POP, {RBX}) // righ
          << cmd(POP, {RAX}) // left
          << cmd(AND, {RAX}, {RBX})
          << cmd(PUSH, {RAX});
        return;
      } else {
        auto _false = getLabel();
        auto _true = getLabel();

        b.left->accept(*this);
        asm_file
          << Comment("and boolean operation")
          << cmd(POP, {RAX})
          << cmd(TEST, {RAX}, {RAX})
          << cmd(JZ, {Label(_false)});

        b.right->accept(*this);

        asm_file
          << cmd(POP, {RAX})
          << cmd(TEST, {RAX}, {RAX})
          << cmd(JZ, {Label(_false)})
          << cmd(PUSH, {(uint64_t) 1})
          << cmd(JMP, {Label(_true)})
          << cmd(Label(_false))
          << cmd(PUSH, {(uint64_t) 0})
          << cmd(Label(_true));
        return;
      }
    }
    case tok::TokenType::Or: {
      if (b.type->isInt()) {
        b.left->accept(*this);
        b.right->accept(*this);
        asm_file
          << Comment("or operation")
          << cmd(POP, {RBX}) // righ
          << cmd(POP, {RAX}) // left
          << cmd(OR, {RAX}, {RBX})
          << cmd(PUSH, {RAX});
        return;
      } else {
        auto _false = getLabel();
        auto _true = getLabel();

        b.left->accept(*this);
        asm_file
          << Comment("or boolean operation")
          << cmd(POP, {RAX})
          << cmd(TEST, {RAX}, {RAX})
          << cmd(JNZ, {Label(_true)});

        b.right->accept(*this);

        asm_file
          << cmd(POP, {RAX})
          << cmd(TEST, {RAX}, {RAX})
          << cmd(JNZ, {Label(_true)})
          << cmd(PUSH, {(uint64_t) 0})
          << cmd(JMP, {Label(_false)})
          << cmd(Label(_true))
          << cmd(PUSH, {(uint64_t) 1})
          << cmd(Label(_false));
        return;
      }
    }
    default:
      break;
  }
}

void AsmGenerator::visit(BinaryOperation& b) {
  switch (b.getOpr()->getTokenType()) {
    case tok::TokenType::Plus:
    case tok::TokenType::Minus:
    case tok::TokenType::Slash:
    case tok::TokenType::Asterisk:
    case tok::TokenType::Div:
    case tok::TokenType::Mod:
    case tok::TokenType::ShiftLeft:
    case tok::TokenType::Shl:
    case tok::TokenType::ShiftRight:
    case tok::TokenType::Shr: {
      visit_arithmetic(b);
      return;
    }
    case tok::TokenType::Equals:
    case tok::TokenType::NotEquals:
    case tok::TokenType::StrictLess:
    case tok::TokenType::StrictGreater:
    case tok::TokenType::LessOrEquals:
    case tok::TokenType::GreaterOrEquals: {
      visit_cmp(b);
      return;
    }
    case tok::TokenType::And:
    case tok::TokenType::Or:
    case tok::TokenType::Xor: {
      visit_logical(b);
    }
    default:
      break;
  }
}

void AsmGenerator::visit(UnaryOperation& u) {
  switch (u.getOpr()->getTokenType()) {
    case tok::TokenType::Minus: {
      u.expr->accept(*this);
      asm_file << Comment("unary minus");
      if (u.type->isInt()) {
        asm_file
          << cmd(POP, {RAX})
          << cmd(NEG, {RAX})
          << cmd(PUSH, {RAX});
      } else {
        asm_file
          << cmd(POP, {RAX})
          << cmd(MOVQ, {XMM1, none}, {RAX})
          << cmd(XORPD, {XMM0, none}, {XMM0, none})
          << cmd(SUBSD, {XMM0, none}, {XMM1, none})
          << cmd(MOVQ, {RAX}, {XMM0, none})
          << cmd(PUSH, {RAX});
      }
      return;
    }
    case tok::TokenType::Not: {
      u.expr->accept(*this);
      if (u.type->isBool()) {
        asm_file
          << Comment("boolean not")
          << cmd(POP, {RAX})
          << cmd(XOR, {RAX}, {(uint64_t) 1})
          << cmd(PUSH, {RAX});
      } else {
        asm_file
          << Comment("not operation")
          << cmd(POP, {RAX})
          << cmd(NOT, {RAX})
          << cmd(PUSH, {RAX});
      }
      return;
    }
    case tok::TokenType::At: {
      asm_file << Comment("@");
      visit_lvalue(*u.expr);
      return;
    }
    case tok::TokenType::Caret: {
      asm_file << Comment("^");
      if (need_lvalue) {
        visit_lvalue(*u.expr);
      } else {
        u.expr->accept(*this);
        asm_file
          << cmd(POP, {RAX})
          << cmd(PUSH, {EffectiveAddress(RAX)});
      }
      return;
    }
    default: {
      break;
    }
  }
}

void AsmGenerator::visit(Pointer& p) {
//  if (bounds.size() == 1) {
//    bounds.back()->accept(*this);
//    bounds.pop_back();
//
//  }
}
void AsmGenerator::visit(StaticArray&) {}
void AsmGenerator::visit(OpenArray&) {}

void AsmGenerator::visit(ArrayAccess& a) {
  Instruction c = need_lvalue ? LEA : MOV;
  visit_lvalue(*a.nameArray);
  bounds = std::move(a.listIndex);
  a.nameArray->type->accept(*this);
  asm_file
    << cmd(POP, {RCX})
    << cmd(c, {EffectiveAddress(RSP)}, {EffectiveAddress(RSP, RCX, 1)}); //
}

void AsmGenerator::visit(RecordAccess& r) {
  Instruction c = need_lvalue ? LEA : MOV;
  visit_lvalue(*r.record);
  auto record = r.record->type->getRecord();
  auto offset = record->offset(r.field->getValueString());
  asm_file
    << Comment("record access")
    << cmd(POP, {R8}) // add_record
    << cmd(c, {R8}, {EffectiveAddress(R8, offset), none})
    << cmd(PUSH, {R8});
}

void AsmGenerator::visit(Cast& c) {
  if (need_lvalue) {
    visit_lvalue(*c.expr);
  } else {
    c.expr->accept(*this);
  }

  if (c.expr->type->isDouble() && c.type->isInt()) {
    asm_file
      << Comment("double to int")
      << cmd(POP, {RAX})
      << cmd(MOVQ, {XMM0, none}, {RAX})
      << cmd(CVTSD2SI, {RAX}, {XMM0, none})
      << cmd(PUSH, {RAX});
  } else if (c.expr->type->isInt() && c.type->isDouble()) {
    asm_file
      << Comment("int to double")
      << cmd(POP, {RAX})
      << cmd(CVTSI2SD, {XMM0, none}, {RAX})
      << cmd(MOVQ, {RAX}, {XMM0, none})
      << cmd(PUSH, {RAX});
  }
}

void AsmGenerator::visit(AssignmentStmt& a) {
  a.right->accept(*this);
  visit_lvalue(*a.left);
  auto t = a.getOpr()->getTokenType();
  switch (t) {
    case tok::TokenType::AssignmentWithPlus:
    case tok::TokenType::AssignmentWithMinus:
    case tok::TokenType::AssignmentWithAsterisk:
    case tok::TokenType::AssignmentWithSlash: {
      asm_file
        << Comment(tok::toString(t))
        << cmd(POP, {RAX}) // left - address
        << cmd(POP, {R8}) // right
        << cmd(MOV, {R9}, {EffectiveAddress(RAX)}); // *left
      if (a.left->type->isDouble()) {
        asm_file
          << cmd(MOVQ, {XMM0, none}, {R9})
          << cmd(MOVQ, {XMM1, none}, {R8})
          << cmd(arith_d.at(t), {XMM0, none}, {XMM1, none})
          << cmd(MOVQ, {R9}, {XMM0, none});
      } else {
        asm_file << cmd(arith_i.at(t), {R9}, {R8});
      }
      asm_file
        << cmd(PUSH, {R9}) // // *left + right
        << cmd(PUSH, {RAX});
      break;
    }
    default: {
      break;
    }
  }
  if (a.left->type->isTrivial()) {
    asm_file
      << Comment("assigment trivial")
      << cmd(POP, {RAX}) // left
      << cmd(POP, {EffectiveAddress(RAX)});
    return;
  }
}

void AsmGenerator::visit(Read&) {}

void AsmGenerator::visit(Trunc&) {}

void AsmGenerator::visit(Round&) {}

void AsmGenerator::visit(Succ&) {}

void AsmGenerator::visit(Prev&) {}

void AsmGenerator::visit(Chr&) {}

void AsmGenerator::visit(Ord&) {}

void AsmGenerator::visit(High&) {}

void AsmGenerator::visit(Low&) {}

void AsmGenerator::visit(Exit&) {}

void AsmGenerator::visit(Write& w) {
  if (syscall_param_type == nullptr && w.isnewLine) {
    asm_file
      << Comment("printf new line")
      << cmd(MOV, {RDI}, {Label(label_fmt_new_line)})
      << cmd(CALL, {Printf});
    return;
  }

  if (syscall_param_type->isString()) {
    asm_file
      << Comment("printf string")
      << cmd(POP, {RAX})
      << cmd(MOV, {RDI}, {RAX})
      << cmd(XOR, {RAX}, {RAX})
      << cmd(CALL, {Printf});
  } else if (syscall_param_type->isInt() || syscall_param_type->isPointer()) {
    asm_file
      << Comment("printf int")
      << cmd(MOV, {RDI}, {Label(label_fmt_int)})
      << cmd(POP, {RSI})
      << cmd(XOR, {RAX}, {RAX})
      << cmd(CALL, {Printf});
  } else if (syscall_param_type->isDouble()) {
    asm_file
      << Comment("printf double")
      << cmd(MOV, {RDI}, {Label(label_fmt_double)})
      << cmd(POP, {RAX})
      << cmd(MOVQ, {XMM0, none}, {RAX})
      << cmd(MOV, {RAX}, {(uint64_t) 1})
      << cmd(CALL, {Printf});
  } else if (syscall_param_type->isChar()) {
    asm_file
      << Comment("printf char")
      << cmd(MOV, {RDI}, {Label(label_fmt_char)})
      << cmd(POP, {RSI})
      << cmd(XOR, {RAX}, {RAX})
      << cmd(CALL, {Printf});
  }

  if (last_param && w.isnewLine) {
    asm_file
      << Comment("printf new line")
      << cmd(MOV, {RDI}, {Label(label_fmt_new_line)})
      << cmd(CALL, {Printf});
  }
}

void AsmGenerator::visit(FunctionCall& f) {
  if (f.getName()->embeddedFunction != nullptr) {
    int i = 0;
    for (const auto& param : f.listParam) {
      syscall_param_type = param->type;
      last_param = i == (f.listParam.size() - 1);
      param->accept(*this);
      f.nameFunction->embeddedFunction->accept(*this);
      ++i;
    }
    if (i == 0 && dynamic_cast<Write*>(f.getName()->embeddedFunction.get())) {
      syscall_param_type = nullptr;
      f.nameFunction->embeddedFunction->accept(*this);
    }
    return;
  }
}

void AsmGenerator::visit(IfStmt& i) {
  auto _else = getLabel();
  auto _endif = getLabel();
  asm_file << Comment("if _else: " + _else + " _endif: " + _endif);
  i.condition->accept(*this);
  asm_file
    << cmd(POP, {RAX})
    << cmd(TEST, {RAX}, {RAX})
    << cmd(JZ, {Label(_else)});
  i.then_stmt->accept(*this);
  asm_file
    << cmd(JMP, {Label(_endif)})
    << cmd(Label(_else));
  if (i.else_stmt != nullptr) {
    i.else_stmt->accept(*this);
  }
  asm_file << cmd(Label(_endif));
}

void AsmGenerator::visit(WhileStmt& w) {
  auto _body = getLabel();
  auto _end = getLabel();
  asm_file << Comment("whiel _body: " + _body + " _end: " + _end);
  loop.push(std::make_pair(_body, _end));
  asm_file
    << cmd(Label(_body));
  w.condition->accept(*this);
  asm_file
    << cmd(POP, {RAX})
    << cmd(TEST, {RAX}, {RAX})
    << cmd(JZ, {Label(_end)});
  w.block->accept(*this);
  asm_file
    << cmd(JMP, {Label(_body)})
    << cmd(Label(_end));
  loop.pop();
}

void AsmGenerator::visit(ForStmt& f) {
  auto _body = getLabel();
  auto _continue = getLabel();
  auto _end = getLabel();
  auto _break = getLabel();

  asm_file
    << Comment("for ")
    << Comment("_body: " + _body + " _continue: " + _continue + " _break: " + _break + " _endfor: " + _end);
  loop.push(std::make_pair(_continue, _break));
  f.low->accept(*this);
  visit_lvalue(*f.var);
  f.high->accept(*this);
  asm_file
    // init
    << cmd(POP, {R13}) // high
    << cmd(POP, {R14}) // add_var
    << cmd(POP, {EffectiveAddress(R14)}) // *add_var = low
    << cmd(PUSH, {R13}) // high
    << cmd(PUSH, {R14}) // add_var
    << cmd(Label(_body))
    << cmd(POP, {R14}) // add_var
    << cmd(POP, {R13}) // high
    << cmd(MOV, {RAX}, {EffectiveAddress(R14)})
    << cmd(CMP, {R13}, {RAX})
    // to - high < *add_var
    // downto - high > *add_var
    << cmd(f.direct ? JL : JG, {Label(_end)})
    << cmd(PUSH, {R13})  // high
    << cmd(PUSH, {R14}); // add_var
  f.block->accept(*this);
  asm_file
    << cmd(Label(_continue))
    << cmd(POP, {R14}) // add_var
    << cmd(f.direct ? INC : DEC, {EffectiveAddress(R14)})
    << cmd(PUSH, {R14})
    << cmd(JMP, {Label(_body)})
    << cmd(Label(_break))
    << cmd(POP, {RAX})
    << cmd(POP, {RAX})
    << cmd(Label(_end));
  loop.pop();
}

void AsmGenerator::visit(BreakStmt&) {
  asm_file
    << Comment("break")
    << cmd(JMP, {Label(loop.top().second)});
}

void AsmGenerator::visit(ContinueStmt&) {
  asm_file
    << Comment("continue")
    << cmd(JMP, {Label(loop.top().first)});
}

void AsmGlobalDecl::visit(GlobalVar& v) {
  v.label = getLabelName(v.name);
  a
    << cmd(bss)
    << Label(v.label) << ": ";
  v.type->accept(*this);
}

void AsmGlobalDecl::visit(Double&) { a << RESQ << " 1\n"; }

void AsmGlobalDecl::visit(Int&) { a << RESQ << " 1\n"; }

void AsmGlobalDecl::visit(Char&) { a << RESQ << " 1\n"; }

void AsmGlobalDecl::visit(Boolean&) { a << RESQ << " 1\n"; }

void AsmGlobalDecl::visit(Pointer&) { a << RESQ << " 1\n"; }

void AsmGlobalDecl::visit(TPointer&) { a << RESQ << " 1\n"; }

void AsmGlobalDecl::visit(FunctionSignature&) { a << RESQ << " 1\n"; }

void AsmGlobalDecl::visit(Alias& a) { a.type->accept(*this); }

void AsmGlobalDecl::visit(StaticArray& s) { a << RESB << " " << s.size() << "\n"; }

void AsmGlobalDecl::visit(Record& r) { a << RESB << " " << r.size() << "\n"; }

void AsmGlobalDecl::visit(std::string name, std::string value) {
  a
    << cmd(data)
    << Label(name) << ": " << DB << " '";
  for (auto& e : value) {
    if (e == 10)
      a << "',10,'";
    else if (e == 13)
      a << "',13,'";
    else
      a << e;
  }
  a << "',0\n";
}

void AsmGlobalDecl::visit(std::string name, int value) {
  a
    << cmd(data)
    << Label(name) << ": " << DB << " " << value << ",0\n";
}