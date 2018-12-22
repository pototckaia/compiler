#pragma once

#include "astnode.h"
#include "table_symbol.h"

class SymType : public Symbol {
 public:
  using Symbol::Symbol;
  virtual bool equals(SymType* s) const = 0;
  bool isAnonymous() const { return name.empty(); }

  virtual bool isVoid() const { return true; }

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
};

class Double : public SymType {
 public:
  Double() : SymType("double") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
};

class Char : public SymType {
 public:
  Char() : SymType("char") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
};

class Boolean : public SymType {
 public:
  Boolean() : SymType("boolean") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
};

class TPointer : public SymType {
 public:
  TPointer() : SymType("pointer") {}
  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
};

class Alias : public SymType {
 public:
  using SymType::SymType;
  Alias(const tok::ptr_Token& t, ptr_Type p)
    : SymType(t), type(std::move(p)) {}

  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;

  ptr_Type type;
};

class ForwardType : public SymType {
 public:
  ForwardType(const tok::ptr_Token& t) : SymType(t) {}

  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;
  bool isForward() const override { return true; }

  ptr_Type resolveType;
};


class Pointer : public SymType {
 public:
  using SymType::SymType;
  Pointer(const tok::ptr_Token& t, ptr_Type p)
    : SymType(t), typeBase(std::move(p)) {}

  void accept(pr::Visitor& v) override;
  bool equals(SymType* s) const override;

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
