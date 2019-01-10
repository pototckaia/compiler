#pragma once

#include "astnode.h"
#include "table_symbol.h"
#include <cstdint>

class Record;

class SymType : public Symbol {
 public:
  using Symbol::Symbol;
  virtual bool equals(SymType* s) const = 0;
  virtual bool equalsForCheckArgument(SymType* s) const { return equals(s); } // вызывет paramenter передается argument
  bool isAnonymous() const { return name.empty(); }

  virtual bool isVoid() const { return false; }
  virtual bool isString() const { return false; }
  virtual bool isInt() const { return false; }
  virtual bool isDouble() const { return false; }
  virtual bool isBool() const { return false; }
  virtual bool isChar() const { return false; }
  virtual bool isPurePointer() const { return false; }
  virtual bool isTypePointer() const { return false; }
  virtual bool isPointer() const { return isTypePointer() || isPurePointer(); }
  virtual bool isProcedureType() const { return false; }
  virtual bool isOpenArray() const { return false; }
  virtual bool isStaticArray() const { return false; }
  virtual ptr_Type getPointerBase() { return nullptr; }
  virtual Record* getRecord() { return nullptr; }

  virtual uint64_t size() const;

 protected:
  bool checkAlias(SymType* s) const;
};

class Void : public SymType {
 public:
  Void() : SymType("void") {}
  bool isVoid() const override { return true; }
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override { return false; }
  uint64_t size() const override;
};

class Int : public SymType {
 public:
  Int() : SymType("integer") {}
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isInt() const override { return true; }
};

class Double : public SymType {
 public:
  Double() : SymType("double") {}
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isDouble() const override { return true; }
};

class Char : public SymType {
 public:
  Char() : SymType("char") {}
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isChar() const override { return true; }
};

class String : public SymType {
 public:
  String() : SymType("string") {}
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override { return s->isString(); }
  bool isString() const override { return true; }
  uint64_t size() const override;
};

class Boolean : public SymType {
 public:
  Boolean() : SymType("boolean") {}
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isBool() const override { return true; }
};

class TPointer : public SymType {
 public:
  TPointer() : SymType("pointer") {}
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isPurePointer() const override { return true; }
};

class Alias : public SymType {
 public:
  using SymType::SymType;
  Alias(const tok::ptr_Token& t, ptr_Type p)
    : SymType(t), type(std::move(p)) {}

  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;

  bool isInt() const override { return type->isInt(); }
  bool isDouble() const override { return type->isDouble(); }
  bool isBool() const override { return type->isBool(); }
  bool isChar() const override { return type->isChar(); }
  bool isPurePointer() const override { return type->isPurePointer(); }
  bool isTypePointer() const override { return type->isTypePointer(); }
  bool isProcedureType() const override { return type->isProcedureType(); }
  bool isStaticArray() const override { return type->isStaticArray(); }
  bool isOpenArray() const override { return type->isOpenArray(); }
  ptr_Type getPointerBase() override { return type->getPointerBase(); }
  Record* getRecord() override { return type->getRecord(); }
  uint64_t size() const override;

  ptr_Type type;
};

class ForwardType : public Alias {
 public:
  ForwardType(const tok::ptr_Token& t) : Alias(t) {}

  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isForward() const override { return true; }
};


class Pointer : public SymType {
 public:
  using SymType::SymType;
  Pointer(ptr_Type p) : SymType(), typeBase(std::move(p)) {}
  Pointer(const tok::ptr_Token& t, ptr_Type p)
    : SymType(t), typeBase(std::move(p)) {}

  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isTypePointer() const override { return true; }
  ptr_Type getPointerBase() override { return typeBase; }

  ptr_Type typeBase;
};


class StaticArray : public SymType {
 public:
  using SymType::SymType;
  using BoundsType = std::list<std::pair<uint64_t, uint64_t>>;

  BoundsType bounds;
  ptr_Type typeElem;
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isStaticArray() const override { return true; }
  uint64_t size() const override;
};

class OpenArray : public SymType{
 public:
  OpenArray(const tok::ptr_Token& decl, ptr_Type type)
    : SymType(decl->getLine(), decl->getColumn()), typeElem(std::move(type)) {}

  ptr_Type typeElem;
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool equalsForCheckArgument(SymType* s) const override;
  bool isOpenArray() const override { return true; }
  uint64_t size() const override;
};

class Record : public SymType {
 public:
  using SymType::SymType;

  void addVar(const ptr_Var&);
  auto& getTable() { return fields; }

  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  uint64_t size() const override;
  uint64_t offset(const std::string& name);
  Record* getRecord() override { return this; }

 private:
  TableSymbol<ptr_Var> fields;
  std::list<ptr_Var> fieldsList;
};

class FunctionSignature : public SymType {
 public:
  using SymType::SymType;

  void setParamsList(ListParam t);
  bool isProcedureType() const override { return true; }
  bool isProcedure() const { return returnType->isVoid(); }
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;

  TableSymbol<std::shared_ptr<ParamVar>> paramsTable;
  ListParam paramsList;
  ptr_Type returnType;
};
