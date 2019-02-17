#include "table_symbol.h"
#include "symbol_type.h"
#include "symbol_var.h"
#include "symbol_fun.h"

#include "exception"
#include "../exception.h"
#include "visitor.h"


Symbol::Symbol(const std::string& n)
  : name(n) {}

Symbol::Symbol(const Token& t)
    :  ASTNode(t), name(t.getString()) {}

bool Tables::checkContain(const std::string& t) {
  return tableType.checkContain(t) || tableVariable.checkContain(t) ||
          tableFunction.checkContain(t) || tableConst.checkContain(t);
}

void Tables::insertCheck(const std::shared_ptr<Symbol>& t) {
  if (checkContain(t->getSymbolName()) &&
    (!tableVariable.find(t->getSymbolName())->isForward() ||
     !tableFunction.find(t->getSymbolName())->isForward())) {
    throw AlreadyDefinedException(t->getDeclPoint(), t->getSymbolName());
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
    if (tableType.find(e->getSymbolName())->isForward()) {
      throw SemanticException(e->getDeclPoint(), "Type \"" + e->getSymbolName() + "\" not resolve");
    }
    e->type = tableType.find(e->getSymbolName());
  }
  forwardType.clear();
}

void Tables::resolveForwardFunction() {
  for (auto& e : forwardFunction) {
    auto& function = tableFunction.find(e->getSymbolName());
    if (function->isForward()) {
      throw SemanticException(e->getDeclPoint(), "Function \"" + e->getSymbolName() + "\" not resolve");
    }
    if (function->signature == nullptr) {
      throw std::logic_error("Signature nullptr");
    }
    if (!function->signature->equals(e->signature.get())) {
      throw SemanticException(e->getDeclPoint(),
        "Signature resolve function not equals with forward function " + e->getSymbolName());
    }
    e->function = function;
  }
  forwardFunction.clear();
}

uint64_t Tables::sizeVar() {
  uint64_t s = 0;
  for (auto& v : tableVariable) {
    s += v.second->type->size();
  }
  return s;
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
  return false;
}

bool OpenArray::equalsForCheckArgument(SymType* s) const {
  if (dynamic_cast<StaticArray*>(s)) {
    auto p = dynamic_cast<StaticArray*>(s);
    auto copy = std::make_shared<StaticArray>(*p);
    copy->bounds.pop_front();
    if (copy->bounds.empty()) {
      return typeElem->equalsForCheckArgument(copy->typeElem.get());
    } else {
      return typeElem->equalsForCheckArgument(copy.get());
    }
  } if (dynamic_cast<Alias*>(s)) {
    auto p = dynamic_cast<Alias*>(s);
    return equalsForCheckArgument(p->type.get());
  }
  return false;
}

