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

void AsmGenerator::visit_lvalue(ptr_Expr& n) {
  needLvalue = true;
  n->accept(*this);
  needLvalue = false;
}

void AsmGenerator::visit(MainFunction& m) {
  asm_file << Command(Printf) // extern
           << Command(Scanf)
           << "\n";
  asm_file << Command(label_main, true); // global main

  AsmGlobalDecl a(asm_file);
  a.visit(label_fmt_int, "%Ld");
  a.visit(label_fmt_double, "%lf");
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
    << Command(code)
    << Command(Label(label_main))
    << Comment("prolog")
    << Command(PUSH, {RBP})
    << Command(MOV, {RBP}, {RSP});


  m.body->accept(*this);

  asm_file
    << Comment("epilog")
    << Command(POP, {RBP})
    << Command(XOR, {RAX}, {RAX})
    << Command(RET);

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
  buf_var_name = Operand(EffectiveAddress(RBP, -((int) l.offset))); // [rbp - offset]
}

void AsmGenerator::visit(GlobalVar& g) {
  buf_var_name = Operand(EffectiveAddress(Label(g.label)), none); // [label]
}

void AsmGenerator::visit(ParamVar& p) {
  buf_var_name = Operand(EffectiveAddress(RBP, (int) p.offset)); // [rbp + offset]
}

void AsmGenerator::visit(Const&) {}

void AsmGenerator::visit(Variable& v) {
  if (stackTable.isFunction(v.getName()->getValueString())) {
    const auto& f = stackTable.findFunction(v.getName()->getValueString());
    buf_var_name = Operand(Label(f->get_label())); // label
  } else {
    stackTable.findVar(v.getName()->getValueString())->accept(*this);
  }
  if (needLvalue) {
    asm_file
      << Comment("lvalue variable")
      << Command(LEA, {RAX}, buf_var_name)
      << Command(PUSH, {RAX});
  } else {
    asm_file
      << Comment("rvalue variable")
      << Command(MOV, {RAX}, buf_var_name)
      << Command(PUSH, {RAX});
  }
}

void AsmGenerator::visit(Literal& l) {
  if (l.type->isInt()) {
    asm_file
      << Comment("int literal")
      << Command(PUSH, {l.getValue()->getInt()});
  } else if (l.type->isDouble()) {
    asm_file
      << Comment("double literal")
      << Command(MOV, {RAX}, {l.getValue()->getDouble(), double64})
      << Command(PUSH, {RAX});
  } else if (l.type->isChar()) {
    asm_file
      << Comment("char literal")
      << Command(PUSH, {l.getValue()->getValueString()});
  } else if (l.type->isString()) {
    auto label_name = add_string(l.getValue()->getValueString());
    asm_file
      << Comment("string literal")
      << Command(PUSH, {Label(label_name)});
  } else if (l.getValue()->getTokenType() == tok::TokenType::Nil ||
             l.getValue()->getTokenType() == tok::TokenType::False) {
    asm_file
      << Comment("false or nil literal")
      << Command(PUSH, {(uint64_t) 0});
  } else if (l.getValue()->getTokenType() == tok::TokenType::True) {
    asm_file
      << Comment("true literal")
      << Command(PUSH, {(uint64_t) 1});
  }
}

