#include "table_symbol.h"
#include "symbol_type.h"
#include "symbol_var.h"
#include "symbol_fun.h"

#include "exception"
#include "../exception.h"
#include "visitor.h"

bool Tables::checkContain(const std::string& t) {
  return tableType.checkContain(t) || tableVariable.checkContain(t) ||
          tableFunction.checkContain(t) || tableConst.checkContain(t);
}

void Tables::insertCheck(const std::shared_ptr<Symbol>& t) {
  if (checkContain(t->name) &&
    (!tableVariable.find(t->name)->isForward() ||
     !tableFunction.find(t->name)->isForward())) {
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
    if (tableType.find(e->name)->isForward()) {
      throw SemanticException(e->line, e->column, "Type \"" + e->name + "\" not resolve");
    }
    e->type = tableType.find(e->name);
  }
  forwardType.clear();
}

void Tables::resolveForwardFunction() {
  for (auto& e : forwardFunction) {
    auto& function = tableFunction.find(e->name);
    if (function->isForward()) {
      throw SemanticException(e->line, e->column, "Function \"" + e->name + "\" not resolve");
    }
    if (function->signature == nullptr) {
      throw std::logic_error("Signature nullptr");
    }
    if (!function->signature->equals(e->signature.get())) {
      throw SemanticException(e->line, e->column,
        "Signature resolve function not equals with forward function " + e->name);
    }
    e->function = function;
  }
  forwardFunction.clear();
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

ptr_Type StackTable::findType(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableType.checkContain(n)) {
      return iter->tableType.find(n);
    }
  }
  throw NotDefinedException(n);
}

bool StackTable::isFunction(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableFunction.checkContain(n)) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<SymFun> StackTable::findFunction(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableFunction.checkContain(n)) {
      return iter->tableFunction.find(n);
    }
  }
  throw NotDefinedException(n);
}

bool StackTable::isConst(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableConst.checkContain(n)) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<Const> StackTable::findConst(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableConst.checkContain(n)) {
      return iter->tableConst.find(n);
    }
  }
  throw NotDefinedException(n);
}

bool StackTable::isVar(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableVariable.checkContain(n)) {
      return true;
    }
  }
  return false;
}

ptr_Var StackTable::findVar(const std::string& n) {
  for (auto iter = stack.rbegin(); iter != stack.rend(); ++iter) {
    if (iter->tableVariable.checkContain(n)) {
      return iter->tableVariable.find(n);
    }
  }
  throw NotDefinedException(n);
}

void FunctionSignature::setParamsList(ListParam t) {
  paramsList = t;
  for (auto& e : t) {
    paramsTable.insert(e);
  }
}

Function::~Function() = default;

std::string toString(ParamSpec p) {
  switch (p) {
    case ParamSpec::NotSpec: {
      return "Not specification";
    }
    case ParamSpec::Var: {
      return "Var";
    }
    case ParamSpec::Const: {
      return "Const";
    }
    case ParamSpec::Out: {
      return "Out";
    }
  }
}

// type equals

bool SymType::checkAlias(SymType* s) const {
  if (dynamic_cast<Alias*>(s)) {
    return equals(dynamic_cast<Alias*>(s)->type.get());
  } else if (dynamic_cast<ForwardType*>(s)) {
    return equals(dynamic_cast<ForwardType*>(s)->type.get());
  }
  return false;
}

bool Int::equals(SymType* s) const {
  return dynamic_cast<Int*>(s) || checkAlias(s);
}

bool Double::equals(SymType* s) const {
  return dynamic_cast<Double*>(s) || checkAlias(s);
}

bool Char::equals(SymType* s) const {
  return dynamic_cast<Char*>(s) || checkAlias(s);
}

bool Boolean::equals(SymType* s) const {
  return dynamic_cast<Boolean*>(s) || checkAlias(s);
}

bool TPointer::equals(SymType* s) const {
  return dynamic_cast<TPointer*>(s) || checkAlias(s);
}

bool Alias::equals(SymType* s) const {
  return type->equals(s);
}

bool Pointer::equals(SymType* s) const {
  if (dynamic_cast<Pointer*>(s)) {
    return typeBase->equals(dynamic_cast<Pointer*>(s)->typeBase.get());
  }
  return checkAlias(s);
}

bool StaticArray::equals(SymType* s) const {
  if (dynamic_cast<StaticArray*>(s)) {
    auto p = dynamic_cast<StaticArray*>(s);
    return p->bounds == bounds && typeElem->equals(p->typeElem.get());
  }
  return checkAlias(s);
}

bool OpenArray::equals(SymType* s) const {
  if (dynamic_cast<OpenArray*>(s)) {
    auto p = dynamic_cast<OpenArray*>(s);
    return typeElem->equals(p->typeElem.get());
  }
  return checkAlias(s);
}

bool OpenArray::equalsForCheckArgument(SymType* s) const {
  if (dynamic_cast<StaticArray*>(s)) {
    auto p = dynamic_cast<StaticArray*>(s);
    return typeElem->equalsForCheckArgument(p->typeElem.get());
  }
  return equals(s);
}

