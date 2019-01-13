#include "opcode.h"

#include <string>
#include <map>
#include <iomanip>

std::string getLabel() {
  static int count = 0;
  return "label_" + std::to_string(count++);
}

std::string getLabelName(const std::string& s) {
  return "___" + s;
}

std::string getStrName() {
  static int count = 0;
  return "str_" + std::to_string(count++);
}

const std::map<Register, std::string> mapRegister = {
  {AL,  "al"},
  {AH,  "ah"},
  {CL, "cl"},
  {RAX, "rax"},
  {RCX, "rcx"},
  {RDX, "rdx"},
  {RBX, "rbx"},
  {RSP, "rsp"},
  {RBP, "rbp"},
  {RSI, "rsi"},
  {RDI, "rdi"},
  {R8, "r8"},
  {R9, "r9"},
  {R10, "r10"},
  {R11, "r11"},
  {R12, "r12"},
  {R13, "r13"},
  {R14, "r14"},
  {R15, "r15"},
  {XMM0, "xmm0"},
  {XMM1, "xmm1"},
};

std::ostream& operator <<(std::ostream &os, const Register& c) {
  os << mapRegister.at(c);
  return os;
}

const std::map<Pref, std::string> mapPref = {
  {b, "BYTE"},
  {w, "WORD"},
  {d, "DWORD"},
  {q, "QWORD"},
  {none, ""},
  {double64, "__float64__"},
};

std::ostream& operator <<(std::ostream &os, const Pref& c) {
  os << mapPref.at(c);
  return os;
}

const std::map<SysCall, std::string> mapSysCall = {
  {Printf, "printf"},
  {Scanf, "scanf"}
};

std::ostream& operator <<(std::ostream &os, const SysCall& c) {
  os << mapSysCall.at(c);
  return os;
}

const std::map<Instruction, std::string> mapInstruction = {
  {DB, "db"},
  {DQ, "dq"},
  {RESB, "resb"},
  {RESQ, "resq"},
  {TIMES, "times"},

  {MOV, "mov"},
  {MOVQ, "movq"},
  {NOT, "not"},
  {XOR, "xor"},
  {AND, "and"},
  {OR, "or"},
  {PUSH, "push"},
  {POP, "pop"},
  {CALL, "call"},
  {LEA, "lea"},
  {RET, "ret"},
  {ADD, "add"},
  {SUB, "sub"},
  {IMUL, "imul"},
  {IDIV, "idiv"},
  {CQO, "cqo"},
  {SETE, "sete"},
  {SETNE, "setne"},
  {SETL, "setl"},
  {SETG, "setg"},
  {SETGE, "setge"},
  {SETLE, "setle"},
  {SETA, "seta"},
  {SETB, "setb"},
  {SETAE, "setae"},
  {SETBE, "setbe"},
  {CMP, "cmp"},
  {CVTSI2SD, "cvtsi2sd"},
  {CVTSD2SI, "cvtsd2si"},
  {ADDSD, "addsd"},
  {SUBSD, "subsd"},
  {DIVSD, "divsd"},
  {MULSD, "mulsd"},
  {MOVSD, "movsd"},
  {COMISD, "comisd"},
  {XORPD, "xorpd"},
  {NEG, "neg"},
  {SHL, "shl"},
  {SHR, "shr"},
  {JZ, "jz"},
  {JNZ, "jnz"},
  {JMP, "jmp"},
  {JE, "je"},
  {JGE, "jge"},
  {JLE, "jle"},
  {JG, "jg"},
  {JL, "jl"},
  {TEST, "test"},
  {INC, "inc"},
  {DEC, "dec"},
};

std::ostream& operator <<(std::ostream &os, const Instruction& c) {
  os << mapInstruction.at(c);
  return os;
}

const std::map<Section, std::string> mapSection = {
  {data, ".data"},
  {bss, ".bss"},
  {code, ".text"},
};

std::ostream& operator <<(std::ostream& os, const Section& s) {
  os << mapSection.at(s);
  return os;
}

Label::Label(std::string s) : s(s) {}
std::ostream& operator <<(std::ostream &os, const Label& c) {
  os << c.s;
  return os;
}

EffectiveAddress::EffectiveAddress(Register b) : isOnlyBase(true) { base << b; }
EffectiveAddress::EffectiveAddress(Label l) : isOnlyBase(true) { base << l; }
EffectiveAddress::EffectiveAddress(Register b, Register i, uint64_t s)
  : index(i), scala(s) { base << b; }
EffectiveAddress::EffectiveAddress(Register b, uint64_t offset, bool isMinus)
  : isOnlyBase(true), isOffset(true), offset(offset), isMinus(isMinus) {
  base << b;
}

std::ostream& operator <<(std::ostream &os, const EffectiveAddress& c) {
  os << "[" << c.base.str();
  char opr = c.isMinus ? '-' : '+';
  if (!c.isOnlyBase) {
    os << opr << c.index << "*" << c.scala;
  }
  if (c.isOffset) {
    os << opr << c.offset;
  }
  os << "]";
  return os;
}

Operand::Operand(uint64_t u, Pref p) { s << p << " " << u; }
Operand::Operand(std::string u, Pref p) { s << p << " \"" << u << "\""; }
Operand::Operand(long double u, Pref p) {
  s << std::showpoint;
  if (p == Pref::double64)
    s << p << "("  << std::to_string(u) << ")";
  else
    s << p << " " << std::to_string(u);
}
Operand::Operand(Label l) { s << l; }
Operand::Operand(EffectiveAddress e, Pref p) { s << p << " " << e; }
Operand::Operand(Register r, Pref p) { s << p << " " << r; }
Operand::Operand(SysCall c) { s << c; }

std::ostream& operator <<(std::ostream &os, const Operand& o) {
  os << o.s.str();
  return os;
}

Command::Command(SysCall c) { s << "extern " << c << "\n"; }
Command::Command(Section o) { s << "\nsection " << o << "\n"; }
Command::Command(Label l) { s << l << ":\n"; }
Command::Command(Label l, bool isGlobal) { s << "global " << l << "\n"; }
Command::Command(Instruction i) { s << "\t" << i << "\n";}
Command::Command(Instruction i, Operand o) { s << "\t" << i << " " << o << "\n"; }
Command::Command(Instruction i, Operand o1, Operand o2) { s << "\t" << i << " " << o1 << ", " << o2 << "\n"; }

std::ostream& operator <<(std::ostream& os, const Command& c) {
  os << c.s.str();
  return os;
}

Comment::Comment(std::string ss) : s(std::move(ss)) {}
std::ostream& operator <<(std::ostream& os, const Comment& c) {
  os << "; " << c.s << "\n";
  return os;
}