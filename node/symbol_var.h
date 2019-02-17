#pragma once

#include "astnode.h"
#include "symbol_type.h"

class LocalVar : public SymVar {
 public:
	LocalVar(const Token&, ptr_Type);
  void accept(Visitor& v) override;
  void setOffset(uint64_t s) override { offset = s; }
	uint64_t getOffset() override { return offset; }

 private:
	uint64_t offset = 0;
};

class GlobalVar : public SymVar {
 public:
	GlobalVar(const Token&, ptr_Type);
  void accept(Visitor& v) override;
  void setOffset(uint64_t s) override {}
	uint64_t getOffset() override { return 0; }
	std::string getLabel() { return label; }
	void setLabel(std::string s) { label = s; }

 private:
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
	ParamVar(ptr_Type, ParamSpec);
	ParamVar(const Token&, ptr_Type, ParamSpec);
	ParamVar(std::string, ptr_Type, ParamSpec);

	void accept(Visitor& v) override;
	bool equals(ParamVar&) const;
	uint64_t size() const override;
	void setOffset(uint64_t s) override { offset = s; }
	uint64_t getOffset() override { return offset; }
	ParamSpec getSpec() { return spec; }

 private:
  ParamSpec spec = ParamSpec::NotSpec;
  uint64_t offset = 0;
};

// todo do const
class Const : public SymVar {
 public:
  using SymVar::SymVar;
  ptr_Expr value;
  void accept(Visitor& v) override;
};