void AsmGenerator::visit_arithmetic(BinaryOperation& b) {
  if (b.left->type->isPointer()) {
    visit_lvalue(b.left);
  } else {
    b.left->accept(*this);
  }
  if (b.right->type->isPointer()) {
    visit_lvalue(b.right);
  } else {
    b.right->accept(*this);
  }

  asm_file
    << Comment("arithmetic operation")
    << Command(POP, {R8}) // right
    << Command(POP, {RAX}); // left

  if (b.type->isDouble()) {
    asm_file
      << Command(MOVQ, {XMM0, none}, {RAX})
      << Command(MOVQ, {XMM1, none}, {R8});
    switch (b.getOpr()->getTokenType()) {
      case tok::TokenType::Plus: {
        asm_file << Command(ADDSD, {XMM0, none}, {XMM1, none});
        break;
      }
      case tok::TokenType::Minus: {
        asm_file << Command(SUBSD, {XMM0, none}, {XMM1, none});
        break;
      }
      case tok::TokenType::Asterisk: {
        asm_file << Command(MULSD, {XMM0, none}, {XMM1, none});
        break;
      }
      case tok::TokenType::Slash: {
        asm_file << Command(DIVSD, {XMM0, none}, {XMM1, none});
        break;
      }
      default:
        break;
    }
    asm_file
      << Command(MOVQ, {RAX}, {XMM0, none})
      << Command(PUSH, {RAX});
  } else {
    switch (b.getOpr()->getTokenType()) {
      case tok::TokenType::Plus: {
        asm_file << Command(ADD, {RAX}, {R8});
        break;
      }
      case tok::TokenType::Minus: {
        asm_file << Command(SUB, {RAX}, {R8});
        break;
      }
      case tok::TokenType::Asterisk: {
        asm_file << Command(IMUL, {RAX}, {R8});
        break;
      }
      case tok::TokenType::Div: {
        asm_file
          << Command(XOR, {RDX}, {RDX})
          << Command(CQO)
          << Command(IDIV, {R8});
        break;
      }
      case tok::TokenType::Mod: {
        asm_file
          << Command(XOR, {RDX}, {RDX})
          << Command(CQO)
          << Command(IDIV, {R8})
          << Command(MOV, {RAX}, {RDX});
        break;
      }
      case tok::TokenType::ShiftLeft:
      case tok::TokenType::Shl: {
        asm_file
          << Command(MOV, {RCX}, {R8})
          << Command(SHL, {RAX}, {CL, Pref::b});
        break;
      }
      case tok::TokenType::ShiftRight:
      case tok::TokenType::Shr: {
        asm_file
          << Command(MOV, {RCX}, {R8})
          << Command(SHR, {RAX}, {CL, Pref::b});
        break;
      }
      default:
        break;
    }
    asm_file << Command(PUSH, {RAX});
  }
}

void AsmGenerator::visit_cmp(BinaryOperation& b) {
  if (b.type->isPointer()) {
    visit_lvalue(b.left);
    visit_lvalue(b.right);
  } else {
    b.left->accept(*this);
    b.right->accept(*this);
  }
  asm_file << Comment("cmp operation");
  if (b.right->type->isDouble()) {
    asm_file
      << Command(POP, {R11}) // right
      << Command(POP, {RAX}) // left
      << Command(MOVQ, {XMM0, none}, {RAX})
      << Command(MOVQ, {XMM1, none}, {R11})
      << Command(XOR, {RAX}, {RAX})
      << Command(COMISD, {XMM0, none}, {XMM1, none});
    switch (b.getOpr()->getTokenType()) {
      case tok::TokenType::Equals: {
        asm_file << Command(SETE, {AL, none});
        break;
      }
      case tok::TokenType::NotEquals: {
        asm_file << Command(SETNE, {AL, none});
        break;
      }
      case tok::TokenType::StrictGreater: {
        asm_file << Command(SETA, {AL, none});
        break;
      }
      case tok::TokenType::GreaterOrEquals: {
        asm_file << Command(SETAE, {AL, none});
        break;
      }
      case tok::TokenType::StrictLess: {
        asm_file << Command(SETB, {AL, none});
        break;
      }
      case tok::TokenType::LessOrEquals: {
        asm_file << Command(SETBE, {AL, none});
        break;
      }
      default:
        break;
    }
  } else {
    asm_file
      << Command(POP, {R11}) // right
      << Command(POP, {RDX}) // left
      << Command(XOR, {RAX}, {RAX})
      << Command(CMP, {RDX}, {R11});
    switch (b.getOpr()->getTokenType()) {
      case tok::TokenType::Equals: {
        asm_file << Command(SETE, {AL, none});
        break;
      }
      case tok::TokenType::NotEquals: {
        asm_file << Command(SETNE, {AL, none});
        break;
      }
      case tok::TokenType::StrictGreater: {
        asm_file << Command(SETG, {AL, none});
        break;
      }
      case tok::TokenType::GreaterOrEquals: {
        asm_file << Command(SETGE, {AL, none});
        break;
      }
      case tok::TokenType::StrictLess: {
        asm_file << Command(SETL, {AL, none});
        break;
      }
      case tok::TokenType::LessOrEquals: {
        asm_file << Command(SETLE, {AL, none});
        break;
      }
      default:
        break;
    }
  }
  asm_file << Command(PUSH, {RAX});
}

