#pragma once

#include <string>
#include <map>
#include <list>
#include <memory>
#include <vector>

#include "astnode.h"
#include "token.h"

class Symbol;
class SymType;
class SymVar;
class Const;
class SymFun;
class ForwardType;
class ForwardFunction;

using ptr_Symbol = std::shared_ptr<Symbol>;
using ListSymbol = std::list<ptr_Symbol>;

class TableSymbol {
 public:
  TableSymbol() = default;

  void insert(const ptr_Symbol&);
  bool checkContain(const std::string&);
  ptr_Symbol find(const std::string&) const;
  void replace(const ptr_Symbol&);

  auto begin() { return table.begin(); };
  auto end() { return table.end(); }

 private:
  std::map<std::string, ptr_Symbol> table;
};

class Tables {
 public:
  bool checkContain(const std::string&);
  void insertCheck(const std::shared_ptr<Symbol>&);

  void insert(const std::shared_ptr<ForwardType>&);
  void insert(const std::shared_ptr<ForwardFunction>&);

  void resolveForwardType();
  TableSymbol tableType;
  TableSymbol tableVariable;
  TableSymbol tableConst;

  TableSymbol tableFunction;
 private:

  std::list<std::shared_ptr<ForwardType>> forwardType;
  std::list<std::shared_ptr<ForwardFunction>> forwardFunction;
};

Tables createGlobalTable();

class StackTable {
 public:
  StackTable() = default;
  StackTable(const Tables& global);

  void push(const Tables& t) { stack.push_back(t); }
  void pop() { stack.pop_back(); }
  Tables& top() { return stack.back(); }
  bool isEmpty() { return stack.empty(); }

  bool checkContain(const std::string& n);

  bool isType(const std::string& n);
  ptr_Symbol findType(const std::string&);

  std::list<Tables> stack;
};



class Symbol : public pr::ASTNode {
 public:
  Symbol();
  Symbol(const std::string&);
  Symbol(const tok::ptr_Token&);

  void setDeclPoint(const tok::ptr_Token& t) {
    line = t->getLine();
    column = t->getColumn();
  }

  std::string name;
  int line, column;
  bool isForward = false;
};

class SymType : public Symbol {
 public:
  using Symbol::Symbol;
};

class Int : public SymType {
 public:
  Int() : SymType("integer") {}
  void accept(pr::Visitor& v) override;
};

class Double : public SymType {
 public:
  Double() : SymType("double") {}
  void accept(pr::Visitor& v) override;
};

class Char : public SymType {
 public:
  Char() : SymType("char") {}
  void accept(pr::Visitor& v) override;
};

class Boolean : public SymType {
 public:
  Boolean() : SymType("boolean") {}
  void accept(pr::Visitor& v) override;
};

class TPointer : public SymType {
 public:
  TPointer() : SymType("pointer") {}
  void accept(pr::Visitor& v) override;
};

class Alias : public SymType {
 public:
  Alias(const tok::ptr_Token&);
  Alias(const tok::ptr_Token&, const ptr_Symbol&);

  ptr_Symbol type;
  void accept(pr::Visitor& v) override;
};

class Pointer : public SymType {
 public:
  Pointer(const tok::ptr_Token&);
  Pointer(const tok::ptr_Token&, const ptr_Symbol&);

  ptr_Symbol typeBase;
  void accept(pr::Visitor& v) override;
};

class StaticArray : public SymType {
 public:
  StaticArray(const tok::ptr_Token&);

  std::list<std::pair<int, int>> bounds;
  ptr_Symbol typeElem;
  void accept(pr::Visitor& v) override;
};

class OpenArray : public SymType {
 public:
  OpenArray(const tok::ptr_Token&);

  ptr_Symbol typeElem;
  void accept(pr::Visitor& v) override;
};

class Record : public SymType {
 public:
  Record(const tok::ptr_Token&);

  TableSymbol fields;
  void accept(pr::Visitor& v) override;
};

class FunctionSignature : public SymType {
 public:
  FunctionSignature() = default;
  FunctionSignature(const tok::ptr_Token&);

  void setParamsList(const ListSymbol&);
  bool isProcedure() { return returnType == nullptr; }

  ptr_Symbol returnType = nullptr;
  void accept(pr::Visitor& v) override;

  TableSymbol paramsTable;
  ListSymbol paramsList;
};

class ForwardType : public SymType {
 public:
  ForwardType(const tok::ptr_Token&);

  ptr_Symbol resolveType = nullptr;
  void accept(pr::Visitor& v) override;
};


class SymVar : public Symbol {
 public:
  using Symbol::Symbol;
  SymVar(const tok::ptr_Token& n, const ptr_Symbol& t);

  ptr_Symbol type;
};

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
};

class Const : public SymVar {
 public:
  using SymVar::SymVar;
  pr::ptr_Expr value;
  void accept(pr::Visitor& v) override;
};

class SymFun : public Symbol {
  using Symbol::Symbol;
};

class ForwardFunction : public SymFun {
 public:
  using SymFun::SymFun;

  ptr_Symbol forwardSignature;
  ptr_Symbol function = nullptr;
  void accept(pr::Visitor& v) override;
};

class Function : public SymFun {
 public:
  using SymFun::SymFun;

  ptr_Symbol signature;
  Tables localVar;
  pr::ptr_Stmt body;
  void accept(pr::Visitor& v) override;
};

class MainFunction : public SymFun {
 public:
  MainFunction() : SymFun("Main block"), body(nullptr) {}

  Tables decl;
  pr::ptr_Stmt body;
  void accept(pr::Visitor& v) override;
};