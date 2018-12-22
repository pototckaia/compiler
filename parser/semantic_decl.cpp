#include <iostream>
#include "semantic_decl.h"

#include "../exception.h"

SemanticDecl::SemanticDecl() : stackTable(Tables()) {
  auto& t = stackTable.top();

  t.tableType.insert(std::make_shared<Int>());
  t.tableType.insert(std::make_shared<Double>());
  t.tableType.insert(std::make_shared<Char>());
  t.tableType.insert(std::make_shared<TPointer>());
  t.tableType.insert(std::make_shared<Boolean>());

  // embedded function
  t.tableFunction.insert(std::make_shared<Write>());
  t.tableFunction.insert(std::make_shared<Write>(true));
  t.tableFunction.insert(std::make_shared<Read>());
  t.tableFunction.insert(std::make_shared<Read>(true));
  t.tableFunction.insert(std::make_shared<Trunc>());
  t.tableFunction.insert(std::make_shared<Round>());
  t.tableFunction.insert(std::make_shared<Succ>());
  t.tableFunction.insert(std::make_shared<Prev>());
  t.tableFunction.insert(std::make_shared<Chr>());
  t.tableFunction.insert(std::make_shared<Ord>());
  t.tableFunction.insert(std::make_shared<High>());
  t.tableFunction.insert(std::make_shared<Low>());
}

void SemanticDecl::parseTypeDecl(std::list<std::pair<pr::ptr_Token, ptr_Type>>& decls) {
  for (auto& e : decls) {
    auto alias = std::make_shared<Alias>(e.first, e.second);
    e.second->name = e.first->getValueString();

    if (!stackTable.checkContain(alias->name)) {
      stackTable.top().tableType.insert(alias);
    } else if (stackTable.top().tableType.checkContain(alias->name) &
               stackTable.top().tableType.find(alias->name)->isForward()) {
      stackTable.top().tableType.replace(alias);
    } else {
      throw AlreadyDefinedException(e.first);
    }
  }
  stackTable.top().resolveForwardType();
}

ptr_Type SemanticDecl::parseSimpleType(pr::ptr_Token t) {
  if (stackTable.isType(t->getValueString())) {
    return stackTable.findType(t->getValueString());
  } else if (stackTable.checkContain(t->getValueString())) {
    throw SemanticException(t->getLine(), t->getColumn(), " Not type");
  } else {
    throw NotDefinedException(t);
  }
}

ptr_Type SemanticDecl::parseArrayType(pr::ptr_Token t, StaticArray::BoundsType b, ptr_Type el) {
  auto array = std::make_shared<StaticArray>(t->getLine(), t->getColumn());
  array->bounds = std::move(b);
  array->typeElem = std::move(el);
  return array;
}

ptr_Type
SemanticDecl::parseRecordType(tok::ptr_Token declPoint,
                              std::list<std::pair<std::unique_ptr<tok::ListToken>, ptr_Type>> listVar) {
  auto record = std::make_shared<Record>(declPoint->getLine(), declPoint->getColumn());
  for (auto& e : listVar) {
    for (auto& id : *(e.first)) {
      if (record->fields.checkContain(id->getValueString())) {
        throw AlreadyDefinedException(id);
      }
      record->fields.insert(std::make_shared<LocalVar>(id, e.second));
    }
  }
  return record;
}

ptr_Type SemanticDecl::parsePointer(tok::ptr_Token declPoint, tok::ptr_Token token, bool isCanForwardType) {
  auto p = std::make_shared<Pointer>(declPoint->getLine(), declPoint->getColumn());
  if (stackTable.isType(token->getValueString())) {
    p->typeBase = stackTable.findType(token->getValueString());
    return p;
  } else if (!stackTable.checkContain(token->getValueString()) && isCanForwardType) {
    auto forward = std::make_shared<ForwardType>(token);
    stackTable.top().insert(forward);
    p->typeBase = forward;
    return p;
  } else {
    throw NotDefinedException(token);
  }
}

