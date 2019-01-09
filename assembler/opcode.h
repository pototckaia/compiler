#pragma once

#include <stdint-gcc.h>
#include <ostream>
#include <sstream>


std::string getLabel();
std::string getStrName();
std::string getLabelName(const std::string&);

enum Register {
  AL, AH, CL,
  RAX, RCX, RDX, RBX,
  RSP, RBP,
  RSI, RDI,
  R8, R9, R10, R11, R12, R13, R14, R15,
  XMM0, XMM1
};

enum Pref {
  b,// (от byte) — операнды размером в 1 байт
  w,//(от word)— операнды размером в 1 слово (2 байта)
  d,//(от long) — операнды размером в 4 байта
  q,// (от quad) — операнды размером в 8 байт
  none,
  double64,
};

enum SysCall {
  Printf,
  Scanf,
};

enum Instruction {
  DB,
  DQ,
  RESB,
  RESQ,
  TIMES,

  MOV,
  MOVQ,
  NOT,
  XOR,
  AND,
  OR,
  PUSH,
  POP,
  CALL,
  LEA,
  RET,
  ADD,
  SUB,
  IMUL,
  IDIV,
  CQO,
  INC,
  DEC,

  SETE,
  SETNE,
  SETL,
  SETG,
  SETGE,
  SETLE,
  SETA,
  SETB,
  SETAE,
  SETBE,

  CMP,
  CVTSI2SD,
  CVTSD2SI,
  ADDSD,
  SUBSD,
  DIVSD,
  MULSD,
  MOVSD,
  COMISD,
  XORPD,
  NEG,
  SHL,
  SHR,
  JZ,
  JNZ,
  JMP,
  JE,
  JGE,
  JLE,
  JG,
  JL,
  TEST,
};

enum Section {
  data,
  bss,
  code,
};

std::ostream& operator <<(std::ostream &os, const Register&);
std::ostream& operator <<(std::ostream &os, const Pref&);
std::ostream& operator <<(std::ostream &os, const SysCall&);
std::ostream& operator <<(std::ostream &os, const Instruction&);
std::ostream& operator <<(std::ostream &os, const Section&);

class Label {
 public:
  Label(std::string);
  friend std::ostream& operator <<(std::ostream &os, const Label&);

 private:
  std::string s;
};

class EffectiveAddress {
 public:
  EffectiveAddress(Register);
  EffectiveAddress(Label);
  EffectiveAddress(Register, int offset);
  EffectiveAddress(Register, Register, uint64_t);
  friend std::ostream& operator <<(std::ostream &os, const EffectiveAddress&);

 private:
  bool isOnlyBase = false;
  bool isOffset = false;

  std::stringstream base;
  Register index;
  uint64_t scala;
  int offset;
};

class Operand {
 public:
  Operand() = default;
  Operand(uint64_t, Pref = q);
  Operand(long double, Pref = q);
  Operand(std::string, Pref = q);
  Operand(Label);
  Operand(EffectiveAddress, Pref = q);
  Operand(Register, Pref = q);
  Operand(SysCall);
  friend std::ostream& operator <<(std::ostream &os, const Operand&);

  Operand(const Operand& o) { s.str(std::string()); s << o.s.str(); }
  Operand& operator=(const Operand& o) {
    s.str(std::string());
    s << o.s.str();
    return *this;
  }

 private:
  std::stringstream s;
};

class Command {
 public:
  Command(SysCall);
  Command(Section);
  Command(Label);
  Command(Label, bool isGlobal);
  Command(Instruction);
  Command(Instruction, Operand);
  Command(Instruction, Operand, Operand);
  friend std::ostream& operator <<(std::ostream& os, const Command&);

 private:
  std::stringstream s;
};

class Comment {
 public:
  Comment(std::string ss);
  friend std::ostream& operator <<(std::ostream& os, const Comment&);

 private:
  std::string s;
};