void AsmGenerator::visit_logical(BinaryOperation& b) {
  switch (b.getOpr()->getTokenType()) {
    case tok::TokenType::Xor: {
      b.left->accept(*this);
      b.right->accept(*this);
      asm_file << Comment("xor operation");
      asm_file
        << Command(POP, {RBX}) // righ
        << Command(POP, {RAX}) // left
        << Command(XOR, {RAX}, {RBX})
        << Command(PUSH, {RAX});
      return;
    }
    case tok::TokenType::And: {
      if (b.type->isInt()) {
        b.left->accept(*this);
        b.right->accept(*this);
        asm_file << Comment("and operation");
        asm_file
          << Command(POP, {RBX}) // righ
          << Command(POP, {RAX}) // left
          << Command(AND, {RAX}, {RBX})
          << Command(PUSH, {RAX});
        return;
      } else {
        auto _false = getLabel();
        auto _true = getLabel();

        b.left->accept(*this);
        asm_file
          << Comment("and boolean operation")
          << Command(POP, {RAX})
          << Command(TEST, {RAX}, {RAX})
          << Command(JZ, {Label(_false)});

        b.right->accept(*this);

        asm_file
          << Command(POP, {RAX})
          << Command(TEST, {RAX}, {RAX})
          << Command(JZ, {Label(_false)})
          << Command(PUSH, {(uint64_t) 1})
          << Command(JMP, {Label(_true)})
          << Command(Label(_false))
          << Command(PUSH, {(uint64_t) 0})
          << Command(Label(_true));
        return;
      }
    }
    case tok::TokenType::Or: {
      if (b.type->isInt()) {
        b.left->accept(*this);
        b.right->accept(*this);
        asm_file
          << Comment("or operation")
          << Command(POP, {RBX}) // righ
          << Command(POP, {RAX}) // left
          << Command(OR, {RAX}, {RBX})
          << Command(PUSH, {RAX});
        return;
      } else {
        auto _false = getLabel();
        auto _true = getLabel();

        b.left->accept(*this);
        asm_file
          << Comment("or boolean operation")
          << Command(POP, {RAX})
          << Command(TEST, {RAX}, {RAX})
          << Command(JNZ, {Label(_true)});

        b.right->accept(*this);

        asm_file
          << Command(POP, {RAX})
          << Command(TEST, {RAX}, {RAX})
          << Command(JNZ, {Label(_true)})
          << Command(PUSH, {(uint64_t) 0})
          << Command(JMP, {Label(_false)})
          << Command(Label(_true))
          << Command(PUSH, {(uint64_t) 1})
          << Command(Label(_false));
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
          << Command(POP, {RAX})
          << Command(NEG, {RAX})
          << Command(PUSH, {RAX});
      } else {
        asm_file
          << Command(POP, {RAX})
          << Command(MOVQ, {XMM1, none}, {RAX})
          << Command(XORPD, {XMM0, none}, {XMM0, none})
          << Command(SUBSD, {XMM0, none}, {XMM1, none})
          << Command(MOVQ, {RAX}, {XMM0, none})
          << Command(PUSH, {RAX});
      }
      return;
    }
    case tok::TokenType::Not: {
      u.expr->accept(*this);
      if (u.type->isBool()) {
        asm_file
          << Comment("boolean not")
          << Command(POP, {RAX})
          << Command(XOR, {RAX}, {(uint64_t) 1})
          << Command(PUSH, {RAX});
      } else {
        asm_file
          << Comment("not operation")
          << Command(POP, {RAX})
          << Command(NOT, {RAX})
          << Command(PUSH, {RAX});
      }
      return;
    }
    case tok::TokenType::At: {
      asm_file << Comment("@");
      visit_lvalue(u.expr);
      return;
    }
    case tok::TokenType::Caret: {
      asm_file << Comment("^");
      if (needLvalue) {
        // node ==pointer
        visit_lvalue(u.expr); // ?
      } else {
        u.expr->accept(*this);
        asm_file
          << Command(POP, {RAX})
          << Command(PUSH, {EffectiveAddress(RAX)});
      }
      return;
    }
    default: {
      break;
    }
  }
}