ptr_Type SemanticDecl::parseOpenArray(tok::ptr_Token declPoint, ptr_Type type) {
  return std::make_shared<OpenArray>(declPoint, std::move(type));
}

std::shared_ptr<FunctionSignature> SemanticDecl::parseFunctionSignature(int line, int column, ListParam params,
                                                                        ptr_Type returnType) {
  auto signature = std::make_shared<FunctionSignature>(line, column);
  signature->setParamsList(std::move(params));
  signature->returnType = std::move(returnType);
  return signature;
}

ListParam SemanticDecl::parseFormalParamSection(TableSymbol<ptr_Var>& paramTable,
                                                ParamSpec paramSpec, tok::ListToken listId, ptr_Type type) {
  ListParam paramList;
  for (auto& e : listId) {
    if (paramTable.checkContain(e->getValueString())) {
      throw AlreadyDefinedException(e);
    }
    auto param = std::make_shared<ParamVar>(e, type);
    param->spec = paramSpec;
    paramTable.insert(param);
    paramList.push_back(param);
  }
  return paramList;
}

std::shared_ptr<MainFunction> SemanticDecl::parseMainBlock(pr::ptr_Stmt body) {
  stackTable.top().resolveForwardFunction();
  auto main = std::make_shared<MainFunction>(stackTable.top(), std::move(body));
  stackTable.pop();
  if (!stackTable.isEmpty()) {
    throw std::logic_error("Table Symbol not empty by the end");
  }
  return main;
}

void SemanticDecl::parseFunctionForward(const tok::ptr_Token& decl, std::shared_ptr<FunctionSignature> si) {
  if (stackTable.checkContain(decl->getValueString())) {
    throw AlreadyDefinedException(decl);
  }
  auto f =  std::make_shared<ForwardFunction>(std::move(decl), std::move(si));
  stackTable.top().insert(f);
}

void SemanticDecl::parseFunctionDeclBegin(std::shared_ptr<FunctionSignature> s) {
  stackTable.pushEmpty(); // for param variable
  for (auto& e : s->paramsTable) {
    stackTable.top().tableVariable.insert(e.second);
  }
  stackTable.pushEmpty(); // for decl
}

void SemanticDecl::parseFunctionDeclEnd(const tok::ptr_Token& decl,
                                        std::shared_ptr<FunctionSignature> s, pr::ptr_Stmt b) {
  auto declTable = stackTable.top();
  stackTable.pop();
  stackTable.pop();
  auto function = std::make_shared<Function>(decl, s, std::move(b), declTable);
  if (!stackTable.checkContain(decl->getValueString())) {
    stackTable.top().tableFunction.insert(function);
    return;
  } else if (stackTable.isFunction(function->name) &&
             stackTable.findFunction(function->name)->isForward()) {
    stackTable.top().tableFunction.replace(function);
  } else {
    throw AlreadyDefinedException(decl);
  }
}

void SemanticDecl::parseConstDecl(const tok::ptr_Token& decl, pr::ptr_Expr expr) {
  if (stackTable.checkContain(decl->getValueString())) {
    throw AlreadyDefinedException(decl);
  }
  auto cons = std::make_shared<Const>(decl);
  // TODO
  //cons->value = expr;
}

void SemanticDecl::parseVariableDecl(tok::ListToken listId, ptr_Type type, bool isGlobal) {
  for (auto& e : listId) {
    if (stackTable.checkContain(e->getValueString())) {
      throw AlreadyDefinedException(e);
    }
    std::shared_ptr<SymVar> var;
    if (isGlobal) {
      var = std::make_shared<GlobalVar>(e, type);
    } else {
      var = std::make_shared<LocalVar>(e, type);
    }
    stackTable.top().tableVariable.insert(var);
  }
}

void SemanticDecl::parseVariableDecl(tok::ListToken id, ptr_Type type, pr::ptr_Expr def, bool isGlobal) {
  auto name = id.back()->getValueString();
  parseVariableDecl(std::move(id), type, isGlobal);
  auto& var = stackTable.top().tableVariable.find(name);
  // TODO
  //  var->defaultValue = def;
}