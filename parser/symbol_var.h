#pragma once

#include "astnode.h"
#include "symbol_type.h"

class LocalVar : public SymVar {
 public:
  using SymVar::SymVar;
  void accept(Visitor& v) override;

  uint64_t offset = 0;
  void setOffset(uint64_t s) override { offset = s; }
  uint64_t getOffset() override { return offset; }
};

class GlobalVar : public SymVar {
 public:
  using SymVar::SymVar;
  void accept(Visitor& v) override;

  std::string label;
  void setOffset(uint64_t s) override {}
  uint64_t getOffset() override { return 0; }
};

enum class ParamSpec {
  NotSpec,
  Var,
  Const,
  Out
};

std::string toString(ParamSpec);

class ParamVar : public SymVar {
 public:
  using SymVar::SymVar;

  ParamSpec spec = ParamSpec::NotSpec;
  void accept(Visitor& v) override;
  bool equals(ParamVar&) const;
  uint64_t size() const override;

  uint64_t offset = 0;
  void setOffset(uint64_t s) override { offset = s; }
  uint64_t getOffset() override { return offset; }
};

class Const : public SymVar {
 public:
  using SymVar::SymVar;
  ptr_Expr value;
  void accept(Visitor& v) override;
};