void AsmGenerator::visit(ArrayAccess&) {
}

void AsmGenerator::visit(RecordAccess&) {
}

void AsmGenerator::visit(Cast& c) {
  c.expr->accept(*this);
  if (c.expr->type->isDouble() && c.type->isInt()) {
    asm_file
      << Comment("double to int")
      << Command(POP, {RAX})
      << Command(MOVQ, {XMM0, none}, {RAX})
      << Command(CVTSD2SI, {RAX}, {XMM0, none})
      << Command(PUSH, {RAX});
  } else if (c.expr->type->isInt() && c.type->isDouble()) {
    asm_file
      << Comment("int to double")
      << Command(POP, {RAX})
      << Command(CVTSI2SD, {XMM0, none}, {RAX})
      << Command(MOVQ, {RAX}, {XMM0, none})
      << Command(PUSH, {RAX});
  }
}

void AsmGenerator::visit(AssignmentStmt& a) {
  if (a.right->type->isPointer() || a.right->type->isProcedureType()) {
    visit_lvalue(a.right);
  } else {
    a.right->accept(*this);
  }
  visit_lvalue(a.left);

  if (a.left->type->isInt() || a.left->type->isDouble() ||
      a.left->type->isChar() || a.left->type->isPointer() ||
      a.left->type->isProcedureType()) {
    switch (a.getOpr()->getTokenType()) {
      case tok::TokenType::AssignmentWithPlus: {
        asm_file
          << Comment("+=")
          << Command(POP, {RAX}) // left - address
          << Command(POP, {R8}) // right
          << Command(MOV, {R9}, {EffectiveAddress(RAX)}); // *left
        if (a.right->type->isDouble()) {
          asm_file
            << Command(MOVQ, {XMM0, none}, {R9})
            << Command(MOVQ, {XMM1, none}, {R8})
            << Command(ADDSD, {XMM0, none}, {XMM1, none})
            << Command(MOVQ, {R9}, {XMM0, none});
        } else {
          asm_file << Command(ADD, {R9}, {R8});
        }
        asm_file
          << Command(PUSH, {R9}) // // *left + right
          << Command(PUSH, {RAX});
        break;
      }
      case tok::TokenType::AssignmentWithMinus: {
        asm_file
          << Comment("+=")
          << Command(POP, {RAX}) // left - address
          << Command(POP, {R8}) // right
          << Command(MOV, {R9}, {EffectiveAddress(RAX)}); // *left
        if (a.right->type->isDouble()) {
          asm_file
            << Command(MOVQ, {XMM0, none}, {R9})
            << Command(MOVQ, {XMM1, none}, {R8})
            << Command(SUBSD, {XMM0, none}, {XMM1, none})
            << Command(MOVQ, {R9}, {XMM0, none});
        } else {
          asm_file << Command(SUB, {R9}, {R8});
        }
        asm_file
          << Command(PUSH, {R9}) // // *left + right
          << Command(PUSH, {RAX});
        break;
      }
      case tok::TokenType::AssignmentWithAsterisk: {
        asm_file
          << Comment("+=")
          << Command(POP, {RAX}) // left - address
          << Command(POP, {R8}) // right
          << Command(MOV, {R9}, {EffectiveAddress(RAX)}); // *left
        if (a.right->type->isDouble()) {
          asm_file
            << Command(MOVQ, {XMM0, none}, {R9})
            << Command(MOVQ, {XMM1, none}, {R8})
            << Command(MULSD, {XMM0, none}, {XMM1, none})
            << Command(MOVQ, {R9}, {XMM0, none});
        } else {
          asm_file << Command(IMUL, {R9}, {R8});
        }
        asm_file
          << Command(PUSH, {R9}) // // *left + right
          << Command(PUSH, {RAX});
        break;
      }
      case tok::TokenType::AssignmentWithSlash: {
        asm_file
          << Comment("+=")
          << Command(POP, {RAX}) // left - address
          << Command(POP, {R8}) // right
          << Command(MOV, {R9}, {EffectiveAddress(RAX)}) // *left
          << Command(MOVQ, {XMM0, none}, {R9})
          << Command(MOVQ, {XMM1, none}, {R8})
          << Command(DIVSD, {XMM0, none}, {XMM1, none})
          << Command(MOVQ, {R9}, {XMM0, none})
          << Command(PUSH, {R9}) // // *left + right
          << Command(PUSH, {RAX});
        break;
      }
      default: {
        break;
      }
    }
    asm_file
      << Comment("assigment")
      << Command(POP, {RAX}) // left
      << Command(POP, {EffectiveAddress(RAX)});
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
      << Command(MOV, {RDI}, {Label(label_fmt_new_line)})
      << Command(CALL, {Printf});
    return;
  }

  if (syscall_param_type->isString()) {
    asm_file
      << Comment("printf string")
      << Command(POP, {RAX})
      << Command(MOV, {RDI}, {RAX})
      << Command(XOR, {RAX}, {RAX})
      << Command(CALL, {Printf});
  } else if (syscall_param_type->isInt()) {
    asm_file
      << Comment("printf int")
      << Command(MOV, {RDI}, {Label(label_fmt_int)})
      << Command(POP, {RSI})
      << Command(XOR, {RAX}, {RAX})
      << Command(CALL, {Printf});
  } else if (syscall_param_type->isDouble()) {
    asm_file
      << Comment("printf double")
      << Command(MOV, {RDI}, {Label(label_fmt_double)})
      << Command(POP, {RAX})
      << Command(MOVQ, {XMM0, none}, {RAX})
      << Command(MOV, {RAX}, {(uint64_t) 1})
      << Command(CALL, {Printf});
  } else if (syscall_param_type->isChar()) {
    asm_file
      << Comment("printf char")
      << Command(MOV, {RDI}, {Label(label_fmt_char)})
      << Command(POP, {RSI})
      << Command(XOR, {RAX}, {RAX})
      << Command(CALL, {Printf});
  }

  if (last_param && w.isnewLine) {
    asm_file
      << Comment("printf new line")
      << Command(MOV, {RDI}, {Label(label_fmt_new_line)})
      << Command(CALL, {Printf});
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
  }
}

