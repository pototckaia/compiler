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

void AsmGenerator::pushRvalue(ptr_Type& t) {
  if (t->isTrivial()) {
    asm_file
      << Comment("rvalue trivial")
      << cmd(POP, {RAX}) // address
      << cmd(PUSH, {adr(RAX)}); // value
  } else {
    auto _start = getLabel();
    auto _end = getLabel();
    uint64_t size = t->size();
    uint64_t len =  size / 8;
    asm_file
      << Comment("rvalue block A_n ... A_0")
      << cmd(POP, {RAX}) // address
      << cmd(ADD, {RAX}, {size - 8}) // end
      << cmd(MOV, {RCX}, {len})
      << cmd(Label(_start))
      << cmd(PUSH, {adr(RAX)})
      << cmd(DEC, {RCX})
      << cmd(JZ, {Label(_end)})
      << cmd(SUB, {RAX}, {(uint64_t) 8})
      << cmd(JMP, {Label(_start)})
      << cmd(Label(_end));
  }
}

void AsmGenerator::visit(MainFunction& m) {
  asm_file
    << cmd(Printf) // extern
    << cmd(Scanf)
    << cmd(label_main, true); // global main

  AsmGlobalDecl a(asm_file);
  a.visit(label_fmt_int, "%Ld");
  a.visit(label_fmt_double, "%1.16E");
  a.visit(label_fmt_char, "%c");
  a.visit(label_fmt_new_line, 10);
  for (auto& var : m.decl.tableVariable) {
    var.second->accept(a);
  }

  // set label for function
  for (auto& var : m.decl.tableFunction) {
    auto label = getLabelName(var.second->getSymbolName());
    var.second->setLabel(label);
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
    << cmd(MOV, {RSP}, {RBP})
    << cmd(POP, {RBP})
    << cmd(XOR, {RAX}, {RAX})
    << cmd(RET);

  // function decl
  for (auto& fun : m.decl.tableFunction) {
    if (fun.second->isEmbedded()) {
      continue;
    }
    visit_function(*fun.second);
  }
  clear_buf_string();
}

void AsmGenerator::visit_function(SymFun& fun) {
  auto s = fun.getSignature();
  stackTable.pushEmpty();

  sizeParam = 0;
  uint64_t offsetParam = 16; // begin [* + 16]; ret -> +8

  // set offset param
  for (auto iter = s->paramsList.rbegin();
       iter != s->paramsList.rend(); ++iter) {
    uint64_t sizeElem = (*iter)->size();
    (*iter)->offset = offsetParam; // pointer for end
    offsetParam += sizeElem;
    sizeParam += sizeElem;
    stackTable.top().tableVariable.insert(*iter);
    asm_file
      << "\n"
      << Comment((*iter)->getSymbolName() +
                 " - A_n .. A_0 <- end: [RBP+" +
                 std::to_string(offsetParam) + "]");
  }

  auto tableLocal = fun.getTable();
  stackTable.push(tableLocal);

  uint64_t sizeLocal = tableLocal.sizeVar();
  uint64_t offsetLocal = 0;

  // set offset local var
  for (auto& e : tableLocal.tableVariable) {
    uint64_t sizeVar = e.second->size();
    if (!s->isProcedure() && e.first == fun.getSymbolName()) { // result var
      sizeLocal -= sizeVar; // not local
      e.second->setOffset(offsetParam);
      asm_file
        << Comment("result " + e.first +
                   " - A_n .. A_0 <- end: [RBP+" +
                   std::to_string(offsetParam) + "]");
    } else {
      offsetLocal += sizeVar;
      e.second->setOffset(offsetLocal);
      asm_file
        << Comment(e.first +
                   ": [RBP-" +
                   std::to_string(offsetLocal) + "]");
    }
  }

  asm_file
    << Comment("Function call " + fun.getSymbolName())
    << cmd(Label(fun.getLabel()))
    << cmd(PUSH, {RBP})
    << cmd(MOV, {RBP}, {RSP})
    << cmd(SUB, {RSP}, {sizeLocal});

  for (auto& e : s->paramsList) {
    if (e->getVarType()->isOpenArray() && e->spec == ParamSpec::NotSpec) {
      auto array = std::dynamic_pointer_cast<OpenArray>(e->getVarType());
      uint64_t sizeElem = array->typeElem->size();
      auto _start = getLabel();
      auto _end = getLabel();
      asm_file
        << Comment("Copy open array ")
        << cmd(LEA, {RAX}, {adr(RBP, e->offset), none}) //
        << cmd(MOV, {RCX}, {adr(RAX, (uint64_t) 8)}) // -8 -> high
        << cmd(INC, {RCX}) // len
        << cmd(MOV, {RDX}, {sizeElem})
        << cmd(IMUL, {RCX}, {RDX}) // // len*sizeElem = size in bit
        << cmd(SUB, {RSP}, {RCX}) // allocate memory
        << cmd(MOV, {RSI}, {adr(RAX)}) // address open array -> источник
        << cmd(MOV, {RDI}, {RSP}) // получатель
        << cmd(REP, MOVSB)
        << cmd(MOV, {adr(RAX)}, {RSP}); // replace address on copy
    }
  }

  fun.getBody()->accept(*this);
  asm_file
    << cmd(MOV, {RSP}, {RBP})
    << cmd(POP, {RBP})
    << cmd(RET, {sizeParam, none});
}

void AsmGenerator::visit(BlockStmt& b) {
  for (auto& e : b.getBlock()) {
    e->accept(*this);
  }
}

void AsmGenerator::visit(LocalVar& l) {
  buf_var_name = Operand(adr(RBP, l.offset, true), none); // [rbp - offset]
}

void AsmGenerator::visit(GlobalVar& g) {
  buf_var_name = Operand(adr(Label(g.label)), none); // [label]
}

void AsmGenerator::visit(ParamVar& p) {
  buf_var_name = Operand(adr(RBP, p.offset), none); // [rbp + offset]
  if (p.getVarType()->isOpenArray()) {
    asm_file
      << Comment("open array")
      << cmd(MOV, {RAX}, buf_var_name);
    buf_var_name = Operand(adr(RAX), none);
    return;
  }
  if (p.spec == ParamSpec::NotSpec) {
    return;
  }
  // по ссылке
  asm_file
    << Comment("var -> pointer")
    << cmd(MOV, {RAX}, buf_var_name);
  buf_var_name = Operand(adr(RAX), none);
}

void AsmGenerator::visit(Const&) {}

void AsmGenerator::visit(Function& f) {
  buf_var_name = Operand(adr(Label(f.getLabel())), none); // label
}

void AsmGenerator::visit(ForwardFunction& f) {
  f.function->accept(*this);
}

void AsmGenerator::visit(Variable& v) {
  if (v.getNodeType()->isProcedureType() &&
      stackTable.isFunction(v.getSubToken().getString())) {
    stackTable.findFunction(v.getSubToken().getString())->accept(*this);
  } else {
    stackTable.findVar(v.getSubToken().getString())->accept(*this);
  }
  asm_file
    << Comment("lvalue variable")
    << cmd(LEA, {RAX}, buf_var_name)
    << cmd(PUSH, {RAX});
  if (!need_lvalue) {
    pushRvalue(v.getNodeType());
  }
}

void AsmGenerator::visit(Literal& l) {
  if (l.getNodeType()->isInt()) {
    asm_file
      << Comment("int literal")
      << cmd(PUSH, {l.getSubToken().getInt()});
  } else if (l.getNodeType()->isDouble()) {
    asm_file
      << Comment("double literal")
      << cmd(MOV, {RAX}, {l.getSubToken().getDouble(), double64})
      << cmd(PUSH, {RAX});
  } else if (l.getNodeType()->isChar()) {
    asm_file
      << Comment("char literal")
      << cmd(PUSH, {l.getSubToken().getString()});
  } else if (l.getNodeType()->isString()) {
    auto label_name = add_string(l.getSubToken().getString());
    asm_file
      << Comment("string literal")
      << cmd(PUSH, {Label(label_name)});
  } else if (l.getSubToken().is(TokenType::Nil) ||
			l.getSubToken().is(TokenType::False)) {
    asm_file
      << Comment("false or nil literal")
      << cmd(PUSH, {(uint64_t) 0});
  } else if (l.getSubToken().is(TokenType::True)) {
    asm_file
      << Comment("true literal")
      << cmd(PUSH, {(uint64_t) 1});
  }
}

void AsmGenerator::visit_arithmetic(BinaryOperation& b) {
  b.getSubLeft()->accept(*this);
  b.getSubRight()->accept(*this);
  auto t = b.getOp().getTokenType();
  asm_file
    << Comment("arithmetic operation")
    << cmd(POP, {RCX}) // right
    << cmd(POP, {RAX}); // left

  if (b.getNodeType()->isDouble()) {
    asm_file
      << cmd(MOVQ, {XMM0, none}, {RAX})
      << cmd(MOVQ, {XMM1, none}, {RCX})
      << cmd(arith_d.at(t), {XMM0, none}, {XMM1, none})
      << cmd(MOVQ, {RAX}, {XMM0, none})
      << cmd(PUSH, {RAX});
  } else {
    switch (b.getOp().getTokenType()) {
      case TokenType::Plus:
      case TokenType::Minus:
      case TokenType::Asterisk: {
        asm_file << cmd(arith_i.at(t), {RAX}, {RCX});
        break;
      }
      case TokenType::Div: {
        asm_file
          << cmd(XOR, {RDX}, {RDX})
          << cmd(CQO)
          << cmd(arith_i.at(t), {RCX});
        break;
      }
      case TokenType::Mod: {
        asm_file
          << cmd(XOR, {RDX}, {RDX})
          << cmd(CQO)
          << cmd(IDIV, {RCX})
          << cmd(MOV, {RAX}, {RDX});
        break;
      }
      case TokenType::ShiftLeft:
      case TokenType::Shl: {
        asm_file
          << cmd(SHL, {RAX}, {CL, Pref::b});
        break;
      }
      case TokenType::ShiftRight:
      case TokenType::Shr: {
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
  b.getSubLeft()->accept(*this);
  b.getSubRight()->accept(*this);
  auto t = b.getOp().getTokenType();
  asm_file << Comment("cmp operation");
  if (b.getSubRight()->getNodeType()->isDouble()) {
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
  switch (b.getOp().getTokenType()) {
    case TokenType::Xor: {
      b.getSubLeft()->accept(*this);
      b.getSubRight()->accept(*this);
      asm_file << Comment("xor operation");
      asm_file
        << cmd(POP, {RBX}) // righ
        << cmd(POP, {RAX}) // left
        << cmd(XOR, {RAX}, {RBX})
        << cmd(PUSH, {RAX});
      return;
    }
    case TokenType::And: {
      if (b.getNodeType()->isInt()) {
        b.getSubLeft()->accept(*this);
        b.getSubRight()->accept(*this);
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

        b.getSubLeft()->accept(*this);
        asm_file
          << Comment("and boolean operation")
          << cmd(POP, {RAX})
          << cmd(TEST, {RAX}, {RAX})
          << cmd(JZ, {Label(_false)});

        b.getSubRight()->accept(*this);

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
    case TokenType::Or: {
      if (b.getNodeType()->isInt()) {
        b.getSubLeft()->accept(*this);
        b.getSubRight()->accept(*this);
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

        b.getSubLeft()->accept(*this);
        asm_file
          << Comment("or boolean operation")
          << cmd(POP, {RAX})
          << cmd(TEST, {RAX}, {RAX})
          << cmd(JNZ, {Label(_true)});

        b.getSubRight()->accept(*this);

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
  switch (b.getOp().getTokenType()) {
    case TokenType::Plus:
    case TokenType::Minus:
    case TokenType::Slash:
    case TokenType::Asterisk:
    case TokenType::Div:
    case TokenType::Mod:
    case TokenType::ShiftLeft:
    case TokenType::Shl:
    case TokenType::ShiftRight:
    case TokenType::Shr: {
      visit_arithmetic(b);
      return;
    }
    case TokenType::Equals:
    case TokenType::NotEquals:
    case TokenType::StrictLess:
    case TokenType::StrictGreater:
    case TokenType::LessOrEquals:
    case TokenType::GreaterOrEquals: {
      visit_cmp(b);
      return;
    }
    case TokenType::And:
    case TokenType::Or:
    case TokenType::Xor: {
      visit_logical(b);
    }
    default:
      break;
  }
}

void AsmGenerator::visit(UnaryOperation& u) {
  switch (u.getOp().getTokenType()) {
    case TokenType::Minus: {
      u.getSubNode()->accept(*this);
      asm_file << Comment("unary minus");
      if (u.getNodeType()->isInt()) {
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
    case TokenType::Not: {
      u.getSubNode()->accept(*this);
      if (u.getNodeType()->isBool()) {
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
    case TokenType::At: {
      asm_file << Comment("@");
      visit_lvalue(*u.getSubNode());
      return;
    }
    case TokenType::Caret: {
      asm_file << Comment("^");
      if (need_lvalue) {
        need_lvalue = false;
        u.getSubNode()->accept(*this);
      } else {
        u.getSubNode()->accept(*this);
        asm_file
          << cmd(POP, {RAX})
          << cmd(PUSH, {adr(RAX)});
      }
      return;
    }
    default: {
      break;
    }
  }
}

void AsmGenerator::visit(Pointer& p) {
  bounds.front()->accept(*this);
  asm_file
    << Comment("compute pointer offset")
    << cmd(POP, {RAX}) // index
    << cmd(MOV, {RCX}, {p.typeBase->size()}) // sizeof 
    << cmd(IMUL, {RAX}, {RCX}); // index*sizeof
  if (bounds.size() == 1) {
    asm_file
      << Comment("save offset")
      << cmd(PUSH, {RAX}); // new index
    return;
  }
  asm_file
    << Comment("ppp")
    << Comment("change base")
    << Comment("^(ppp + 1)")
    << Comment("")
    << cmd(POP, {RCX}) // base
    << cmd(MOV, {RCX}, {adr(RCX, RAX, 1), none})
    << cmd(PUSH, {RCX}); // new base = *(base + index*sizeof)
  bounds.pop_front();
  p.typeBase->accept(*this);
}

void AsmGenerator::visit(StaticArray& s) {
  auto real_size = bounds.size();
  auto array_size = s.bounds.size();
  auto& real_bounds = bounds;
  auto& array_bounds = s.bounds;

  std::vector<uint64_t> coeff(array_size, 1);
  long int i_coeff = array_size - 2;
  for (auto it = s.bounds.rbegin(); i_coeff != -1 && it != s.bounds.rend();
       ++it, --i_coeff) {
    uint64_t len = it->second - it->first + 1;
    coeff.at(i_coeff) = coeff.at(i_coeff + 1) * len;
  }

  auto it = array_bounds.begin();
  i_coeff = 0;
  asm_file << cmd(PUSH, {(uint64_t) 0});
  for (uint64_t i = 0; i < real_size; ++i, ++i_coeff, ++it) {
    uint64_t begin = it->first;
    real_bounds.front()->accept(*this);
    real_bounds.pop_front();

    asm_file
      << Comment("compute offset for " + std::to_string(i))
      << cmd(POP, {RAX}) // index
      << cmd(MOV, {RCX}, {begin}) // low bound
      << cmd(SUB, {RAX}, {RCX}) // index - low bound
      << cmd(IMUL, {RAX}, {coeff.at(i_coeff)}) // (index - low bound)*len[-1]*len[-2]*..*len[i]
      << cmd(POP, {RCX}) // prev index
      << cmd(ADD, {RCX}, {RAX}) // prev index + %RAX
      << cmd(PUSH, {RCX});
  }
  asm_file
    << Comment("save static array offset")
    << cmd(POP, {RAX})
    << cmd(IMUL, {RAX}, {s.typeElem->size()}) // index*sizeof
    << cmd(PUSH, {RAX});

  if (real_size > array_size) {
    asm_file
      << Comment("change base")
      << cmd(POP, {RAX}) // index
      << cmd(POP, {RCX}) // base
      << cmd(ADD, {RCX}, {RAX}) // base + index
      << cmd(PUSH, {RCX}); // new_base
    s.typeElem->accept(*this);
  }
}

void AsmGenerator::visit(OpenArray& o) {
  bounds.front()->accept(*this);
  asm_file
    << Comment("compute open array offset")
    << cmd(POP, {RAX}) // index
    << cmd(MOV, {RCX}, {o.typeElem->size()})
    << cmd(IMUL, {RAX}, {RCX}); // new index = index*sizeof
  if (bounds.size() == 1) {
    asm_file
      << Comment("save open array offset")
      << cmd(PUSH, {RAX}); // new index
    return;
  }
  asm_file
    << Comment("continue compute offset")
    << cmd(POP, {RCX})
    << cmd(LEA, {RCX}, {adr(RCX, RAX, 1), none})
    << cmd(PUSH, {RCX}); // new base = base + index*sizeof
  bounds.pop_front();
  bounds.front()->getNodeType()->accept(*this);
}

void AsmGenerator::visit(ArrayAccess& a) {
  bool lvalue = need_lvalue;
  if (a.getSubNode()->getNodeType()->isPointer()) {
    need_lvalue = false;
    a.getSubNode()->accept(*this);
  } else {
    visit_lvalue(*a.getSubNode());
  }
  bounds = std::move(a.getListIndex());
  a.getSubNode()->getNodeType()->accept(*this);
  asm_file
    << Comment("push address base[index]")
    << cmd(POP, {RCX}) // index
    << cmd(POP, {RAX}) // base
    << cmd(LEA, {RAX}, {adr(RAX, RCX, 1), none})
    << cmd(PUSH, {RAX}); // base[index]
  if (!lvalue) {
    pushRvalue(a.getNodeType());
  }
}

void AsmGenerator::visit(Alias& a) { a.type->accept(*this); }

void AsmGenerator::visit(RecordAccess& r) {
  bool lvalue = need_lvalue;
  visit_lvalue(*r.record);
  auto record = r.record->getNodeType()->getRecord();
  auto offset = record->offset(r.field.getString());
  asm_file
    << Comment("record access")
    << cmd(POP, {R8}) // add_record
    << cmd(LEA, {R8}, {adr(R8, offset), none})
    << cmd(PUSH, {R8});
  if (!lvalue) {
    pushRvalue(r.getNodeType());
  }
}

void AsmGenerator::visit(Cast& c) {
  if (need_lvalue) {
    visit_lvalue(*c.expr);
  } else {
    c.expr->accept(*this);
  }

  if (c.expr->getNodeType()->isDouble() && c.getNodeType()->isInt()) {
    asm_file
      << Comment("double to int")
      << cmd(POP, {RAX})
      << cmd(MOVQ, {XMM0, none}, {RAX})
      << cmd(CVTSD2SI, {RAX}, {XMM0, none})
      << cmd(PUSH, {RAX});
  } else if (c.expr->getNodeType()->isInt() && c.getNodeType()->isDouble()) {
    asm_file
      << Comment("int to double")
      << cmd(POP, {RAX})
      << cmd(CVTSI2SD, {XMM0, none}, {RAX})
      << cmd(MOVQ, {RAX}, {XMM0, none})
      << cmd(PUSH, {RAX});
  }
}

void AsmGenerator::visit(AssignmentStmt& a) {
  a.getSubRight()->accept(*this);
  visit_lvalue(*a.getSubLeft());
  if (a.getSubLeft()->getNodeType()->isTrivial()) {
    auto t = a.getOp().getTokenType();
    switch (t) {
      case TokenType::AssignmentWithPlus:
      case TokenType::AssignmentWithMinus:
      case TokenType::AssignmentWithAsterisk:
      case TokenType::AssignmentWithSlash: {
        asm_file
          << Comment(toString(t))
          << cmd(POP, {RAX}) // left - address
          << cmd(POP, {R8}) // right
          << cmd(MOV, {R9}, {adr(RAX)}); // *left
        if (a.getSubLeft()->getNodeType()->isDouble()) {
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
    asm_file
      << Comment("assignment trivial")
      << cmd(POP, {RAX}) // left
      << cmd(POP, {adr(RAX)});
    return;
  }
  // count 8 byte
  uint64_t len = a.getSubLeft()->getNodeType()->size() / 8;
  auto _start = getLabel();
  auto _end = getLabel();
  asm_file
    << Comment("assignment with copy")
    << cmd(POP, {RDI}) // adr left - назначение
    << cmd(POP, {RSI}) // value right - источник
    << cmd(MOV, {RCX}, {len})
    << cmd(Label(_start))
    << cmd(MOV, {adr(RDI)}, {RSI})
    << cmd(DEC, {RCX})
    << cmd(JZ, {Label(_end)})
    << cmd(POP, {RSI})
    << cmd(ADD, {RDI}, {(uint64_t) 8})
    << cmd(JMP, {Label(_start)})
    << cmd(Label(_end));
}

void AsmGenerator::visit(Read&) {}

void AsmGenerator::visit(Trunc&) {
  syscall_params.front()->accept(*this);
  asm_file
    << cmd(POP, {RAX})
    << cmd(MOVQ, {XMM1, none}, {RAX, none})
    << cmd(ROUNDSD, {XMM0, none}, {XMM1, none}, {(uint64_t)11, none})
    << cmd(CVTSD2SI, {RAX}, {XMM0, none})
    << cmd(PUSH, {RAX});
}

void AsmGenerator::visit(Round&) {
  syscall_params.front()->accept(*this);
  asm_file
    << cmd(POP, {RAX})
    << cmd(MOVQ, {XMM1, none}, {RAX, none})
    << cmd(ROUNDSD, {XMM0, none}, {XMM1, none}, {(uint64_t)8, none})
    << cmd(CVTSD2SI, {RAX}, {XMM0, none})
    << cmd(PUSH, {RAX});
}

void AsmGenerator::visit(Succ&) {
  syscall_params.front()->accept(*this);
  asm_file << cmd(INC, {adr(RSP)});
}

void AsmGenerator::visit(Prev&) {
  syscall_params.front()->accept(*this);
  asm_file << cmd(DEC, {adr(RSP)});
}

void AsmGenerator::visit(Chr&) {
  syscall_params.front()->accept(*this);
}

void AsmGenerator::visit(Ord&) {
  syscall_params.front()->accept(*this);
}

void AsmGenerator::visit(High&) {
  auto& param = syscall_params.front();
  if (param->getNodeType()->isOpenArray()) {
    uint64_t offset = stackTable.findVar(param->getVarName())->getOffset();
    asm_file
      << Comment("High open array")
      << cmd(PUSH, {adr(RBP, offset + 8)});
  } else {
    auto array = param->getNodeType()->getStaticArray();
    asm_file << cmd(PUSH, {array->bounds.front().second});
  }
}

void AsmGenerator::visit(Low&) {
  auto& param = syscall_params.front();
  if (param->getNodeType()->isOpenArray()) {
    asm_file << cmd(PUSH, {(uint64_t)0});
  } else {
    auto array = param->getNodeType()->getStaticArray();
    asm_file << cmd(PUSH, {array->bounds.front().first});
  }
}

void AsmGenerator::visit(Exit& e) {
  if (!e.returnType->isVoid()) {
    AssignmentStmt c( // result := expr;
      Token(-1, -1, TokenType::Assignment),
      std::make_unique<Variable>(
        Token(-1, -1, TokenType::String,
              e.assignmentVar->getSymbolName(), e.assignmentVar->getSymbolName()),
        e.returnType),
      std::move(syscall_params.front())
    );
    visit(c);
  }
  asm_file
    << cmd(MOV, {RSP}, {RBP})
    << cmd(POP, {RBP})
    << cmd(RET, {sizeParam, none});
}

void AsmGenerator::visit(Write& w) {
  auto params = std::move(syscall_params);
  for (auto& e : params) {
    e->accept(*this);
    if (e->getNodeType()->isString()) {
      asm_file
        << Comment("printf string")
        << cmd(POP, {RAX})
        << cmd(MOV, {RDI}, {RAX})
        << cmd(XOR, {RAX}, {RAX})
        << cmd(CALL, {Printf});
    } else if (e->getNodeType()->isInt() || e->getNodeType()->isPointer()) {
      asm_file
        << Comment("printf int")
        << cmd(MOV, {RDI}, {Label(label_fmt_int)})
        << cmd(POP, {RSI})
        << cmd(XOR, {RAX}, {RAX})
        << cmd(CALL, {Printf});
    } else if (e->getNodeType()->isDouble()) {
      asm_file
        << Comment("printf double")
        << cmd(MOV, {RDI}, {Label(label_fmt_double)})
        << cmd(POP, {RAX})
        << cmd(MOVQ, {XMM0, none}, {RAX})
        << cmd(MOV, {RAX}, {(uint64_t) 1})
        << cmd(CALL, {Printf});
    } else if (e->getNodeType()->isChar()) {
      asm_file
        << Comment("printf char")
        << cmd(MOV, {RDI}, {Label(label_fmt_char)})
        << cmd(POP, {RSI})
        << cmd(XOR, {RAX}, {RAX})
        << cmd(CALL, {Printf});
    }
  }
  if (w.isnewLine) {
    asm_file
      << Comment("printf new line")
      << cmd(MOV, {RDI}, {Label(label_fmt_new_line)})
      << cmd(XOR, {RAX}, {RAX})
      << cmd(CALL, {Printf});
  }
}

void AsmGenerator::visit(FunctionCallStmt& f) {
  isSkipResult = true;
  f.getFunctionCall()->accept(*this);
  isSkipResult = false;
}

void AsmGenerator::visit(FunctionCall& f) {
  bool skip = isSkipResult;
  isSkipResult = false;
  asm_file << Comment("function call");
  if (f.getSubNode()->getEmbeddedFunction() != nullptr) {
    syscall_params = std::move(f.getListParam());
    f.getSubNode()->getEmbeddedFunction()->accept(*this);
    return;
  } else {
    auto s = f.getSubNode()->getNodeType()->getSignature();
    uint64_t sizeReturn = 0;
    if (!s->isProcedure()) {
      sizeReturn = s->returnType->size();
      asm_file
        << Comment("allocate return type")
        << cmd(SUB, {RSP}, {sizeReturn}); // allocate return type
    }
    // arguments push
    auto iterParams = s->paramsList.begin();
    for (auto iterArgs = f.getListParam().begin();
         iterArgs != f.getListParam().end(); ++iterArgs, ++iterParams) {
      if ((*iterParams)->getVarType()->isOpenArray()) {
        auto array = (*iterArgs)->getNodeType()->getStaticArray();
        asm_file
          << Comment("open array argument")
          << cmd(PUSH, {array->bounds.front().second}); // high
        visit_lvalue(**iterArgs); // address
        continue;
      }
      asm_file << Comment("argument");
      if ((*iterParams)->spec == ParamSpec::NotSpec) {
        (*iterArgs)->accept(*this);
      } else {
        visit_lvalue(**iterArgs);
      }
    }
    if (stackTable.isFunction(f.getSubNode()->getNodeType()->getSymbolName())) {
      visit_lvalue(*f.getSubNode());
    } else {
      f.getSubNode()->accept(*this);
    }
    asm_file
      << Comment("call function")
      << cmd(POP, {RAX}) // address
      << cmd(CALL, {RAX});
    if (skip) {
      asm_file
        << Comment("skip result")
        << cmd(ADD, {RSP}, {sizeReturn});
    }
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
    << cmd(POP, {adr(R14)}) // *add_var = low
    << cmd(PUSH, {R13}) // high
    << cmd(PUSH, {R14}) // add_var
    << cmd(Label(_body))
    << cmd(POP, {R14}) // add_var
    << cmd(POP, {R13}) // high
    << cmd(MOV, {RAX}, {adr(R14)})
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
    << cmd(f.direct ? INC : DEC, {adr(R14)})
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


// Global Decl

void AsmGlobalDecl::visit(GlobalVar& v) {
  v.label = getLabelName(v.getSymbolName());
  a
    << cmd(bss)
    << Label(v.label) << ": ";
  v.getVarType()->accept(*this);
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