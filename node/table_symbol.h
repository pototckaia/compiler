#pragma once

#include <string>
#include <map>
#include <list>
#include <memory>
#include <vector>

#include "astnode.h"
#include "token.h"

template <typename T>
class TableSymbol {
 public:
  TableSymbol() = default;

  void insert(T);
  bool checkContain(const std::string&);
  T& find(const std::string&);
  void replace(T);
  auto begin() { return order.begin(); };
  auto end() { return order.end(); }

 private:
  std::map<std::string, T> table;
  std::vector<T> order;
};

template<class T>
void TableSymbol<T>::replace(T t) {
  table[t->getSymbolName()] = std::forward<T>(t);
}

template<class T>
void TableSymbol<T>::insert(T t) {
  if (checkContain(t->getSymbolName())) {
    throw std::logic_error("Already defined " + t->getSymbolName());
  }
  table[t->getSymbolName()] = std::forward<T>(t);
  order.push_back(t);
}

template<class T>
bool TableSymbol<T>::checkContain(const std::string& n) {
  return table.count(n) > 0;
}

template<class T>
T& TableSymbol<T>::find(const std::string& n) {
  return table.at(n);
}

class Tables {
 public:
  bool checkContain(const std::string&);
  void insert(const std::shared_ptr<ForwardType>&);
  void insert(const std::shared_ptr<ForwardFunction>&);
  uint64_t sizeVar();

  void resolveForwardType();
  void resolveForwardFunction();
  TableSymbol<std::shared_ptr<SymType>> tableType;
  TableSymbol<std::shared_ptr<SymVar>> tableVariable;
  TableSymbol<std::shared_ptr<Const>> tableConst;
  TableSymbol<std::shared_ptr<SymFun>> tableFunction;
 
 private:
  void insertCheck(const std::shared_ptr<Symbol>&);
  std::list<std::shared_ptr<ForwardType>> forwardType;
  std::list<std::shared_ptr<ForwardFunction>> forwardFunction;
};

class StackTable {
 public:
  StackTable() = default;
  StackTable(const Tables& global);

  void push(const Tables& t) { stack.push_back(t); }
  void pushEmpty() { stack.push_back(Tables()); }
  void pop() { stack.pop_back(); }
  Tables& top() { return stack.back(); }
  bool isEmpty() { return stack.empty(); }

  bool checkContain(const std::string& n);

  bool isType(const std::string& n);
  bool isFunction(const std::string& n);
  bool isConst(const std::string& n);
  bool isVar(const std::string& n);

  ptr_Type findType(const std::string&);
  ptr_Fun findFunction(const std::string&);
  ptr_Const findConst(const std::string&);
  ptr_Var findVar(const std::string&);

 private:
  std::list<Tables> stack;
};
