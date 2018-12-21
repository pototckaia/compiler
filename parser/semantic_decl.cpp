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
               stackTable.top().tableType.find(alias->name)->isForward) {
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
  auto main = std::make_shared<MainFunction>(stackTable.top(), std::move(body));
  stackTable.pop();
  if (!stackTable.isEmpty()) {
    throw std::logic_error("Table Symbol not empty by the end");
  }
  return main;
}