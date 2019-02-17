#pragma once

#include "astnode.h"
#include "node.h"
#include "symbol_type.h"
#include "table_symbol.h"

class Function : public SymFun {
 public:
  //using SymFun::SymFun;
  Function(const Token&, ptr_Sign);
  Function(const Token&, ptr_Sign, ptr_Stmt, Tables);
  ~Function() override;

  // todo remove
  bool isEmbedded() const override { return false; }
  // todo remove virtual
  ptr_Stmt& getBody() override { return body; }
  Tables& getTable() override  { return localVar; }
	void accept(Visitor& v) override;

 private:
	Tables localVar;
	ptr_Stmt body;
};

class ForwardFunction : public Function {
 public:
	// todo move to cpp
  ForwardFunction(const Token& t, std::shared_ptr<FunctionSignature> f)
    : Function(t, std::move(f)) {}

  void accept(Visitor& v) override;
  bool isEmbedded() const override { return false; }
  bool isForward() const override { return true; }

  std::string getLabel() override { return function->getLabel(); }
  void setLabel(const std::string& s) override { function->setLabel(s); }

	// todo move to cpp setter to function
  std::shared_ptr<SymFun> function;
  ptr_Sign& getSignature() override;
  ptr_Stmt& getBody() override;
  Tables& getTable() override;
};

class MainFunction : public SymFun {
 public:
  using SymFun::SymFun;
	// todo move to cpp
  MainFunction(Tables t, ptr_Stmt b)
    : SymFun("Main block"), body(std::move(b)), decl(std::move(t)) {}

  void accept(Visitor& v) override;
  bool isEmbedded() const override { return false; }
// todo privat
  ptr_Stmt body;
  Tables decl;
};

class Write : public SymFun {
 public:
  Write(bool newLine = false);
  void accept(Visitor& v) override;
  bool isnewLine;
};

class Read: public SymFun {
 public:
  Read(bool newLine = false);
  void accept(Visitor& v) override;
};

class Trunc : public SymFun {
 public:
  Trunc();
  void accept(Visitor& v) override;
};

class Round : public SymFun {
 public:
  Round();
  void accept(Visitor& v) override;
};

class Succ : public SymFun {
 public:
  Succ();
  void accept(Visitor& v) override;
};

class Prev : public SymFun {
 public:
  Prev();
  void accept(Visitor& v) override;
};

class Chr : public SymFun {
 public:
  Chr();
  void accept(Visitor& v) override;
};

class Ord : public SymFun {
 public:
  Ord();
  void accept(Visitor& v) override;
};

class High : public SymFun {
 public:
  High();
  void accept(Visitor& v) override;
};

class Low : public SymFun {
 public:
  Low();
  void accept(Visitor& v) override;
};

class Exit : public SymFun {
 public:
  Exit(ptr_Type returnType);
  Exit(ptr_Type returnType, std::shared_ptr<ParamVar> var);

  void accept(Visitor& v) override;
  // todo private
  ptr_Type returnType;
  std::shared_ptr<ParamVar> assignmentVar = nullptr;
};