#pragma once

#include "astnode.h"
#include "symbol_type.h"

class LocalVar : public SymVar {
 public:
  using SymVar::SymVar;
  void accept(Visitor& v) override;

  uint64_t offset = 0;
};

class GlobalVar : public SymVar {
 public:
  using SymVar::SymVar;
  void accept(Visitor& v) override;

  std::string label;
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

  uint64_t offset = 0;
};

class Const : public SymVar {
 public:
  using SymVar::SymVar;
  ptr_Expr value;
  void accept(Visitor& v) override;
};
