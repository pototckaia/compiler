#pragma once

#include <string>
#include <map>
#include <list>
#include <memory>
#include <vector>

#include "astnode.h"
#include "token.h"
#include "exception"


class ParamVar;
class ForwardType;
class ForwardFunction;
class Const;
using ListParam = std::list<std::shared_ptr<ParamVar>>;

template <typename T>
class TableSymbol {
 public:
  TableSymbol() = default;

  void insert(T);
  bool checkContain(const std::string&);
  T& find(const std::string&);
  void replace(T);

  auto begin() { return table.begin(); };
  auto end() { return table.end(); }

 private:
  std::map<std::string, T> table;
};

class Tables {
 public:
  bool checkContain(const std::string&);
  void insertCheck(const std::shared_ptr<Symbol>&);

  void insert(const std::shared_ptr<ForwardType>&);
  void insert(const std::shared_ptr<ForwardFunction>&);

  void resolveForwardType();
  TableSymbol<std::shared_ptr<SymType>> tableType;
  TableSymbol<std::shared_ptr<SymVar>> tableVariable;
  TableSymbol<std::shared_ptr<Const>> tableConst;
  TableSymbol<std::shared_ptr<SymFun>> tableFunction;
 
 private:
  std::list<std::shared_ptr<ForwardType>> forwardType;
  std::list<std::shared_ptr<ForwardFunction>> forwardFunction;
};

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
  ptr_Type findType(const std::string&);

  std::list<Tables> stack;
};


template<class T>
void TableSymbol<T>::replace(T t) {
  table[t->name] = std::forward<T>(t);
}

template<class T>
void TableSymbol<T>::insert(T t) {
  if (checkContain(t->name) && !find(t->name)->isForward) {
    throw std::logic_error("Already defined " + t->name);
  }
  table[t->name] = std::forward<T>(t);
}

template<class T>
bool TableSymbol<T>::checkContain(const std::string& n) {
  return table.count(n) > 0;
}

template<class T>
T& TableSymbol<T>::find(const std::string& n) {
  return table.at(n);
}