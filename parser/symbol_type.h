#pragma once

#include "astnode.h"
#include "table_symbol.h"

class SymType : public Symbol {
 public:
  using Symbol::Symbol;
  virtual bool equals(SymType* s) const = 0;
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

 protected:
  bool checkAlias(SymType* s) const;
};

class Void : public SymType {
 public:
  Void() : SymType("void") {}
  bool isVoid() const override { return true; }
  void accept(pr::Visitor& v) override {}
  bool equals(SymType* s) const override { return false; }
};

class Int : public SymType {
 public:
  Int() : SymType("integer") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isInt() const override { return true; }
};

class Double : public SymType {
 public:
  Double() : SymType("double") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isDouble() const override { return true; }
};

class Char : public SymType {
 public:
  Char() : SymType("char") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isChar() const override { return true; }
};

class String : public SymType {
 public:
  String() : SymType("string") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override { return s->isString(); }
  bool isString() const override { return true;}
};

class Boolean : public SymType {
 public:
  Boolean() : SymType("boolean") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isBool() const override { return true; }
};

class TPointer : public SymType {
 public:
  TPointer() : SymType("pointer") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isPurePointer() const override { return true; }
};

class Alias : public SymType {
 public:
  using SymType::SymType;
  Alias(const tok::ptr_Token& t, ptr_Type p)
    : SymType(t), type(std::move(p)) {}

  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;

  bool isInt() const override { return type->isInt(); }
  bool isDouble() const override { return type->isDouble(); }
  bool isBool() const override { return type->isBool(); }
  bool isChar() const override { return type->isChar(); }
  bool isPurePointer() const override { return type->isPurePointer(); }
  bool isTypePointer() const override { return type->isTypePointer(); }

  ptr_Type type;
};

class ForwardType : public Alias {
 public:
  ForwardType(const tok::ptr_Token& t) : Alias(t) {}

  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;

  bool isForward() const override { return true; }
};


class Pointer : public SymType {
 public:
  using SymType::SymType;
  Pointer(const tok::ptr_Token& t, ptr_Type p)
    : SymType(t), typeBase(std::move(p)) {}

  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isTypePointer() const override { return true; }

  ptr_Type typeBase;
};

class StaticArray : public SymType {
 public:
  using SymType::SymType;
  using BoundsType = std::list<std::pair<int, int>>;

  BoundsType bounds;
  ptr_Type typeElem;
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
};

class OpenArray : public SymType {
 public:
  OpenArray(const tok::ptr_Token& decl, ptr_Type type)
    : SymType(decl->getLine(), decl->getColumn()), typeElem(std::move(type)) {}

  ptr_Type typeElem;
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
};

class Record : public SymType {
 public:
  using SymType::SymType;

  TableSymbol<ptr_Var> fields;
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
};

class FunctionSignature : public SymType {
 public:
  using SymType::SymType;

  void setParamsList(ListParam t);
  bool isProcedure() const { return returnType->isVoid(); }
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;

  TableSymbol<std::shared_ptr<ParamVar>> paramsTable;
  ListParam paramsList;
  ptr_Type returnType;
};
