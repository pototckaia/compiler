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

  // todo remove virtual
  ptr_Stmt& getBody() override { return body; }
  Tables& getTable() override  { return localVar; }
	void accept(Visitor& v) override;

 private:
	Tables localVar;
	ptr_Stmt body;
};

class ForwardFunction : public SymFun {
 public:
  ForwardFunction(const Token& t, ptr_Sign f);

	bool isForward() const override;
	void accept(Visitor& v) override;
	// todo remove virtual
  std::string getLabel() override { return function->getLabel(); }
  void setLabel(const std::string& s) override { function->setLabel(s); }

//  ptr_Sign& getSignature() override { return function->getSignature(); }
	ptr_Stmt& getBody() override { return function->getBody(); }
	Tables& getTable() override { return function->getTable(); }

 void setFunction(ptr_Fun f) { function = std::move(f); }
 auto& getFunction() { return  function; }

 private:
	ptr_Fun function;
};

class BuildInFun : public SymFun {
	using SymFun::SymFun;
	bool isBuildIn() const override;
};

class MainFunction : public SymFun {
 public:
  MainFunction(Tables t, ptr_Stmt b);

  void accept(Visitor& v) override;
  // todo remove
  bool isBuildIn() const override { return false; }
  // todo remove virtual
	ptr_Stmt& getBody() override { return body; }
	Tables& getTable() { return decl; }

 private:
  ptr_Stmt body;
  Tables decl;
};

class Write : public BuildInFun {
 public:
  Write(bool newLine = false);
	bool isNewLine();
	void accept(Visitor& v) override;
};

class Read: public BuildInFun {
 public:
  Read(bool newLine = false);
  void accept(Visitor& v) override;
};

class Trunc : public BuildInFun {
 public:
  Trunc();
  void accept(Visitor& v) override;
};

class Round : public BuildInFun {
 public:
  Round();
  void accept(Visitor& v) override;
};

class Succ : public BuildInFun {
 public:
  Succ();
  void accept(Visitor& v) override;
};

class Prev : public BuildInFun {
 public:
  Prev();
  void accept(Visitor& v) override;
};

class Chr : public BuildInFun {
 public:
  Chr();
  void accept(Visitor& v) override;
};

class Ord : public BuildInFun {
 public:
  Ord();
  void accept(Visitor& v) override;
};

class High : public BuildInFun {
 public:
  High();
  void accept(Visitor& v) override;
};

class Low : public BuildInFun {
 public:
  Low();
  void accept(Visitor& v) override;
};

class Exit : public BuildInFun {
 public:
  Exit(ptr_Type returnType);
  Exit(ptr_Type returnType, std::shared_ptr<ParamVar> var);

	auto& getReturnType() { return returnType; }
	auto& getVar() { return assignmentVar; }
	void accept(Visitor& v) override;


 private:
  ptr_Type returnType;
  std::shared_ptr<ParamVar> assignmentVar = nullptr;
};