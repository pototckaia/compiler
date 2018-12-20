#include "symbol.h"

#include "exception"
#include "../exception.h"
#include "visitor.h"

void TableSymbol::replace(const ptr_Symbol& t) {
  table[t->name] = t;
}

void TableSymbol::insert(const ptr_Symbol& t) {
  if (checkContain(t->name) && !find(t->name)->isForward) {
    throw AlreadyDefinedException(t->line, t->column, t->name);
  }
  table[t->name] = t;
}

bool TableSymbol::checkContain(const std::string& n) {
  return table.count(n) > 0;
}

ptr_Symbol TableSymbol::find(const std::string& n) const {
  return table.at(n);
}

bool Tables::checkContain(const std::string& t) {
  return tableType.checkContain(t) & tableVariable.checkContain(t) &
          tableFunction.checkContain(t) & tableConst.checkContain(t);
}

void Tables::insertCheck(const std::shared_ptr<Symbol>& t) {
  if (checkContain(t->name)) {
    throw AlreadyDefinedException(t->line, t->column, t->name);
  }
}

void Tables::insert(const std::shared_ptr<ForwardType>& f) {
  forwardType.push_back(f);
  insertCheck(f);
  tableType.insert(f);
}

void Tables::insert(const std::shared_ptr<ForwardFunction>& f) {
  forwardFunction.push_back(f);
  insertCheck(f);
  tableFunction.insert(f);
}

void Tables::resolveForwardType() {
  for (auto& e : forwardType) {
    if (tableType.find(e->name)->isForward) {
      throw SemanticException(e->line, e->column, "Type \"" + e->name + "\" not resolve");
    }
    e->resolveType = tableType.find(e->name);
  }
  forwardType.clear();
}


StackTable::StackTable(const Tables& global) : stack(1, global) {}

bool StackTable::checkContain(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->checkContain(n)) {
      return true;
    }
  }
  return false;
}

bool StackTable::isType(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableType.checkContain(n)) {
      return true;
    }
  }
  return false;
}

ptr_Symbol StackTable::findType(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableType.checkContain(n)) {
      return iter->tableType.find(n);
    }
  }
  throw NotDefinedException(n);
}

Symbol::Symbol() : name(), line(1), column(1) {}
Symbol::Symbol(const std::string& n) : name(n), line(1), column(1) {}
Symbol::Symbol(const tok::ptr_Token& t)
  : name(t->getValueString()), line(t->getLine()), column(t->getColumn()) {}

Alias::Alias(const tok::ptr_Token& t) : SymType(t), type(nullptr) {}
Alias::Alias(const tok::ptr_Token& t, const ptr_Symbol& p)
  : SymType(t), type(p) {}

Pointer::Pointer(const tok::ptr_Token& t) : SymType(t), typeBase(nullptr) {}
Pointer::Pointer(const tok::ptr_Token& t, const ptr_Symbol& p)
  : SymType(t), typeBase(p) {}

StaticArray::StaticArray(const tok::ptr_Token& t) : SymType(t), typeElem(nullptr) {}
OpenArray::OpenArray(const tok::ptr_Token& t)
  : SymType(t), typeElem(nullptr) {}

Record::Record(const tok::ptr_Token& t) : SymType(t) {}

FunctionSignature::FunctionSignature(const tok::ptr_Token& t) : SymType(t) {}
void FunctionSignature::setParamsList(const ListSymbol& t) {
  paramsList = t;
  for (auto& e : t) {
    paramsTable.insert(e);
  }
}

ForwardType::ForwardType(const tok::ptr_Token& t) : SymType(t) {
  SymType::isForward = true;
}


SymVar::SymVar(const tok::ptr_Token& n, const ptr_Symbol& t)
  : Symbol(n), type(t) {}


Tables createGlobalTable() {
  Tables t;
  t.tableType.insert(std::make_shared<Int>());
  t.tableType.insert(std::make_shared<Double>());
  t.tableType.insert(std::make_shared<Char>());
  t.tableType.insert(std::make_shared<TPointer>());
  t.tableType.insert(std::make_shared<Boolean>());
  return t;
}

std::string toString(ParamSpec p) {
  switch (p) {
    case ParamSpec::NotSpec: {
      return "Not specification";
    }
    case ParamSpec::Var: {
      return "var";
    }
    case ParamSpec::Const: {
      return "Const";
    }
    case ParamSpec::Out: {
      return "Out";
    }
  }
}

void Int::accept(pr::Visitor& v) { v.visit(*this); }
void Double::accept(pr::Visitor& v) { v.visit(*this); }
void Char::accept(pr::Visitor& v) { v.visit(*this); }
void Boolean::accept(pr::Visitor& v) { v.visit(*this); }
void TPointer::accept(pr::Visitor& v) { v.visit(*this); }

void Alias::accept(pr::Visitor& v) { v.visit(*this); }
void Pointer::accept(pr::Visitor& v) { v.visit(*this); }
void StaticArray::accept(pr::Visitor& v) { v.visit(*this); }
void OpenArray::accept(pr::Visitor& v) { v.visit(*this); }
void Record::accept(pr::Visitor& v) { v.visit(*this); }
void FunctionSignature::accept(pr::Visitor& v) { v.visit(*this); }
void ForwardType::accept(pr::Visitor& v) { v.visit(*this); }

void LocalVar::accept(pr::Visitor& v) { v.visit(*this); }
void GlobalVar::accept(pr::Visitor& v) { v.visit(*this); }
void ParamVar::accept(pr::Visitor& v) { v.visit(*this); }
void Const::accept(pr::Visitor& v) { v.visit(*this); }

void ForwardFunction::accept(pr::Visitor& v) { v.visit(*this); }
void Function::accept(pr::Visitor& v) { v.visit(*this); }
void MainFunction::accept(pr::Visitor& v) { v.visit(*this); }