void AsmGenerator::visit(IfStmt& i) {
  auto _else = getLabel();
  auto _endif = getLabel();
  asm_file << Comment("if _else: " + _else + " _endif: " + _endif);
  i.condition->accept(*this);
  asm_file
    << Command(POP, {RAX})
    << Command(TEST, {RAX}, {RAX})
    << Command(JZ, {Label(_else)});
  i.then_stmt->accept(*this);
  asm_file
    << Command(JMP, {Label(_endif)})
    << Command(Label(_else));
  if (i.else_stmt != nullptr) {
    i.else_stmt->accept(*this);
  }
  asm_file << Command(Label(_endif));
}

void AsmGenerator::visit(WhileStmt& w) {
  auto _body = getLabel();
  auto _end = getLabel();
  asm_file << Comment("whiel _body: " + _body + " _end: " + _end);
  loop.push(std::make_pair(_body, _end));
  asm_file
    << Command(Label(_body));
  w.condition->accept(*this);
  asm_file
    << Command(POP, {RAX})
    << Command(TEST, {RAX}, {RAX})
    << Command(JZ, {Label(_end)});
  w.block->accept(*this);
  asm_file
    << Command(JMP, {Label(_body)})
    << Command(Label(_end));
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

  needLvalue = true;
  f.getVar()->accept(*this);
  needLvalue = false;

  f.high->accept(*this);

  asm_file
    // init
    << Command(POP, {R13}) // high
    << Command(POP, {R14}) // add_var
    << Command(POP, {EffectiveAddress(R14)}) // *add_var = low
    << Command(PUSH, {R13}) // high
    << Command(PUSH, {R14}) // add_var
    << Command(Label(_body))
    << Command(POP, {R14}) // add_var
    << Command(POP, {R13}) // high
    << Command(CMP, {R13}, {EffectiveAddress(R14)});

  if (f.direct) {
    // to - high < *add_var
    asm_file << Command(JL, {Label(_end)});
  } else {
    // downto - high > *add_var
    asm_file << Command(JG, {Label(_end)});
  }

  asm_file
    << Command(PUSH, {R13})  // high
    << Command(PUSH, {R14}); // add_var

  f.block->accept(*this);
  asm_file
    << Command(Label(_continue))
    << Command(POP, {R14}); // add_var

  if (f.direct) {
    asm_file << Command(INC, {EffectiveAddress(R14)});
  } else {
    asm_file << Command(DEC, {EffectiveAddress(R14)});
  }
  asm_file
    << Command(PUSH, {R14})
    << Command(JMP, {Label(_body)})
    << Command(Label(_break))
    << Command(POP, {RAX})
    << Command(POP, {RAX})
    << Command(Label(_end));

  loop.pop();
}

