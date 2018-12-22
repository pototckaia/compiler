#pragma once

#include "astnode.h"
#include "symbol_type.h"

class LocalVar : public SymVar {
 public:
  using SymVar::SymVar;
  void accept(pr::Visitor& v) override;
};

class GlobalVar : public SymVar {
 public:
  using SymVar::SymVar;
  void accept(pr::Visitor& v) override;
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
  void accept(pr::Visitor& v) override;
  bool equals(ParamVar&) const;
};



class Const : public SymVar {
 public:
  using SymVar::SymVar;
  pr::ptr_Expr value;
  void accept(pr::Visitor& v) override;
};