bool Record::equals(SymType* s) const {
  if (dynamic_cast<Record*>(s)) {
    auto record = dynamic_cast<Record*>(s);
    return (!isAnonymous() && s->getSymbolName() == this->getSymbolName()) ||
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

Write::Write(bool newLine) : isnewLine(!newLine) {
  if (newLine)
    setSymbolName("write");
  else
    setSymbolName("writeln");
}

Read::Read(bool newLine) {
  if (newLine)
    setSymbolName("read");
  else
    setSymbolName("readln");
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
  SymFun::signature->returnType = std::make_shared<Char>();
  ListParam p;
  auto var = std::make_shared<ParamVar>();
  var->type = std::make_shared<Int>();
  var->spec = ParamSpec::NotSpec;
  p.push_back(var);
  SymFun::signature->setParamsList(p);
}

Ord::Ord() : SymFun("ord") {
  SymFun::signature = std::make_shared<FunctionSignature>();
  SymFun::signature->returnType = std::make_shared<Int>();
  ListParam p;
  auto var = std::make_shared<ParamVar>();
  var->type = std::make_shared<Char>();
  var->spec = ParamSpec::NotSpec;
  p.push_back(var);
  SymFun::signature->setParamsList(p);
}

High::High() : SymFun("high") {}

Low::Low() : SymFun("low") {}

Exit::Exit(ptr_Type returnType) : SymFun("exit"), returnType(std::move(returnType)) {};
Exit::Exit(ptr_Type returnType, std::shared_ptr<ParamVar> var)
  : SymFun("exit"), returnType(std::move(returnType)), assignmentVar(std::move(var)) {}

  // in byte
uint64_t SymVar::size() const { return type->size(); }

uint64_t SymType::size() const { return 8; }
uint64_t Void::size() const { return 0; }
uint64_t String::size() const { return 0; }
uint64_t Alias::size() const { return type->size(); }

uint64_t StaticArray::size() const {
  uint64_t size_type = typeElem->size();
  for (auto& e : bounds) {
    size_type *= e.second - e.first + 1;
  }
  return size_type;
}

uint64_t OpenArray::size() const { return 0; }

uint64_t Record::size() const {
  uint64_t size = 0;
  for (auto& e : fieldsList) {
    size += e->size();
  }
  return size;
}

uint64_t ParamVar::size() const {
  if (type->isOpenArray() || spec == ParamSpec::NotSpec) {
    return type->size();
  }
  return 8; // pointer
}

void Record::addVar(const ptr_Var& v) {
  fieldsList.push_back(v);
  fields.insert(v);
}

uint64_t Record::offset(const std::string& name) {
  uint64_t offset = 0;
  auto iter = fieldsList.begin();
  while (iter != fieldsList.end() && (*iter)->getSymbolName() != name) {
    offset += (*iter)->size();
    ++iter;
  }
  return offset;
}

bool SymType::isTrivial() const {
  return this->isInt() || this->isDouble() ||
         this->isChar() || this->isPointer() ||
         this->isProcedureType();
}

SymFun::ptr_Sign& SymFun::getSignature() { return signature; }
ptr_Stmt& Function::getBody() { return body; }
Tables& Function::getTable() { return localVar; }
SymFun::ptr_Sign& ForwardFunction::getSignature() { return function->signature; }
ptr_Stmt& ForwardFunction::getBody() { return function->getBody(); }
Tables& ForwardFunction::getTable() { return function->getTable(); }

// accept

void Int::accept(Visitor& v) { v.visit(*this); }
void Double::accept(Visitor& v) { v.visit(*this); }
void Char::accept(Visitor& v) { v.visit(*this); }
void Boolean::accept(Visitor& v) { v.visit(*this); }
void String::accept(Visitor& v) { v.visit(*this); }
void Void::accept(Visitor& v) { v.visit(*this); }

void TPointer::accept(Visitor& v) { v.visit(*this); }
void Alias::accept(Visitor& v) { v.visit(*this); }
void Pointer::accept(Visitor& v) { v.visit(*this); }
void StaticArray::accept(Visitor& v) { v.visit(*this); }
void OpenArray::accept(Visitor& v) { v.visit(*this); }
void Record::accept(Visitor& v) { v.visit(*this); }
void FunctionSignature::accept(Visitor& v) { v.visit(*this); }
void ForwardType::accept(Visitor& v) { v.visit(*this); }

void LocalVar::accept(Visitor& v) { v.visit(*this); }
void GlobalVar::accept(Visitor& v) { v.visit(*this); }
void ParamVar::accept(Visitor& v) { v.visit(*this); }
void Const::accept(Visitor& v) { v.visit(*this); }

void ForwardFunction::accept(Visitor& v) { v.visit(*this); }
void Function::accept(Visitor& v) { v.visit(*this); }
void MainFunction::accept(Visitor& v) { v.visit(*this); }

void Write::accept(Visitor& v) { v.visit(*this); }
void Read::accept(Visitor& v) { v.visit(*this); }
void Trunc::accept(Visitor& v) { v.visit(*this); }
void Round::accept(Visitor& v) { v.visit(*this); }
void Succ::accept(Visitor& v) { v.visit(*this); }
void Prev::accept(Visitor& v) { v.visit(*this); }
void Chr::accept(Visitor& v) { v.visit(*this); }
void Ord::accept(Visitor& v) { v.visit(*this); }
void High::accept(Visitor& v) { v.visit(*this); }
void Low::accept(Visitor& v) { v.visit(*this); }
void Exit::accept(Visitor& v) { v.visit(*this); }