bool Record::equals(SymType* s) const {
  if (dynamic_cast<Record*>(s)) {
    auto record = dynamic_cast<Record*>(s);
    return (!isAnonymous() && s->name == name) ||
           (isAnonymous() && this == record);
  }
  return checkAlias(s);
}

bool FunctionSignature::equals(SymType* s) const  {
  if (dynamic_cast<FunctionSignature*>(s)) {
    auto p = dynamic_cast<FunctionSignature*>(s);
    bool eq = (isProcedure() && p->isProcedure()) || returnType->equals(p->returnType.get());
    eq &= paramsList.size() == p->paramsList.size();
    auto first = paramsList.begin();
    auto second = p->paramsList.begin();
    for (; eq && first != paramsList.end(); ++first, ++second) {
      eq &= (*first)->equals(**second);
    }
    return eq;
  }
  return checkAlias(s);
}

bool ForwardType::equals(SymType* s) const {
  if (dynamic_cast<ForwardType*>(s)) {
    auto p = dynamic_cast<ForwardType*>(s);
    return type->equals(p->type.get());
  }
  return checkAlias(s);
}

bool ParamVar::equals(ParamVar& p) const {
  return spec == p.spec && type->equals(p.type.get());
}

Write::Write(bool newLine)  {
  if (newLine)
    name =  "write";
  else
    name = "writeln";
}

Read::Read(bool newLine) {
  if (newLine)
    name =  "read";
  else
    name = "readln";
}

Round::Round() : SymFun("round") {
  SymFun::signature = std::make_shared<FunctionSignature>();
  SymFun::signature->returnType = std::make_shared<Int>();
  ListParam p;
  auto var = std::make_shared<ParamVar>();
  var->type = std::make_shared<Double>();
  var->spec = ParamSpec::NotSpec;
  p.push_back(var);
  SymFun::signature->setParamsList(p);
}

Trunc::Trunc() : SymFun("trunc") {
  SymFun::signature = std::make_shared<FunctionSignature>();
  SymFun::signature->returnType = std::make_shared<Int>();
  ListParam p;
  auto var = std::make_shared<ParamVar>();
  var->type = std::make_shared<Double>();
  var->spec = ParamSpec::NotSpec;
  p.push_back(var);
  SymFun::signature->setParamsList(p);
};

Succ::Succ() : SymFun("succ") {
  SymFun::signature = std::make_shared<FunctionSignature>();
  SymFun::signature->returnType = std::make_shared<Int>();
  ListParam p;
  auto var = std::make_shared<ParamVar>();
  var->type = std::make_shared<Int>();
  var->spec = ParamSpec::NotSpec;
  p.push_back(var);
  SymFun::signature->setParamsList(p);
}

Prev::Prev()  : SymFun("prev") {
  SymFun::signature = std::make_shared<FunctionSignature>();
  SymFun::signature->returnType = std::make_shared<Int>();
  ListParam p;
  auto var = std::make_shared<ParamVar>();
  var->type = std::make_shared<Int>();
  var->spec = ParamSpec::NotSpec;
  p.push_back(var);
  SymFun::signature->setParamsList(p);
}

Chr::Chr() : SymFun("chr") {
  SymFun::signature = std::make_shared<FunctionSignature>();
  SymFun::signature->returnType = std::make_shared<Int>();
  ListParam p;
  auto var = std::make_shared<ParamVar>();
  var->type = std::make_shared<Char>();
  var->spec = ParamSpec::NotSpec;
  p.push_back(var);
  SymFun::signature->setParamsList(p);
}

Ord::Ord() : SymFun("ord") {
  SymFun::signature = std::make_shared<FunctionSignature>();
  SymFun::signature->returnType = std::make_shared<Char>();
  ListParam p;
  auto var = std::make_shared<ParamVar>();
  var->type = std::make_shared<Int>();
  var->spec = ParamSpec::NotSpec;
  p.push_back(var);
  SymFun::signature->setParamsList(p);
}

High::High() : SymFun("high") {}

Low::Low() : SymFun("low") {}

Exit::Exit(ptr_Type returnType) : returnType(std::move(returnType)) {  };

// accept

void Int::accept(pr::Visitor& v) { v.visit(*this); }
void Double::accept(pr::Visitor& v) { v.visit(*this); }
void Char::accept(pr::Visitor& v) { v.visit(*this); }
void Boolean::accept(pr::Visitor& v) { v.visit(*this); }
void String::accept(pr::Visitor& v) { v.visit(*this); }

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

void Write::accept(pr::Visitor& v) { v.visit(*this); }
void Read::accept(pr::Visitor& v) { v.visit(*this); }
void Trunc::accept(pr::Visitor& v) { v.visit(*this); }
void Round::accept(pr::Visitor& v) { v.visit(*this); }
void Succ::accept(pr::Visitor& v) { v.visit(*this); }
void Prev::accept(pr::Visitor& v) { v.visit(*this); }
void Chr::accept(pr::Visitor& v) { v.visit(*this); }
void Ord::accept(pr::Visitor& v) { v.visit(*this); }
void High::accept(pr::Visitor& v) { v.visit(*this); }
void Low::accept(pr::Visitor& v) { v.visit(*this); }
void Exit::accept(pr::Visitor& v) { v.visit(*this); }