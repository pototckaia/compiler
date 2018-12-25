#pragma once

#include "astnode.h"
#include "node.h"
#include "symbol_type.h"
#include "table_symbol.h"

class ForwardFunction : public SymFun {
 public:
  using SymFun::SymFun;
  ForwardFunction(const tok::ptr_Token& t, std::shared_ptr<FunctionSignature> f)
    : SymFun(t, std::move(f)) {}

  void accept(pr::Visitor& v) override;
  bool isEmbedded() const override { return false; }
  void setFunction(const std::shared_ptr<SymFun>& f)  { function = f; }
  bool isForward() const override { return true; }

  std::shared_ptr<SymFun> function;
};

class Function : public SymFun {
 public:
  using SymFun::SymFun;
  Function(const tok::ptr_Token& t, std::shared_ptr<FunctionSignature> f,
          pr::ptr_Stmt p, Tables l)
    : SymFun(t, std::move(f)), localVar(std::move(l)), body(std::move(p)) {}

  ~Function() override;

  Tables localVar;
  pr::ptr_Stmt body;
  void accept(pr::Visitor& v) override;
  bool isEmbedded() const override { return false; }
};

class MainFunction : public SymFun {
 public:
  using SymFun::SymFun;
  MainFunction(Tables t, pr::ptr_Stmt b)
    : SymFun("Main block"), body(std::move(b)), decl(std::move(t)) {}

  void accept(pr::Visitor& v) override;
  bool isEmbedded() const override { return false; }

  pr::ptr_Stmt body;
  Tables decl;
};

class Write : public SymFun {
 public:
  Write(bool newLine = false);
  void accept(pr::Visitor& v) override;
};

class Read: public SymFun {
 public:
  Read(bool newLine = false);
  void accept(pr::Visitor& v) override;
};

class Trunc : public SymFun {
 public:
  Trunc();
  void accept(pr::Visitor& v) override;
};

class Round : public SymFun {
 public:
  Round();
  void accept(pr::Visitor& v) override;
};

class Succ : public SymFun {
 public:
  Succ();
  void accept(pr::Visitor& v) override;
};

class Prev : public SymFun {
 public:
  Prev();
  void accept(pr::Visitor& v) override;
};

class Chr : public SymFun {
 public:
  Chr();
  void accept(pr::Visitor& v) override;
};

class Ord : public SymFun {
 public:
  Ord();
  void accept(pr::Visitor& v) override;
};

class High : public SymFun {
 public:
  High();
  void accept(pr::Visitor& v) override;
};

class Low : public SymFun {
 public:
  Low();
  void accept(pr::Visitor& v) override;
};

class Exit : public SymFun {
 public:
  Exit(ptr_Type returnType);
  void accept(pr::Visitor& v) override;
  ptr_Type returnType;
};