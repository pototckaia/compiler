#pragma once

#include "astnode.h"
#include "table_symbol.h"
#include <cstdint>

class Record;
class FunctionSignature;
class StaticArray;

class SymType : public Symbol {
 public:
  using Symbol::Symbol;
  // todo normal double dispatch
  virtual bool equals(SymType* s) const = 0;
  // todo move to cpp
  virtual bool equalsForCheckArgument(SymType* s) const { return equals(s); } // вызывет paramenter передается argument

  // todo how remove this
  virtual bool isVoid() const { return false; }
  virtual bool isString() const { return false; }
  virtual bool isInt() const { return false; }
  virtual bool isDouble() const { return false; }
  virtual bool isBool() const { return false; }
  virtual bool isChar() const { return false; }
  virtual bool isPurePointer() const { return false; }
  virtual bool isTypePointer() const { return false; }
  bool isPointer() const { return isTypePointer() || isPurePointer(); }
  virtual bool isProcedureType() const { return false; }
  virtual bool isOpenArray() const { return false; }
  virtual bool isStaticArray() const { return false; }
  bool isTrivial() const;

  // todo remove it
  virtual ptr_Type getPointerBase() { return nullptr; }
  virtual Record* getRecord() { return nullptr; }
  virtual FunctionSignature* getSignature() { return nullptr; }
  virtual StaticArray* getStaticArray() { return nullptr; }

  // todo remove
  virtual uint64_t size() const;

 protected:
  bool checkAlias(SymType* s) const;
};

class Void : public SymType {
 public:
  Void();
  bool isVoid() const override { return true; }
  void accept(Visitor& v) override;
  // todo move to cpp
  bool equals(SymType* s) const override { return false; }
  uint64_t size() const override;
};

class Int : public SymType {
 public:
  Int();
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isInt() const override { return true; }
};

class Double : public SymType {
 public:
  Double();
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isDouble() const override { return true; }
};

class Char : public SymType {
 public:
  Char();
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isChar() const override { return true; }
};

class String : public SymType {
 public:
  String();
  void accept(Visitor& v) override;
  // todo move to cpp
  bool equals(SymType* s) const override { return s->isString(); }
  bool isString() const override { return true; }
  uint64_t size() const override;
};

class Boolean : public SymType {
 public:
  Boolean();
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isBool() const override { return true; }
};

class TPointer : public SymType {
 public:
  TPointer();
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isPurePointer() const override { return true; }
};

class Alias : public SymType {
 public:
  Alias(const Token&);
  Alias(const Token&, ptr_Type);

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

  // todo remove this
  ptr_Type getPointerBase() override { return type->getPointerBase(); }
  Record* getRecord() override { return type->getRecord(); }
  FunctionSignature* getSignature() override { return type->getSignature(); }
  StaticArray* getStaticArray() override { return type->getStaticArray(); }

  // todo remove virtual
  uint64_t size() const override;
  auto& getRefType() { return type; }

 protected:
  ptr_Type type;
};

class ForwardType : public Alias {
 public:
  ForwardType(const Token& t);

  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  // todo remove virtual
  bool isForward() const override { return true; }
  void setRefType(ptr_Type t) { type = std::move(t); }
};


class Pointer : public SymType {
 public:
  using SymType::SymType;
  Pointer(ptr_Type p);
  Pointer(const Token& t, ptr_Type p);

  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isTypePointer() const override { return true; }
  // todo remove virtual
  ptr_Type getPointerBase() override { return typeBase; }
  void setPointerBase(ptr_Type t) { typeBase = t; }

 private:
  ptr_Type typeBase;
};


class StaticArray : public SymType {
 public:
  using SymType::SymType;
  using BoundsType = std::list<std::pair<uint64_t, uint64_t>>;
  StaticArray(const Token&, ptr_Type, const BoundsType&);

  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isStaticArray() const override { return true; }
  StaticArray* getStaticArray() { return this; }
  uint64_t size() const override;

  auto& getBounds() { return bounds; }
  auto& getRefType() { return typeElem; }

 private:
  BoundsType bounds;
  ptr_Type typeElem;
};

class OpenArray : public SymType {
 public:
  OpenArray(const Token& decl, ptr_Type type);

  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  bool equalsForCheckArgument(SymType* s) const override;
  bool isOpenArray() const override { return true; }
  uint64_t size() const override;

  auto& getRefType() { return typeElem; }

 private:
  ptr_Type typeElem;
};

class Record : public SymType {
 public:
	Record(const Token&);

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
	FunctionSignature(ListParam, ptr_Type);
  FunctionSignature(const Token&, ListParam, ptr_Type);

  auto& getParamList() { return paramsList; }
  auto& getReturnType() { return returnType; }
  auto& getParamTable() { return paramsTable; }
  void setParamsList(ListParam t);
  bool isProcedure() const { return returnType->isVoid(); }

  bool isProcedureType() const override { return true; }
  void accept(Visitor& v) override;
  bool equals(SymType* s) const override;
  FunctionSignature* getSignature() override { return this; }

 private:
  TableSymbol<std::shared_ptr<ParamVar>> paramsTable;
  ListParam paramsList;
  ptr_Type returnType;
};
