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
};

class MainFunction : public SymFun {
 public:
  using SymFun::SymFun;
  MainFunction(Tables t, pr::ptr_Stmt b)
    : SymFun("Main block"), body(std::move(b)), decl(std::move(t)) {}

  void accept(pr::Visitor& v) override;

  pr::ptr_Stmt body;
  Tables decl;
};

class StaticCast : public SymFun {
 public:
  using SymFun::SymFun;
  StaticCast(ptr_Type to) : typeConvert(std::move(to)) {}

  void accept(pr::Visitor& v) override;
  ptr_Type typeConvert;
};

class Write : public SymFun {
 public:
  Write(bool newLine = false) {
    if (newLine)
      name =  "write";
    else
      name = "writeln";
  }
  void accept(pr::Visitor& v) override;
};

class Read: public SymFun {
 public:
  Read(bool newLine = false) {
    if (newLine)
      name =  "read";
    else
      name = "readln";
  }
  void accept(pr::Visitor& v) override;
};

class Trunc : public SymFun {
 public:
  Trunc() : SymFun("trunc") {};
  void accept(pr::Visitor& v) override;
};

class Round : public SymFun {
 public:
  Round() : SymFun("round") {}
  void accept(pr::Visitor& v) override;
};

class Succ : public SymFun {
 public:
  Succ(): SymFun("succ") {}
  void accept(pr::Visitor& v) override;
};

class Prev : public SymFun {
 public:
  Prev() : SymFun("prev") {}
  void accept(pr::Visitor& v) override;
};

class Chr : public SymFun {
 public:
  Chr() : SymFun("chr") {}
  void accept(pr::Visitor& v) override;
};

class Ord : public SymFun {
 public:
  Ord() : SymFun("ord") {}
  void accept(pr::Visitor& v) override;
};

class High : public SymFun {
 public:
  High() : SymFun("high") {}
  void accept(pr::Visitor& v) override;
};

class Low : public SymFun {
 public:
  Low() : SymFun("low") {}
  void accept(pr::Visitor& v) override;
};