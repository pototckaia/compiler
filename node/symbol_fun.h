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
  ForwardFunction(const Token& t, ptr_Sign f);

  // todo remove
  bool isEmbedded() const override { return false; }
	bool isForward() const override { return true; }
	void accept(Visitor& v) override;
	// todo remove virtual
  std::string getLabel() override { return function->getLabel(); }
  void setLabel(const std::string& s) override { function->setLabel(s); }
  ptr_Sign& getSignature() override { return function->getSignature(); }
	ptr_Stmt& getBody() override { return function->getBody(); }
	Tables& getTable() override { return function->getTable(); }

 void setFunction(ptr_Fun f) { function = std::move(f); }
 auto& getFunction() { return  function; }

 private:
	ptr_Fun function;
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