void AsmGenerator::visit(BreakStmt&) {
  asm_file
    << Comment("break")
    << Command(JMP, {Label(loop.top().second)});
}

void AsmGenerator::visit(ContinueStmt&) {
  asm_file
    << Comment("continue")
    << Command(JMP, {Label(loop.top().first)});
}

void AsmGenerator::visit(Int&) {
}

void AsmGenerator::visit(Double&) {
}

void AsmGenerator::visit(Char&) {
}

void AsmGenerator::visit(TPointer&) {
}

void AsmGenerator::visit(Boolean&) {
}

void AsmGenerator::visit(String&) {
}

void AsmGenerator::visit(Void&) {
}

void AsmGenerator::visit(Alias&) {
}

void AsmGenerator::visit(Pointer&) {
}

void AsmGenerator::visit(StaticArray&) {
}

void AsmGenerator::visit(OpenArray&) {
}

void AsmGenerator::visit(Record&) {
}

void AsmGenerator::visit(FunctionSignature&) {
}

void AsmGenerator::visit(ForwardType&) {
}

void AsmGenerator::visit(ForwardFunction&) {
}

void AsmGenerator::visit(Function&) {
}

void AsmGlobalDecl::visit(GlobalVar& v) {
  a << Command(bss);
  v.label = getLabelName(v.name);
  a << Label(v.label) << ": ";
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

void AsmGlobalDecl::visit(StaticArray& s) {
  a << RESB << " " << s.size() << "\n";
}

void AsmGlobalDecl::visit(Record& r) {
  a << RESB << " " << r.size() << "\n";
}

void AsmGlobalDecl::visit(std::string name, std::string value) {
  a
    << Command(data)
    << Label(name) << ": " << DB << " '";
  for (auto& e : value) {
    if (e == 10) {
      a << "',10,'";
    } else if (e == 13) {
      a << "',13,'";
    } else {
      a << e;
    }
  }
  a << "',0\n";
}

void AsmGlobalDecl::visit(std::string name, int value) {
  a
    << Command(data)
    << Label(name) << ": " << DB << " " << value << ",0\n";
}