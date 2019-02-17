#include "semantic_decl.h"

#include "../exception.h"
#include "type_checker.h"

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

ptr_Expr SemanticDecl::parseFunctionCall(const Token& d, ptr_Expr e, ListExpr l) {
  if (dynamic_cast<Variable*>(e.get())) {
    auto name = dynamic_cast<Variable *>(e.get())->getSubToken().getString();
    if (stackTable.isType(name)) {
      if (l.empty() || l.size() > 1) {
        throw SemanticException(d, "Cast expect 1 argument");
      }
      auto c = std::make_unique<Cast>(stackTable.findType(name), std::move(l.back()));
      c->setDeclPoint(d);
      return c;
    }
  }
  return std::make_unique<FunctionCall>(d, std::move(e), std::move(l));
}


void SemanticDecl::parseTypeDecl(Token decl, ptr_Type type) {
  auto alias = std::make_shared<Alias>(decl, type);
  type->setSymbolName(decl.getString());

  if (!stackTable.top().checkContain(alias->getSymbolName())) {
    stackTable.top().tableType.insert(alias);
  } else if (stackTable.top().tableType.checkContain(alias->getSymbolName()) &
             stackTable.top().tableType.find(alias->getSymbolName())->isForward()) {
    stackTable.top().tableType.replace(alias);
  } else {
    throw AlreadyDefinedException(decl);
  }
}

void SemanticDecl::parseTypeDeclEnd() {
  stackTable.top().resolveForwardType();
}

ptr_Type SemanticDecl::parseSimpleType(Token t) {
  if (stackTable.isType(t.getString())) {
    return stackTable.findType(t.getString());
  } else if (stackTable.checkContain(t.getString())) {
    throw SemanticException(t.getLine(), t.getColumn(), " Not type");
  } else {
    throw NotDefinedException(t);
  }
}

ptr_Type SemanticDecl::parseArrayType(Token t, StaticArray::BoundsType b, ptr_Type el) {
  return std::make_shared<StaticArray>(t, std::move(el), b);
}

ptr_Type
SemanticDecl::parseRecordType(Token declPoint,
                              std::list<std::pair<std::unique_ptr<ListToken>, ptr_Type>> listVar) {
  auto record = std::make_shared<Record>(declPoint);
  for (auto& e : listVar) {
    for (auto& id : *(e.first)) {
      if (record->getTable().checkContain(id.getString())) {
        throw AlreadyDefinedException(id);
      }
      record->addVar(std::make_shared<LocalVar>(id, e.second));
    }
  }
  return record;
}

ptr_Type SemanticDecl::parsePointer(Token declPoint, Token token, bool isCanForwardType) {
  auto p = std::make_shared<Pointer>(declPoint);
  if (stackTable.isType(token.getString())) {
    p->setPointerBase(stackTable.findType(token.getString()));
    return p;
  } else if (!stackTable.checkContain(token.getString()) && isCanForwardType) {
    auto forward = std::make_shared<ForwardType>(token);
    stackTable.top().insert(forward);
    p->setPointerBase(forward);
    return p;
  } else {
    throw NotDefinedException(token);
  }
}

ptr_Type SemanticDecl::parseOpenArray(Token declPoint, ptr_Type type) {
  return std::make_shared<OpenArray>(declPoint, std::move(type));
}

std::shared_ptr<FunctionSignature> SemanticDecl::parseFunctionSignature(const Token& t, ListParam params,
                                                                        ptr_Type returnType) {
  return std::make_shared<FunctionSignature>(t, std::move(params), std::move(returnType));
}

ListParam SemanticDecl::parseFormalParamSection(TableSymbol<ptr_Var>& paramTable,
                                                ParamSpec paramSpec, ListToken listId, ptr_Type type) {
  ListParam paramList;
  for (auto& e : listId) {
    if (paramTable.checkContain(e.getString())) {
      throw AlreadyDefinedException(e);
    }
    auto param = std::make_shared<ParamVar>(e, type, paramSpec);
    paramTable.insert(param);
    paramList.push_back(param);
  }
  return paramList;
}

std::shared_ptr<MainFunction> SemanticDecl::parseMainBlock(ptr_Stmt body) {
  stackTable.top().resolveForwardFunction();
  stackTable.top().tableFunction.insert(std::make_shared<Exit>(std::make_shared<Void>()));

  TypeChecker checkType(stackTable);
  body->accept(checkType);
  auto main = std::make_shared<MainFunction>(stackTable.top(), std::move(body));
  stackTable.pop();
  if (!stackTable.isEmpty()) {
    throw std::logic_error("Table Symbol not empty by the end");
  }
  return main;
}

void SemanticDecl::parseFunctionForward(const Token& decl, std::shared_ptr<FunctionSignature> si) {
  if (stackTable.top().checkContain(decl.getString())) {
    throw AlreadyDefinedException(decl);
  }
  auto f =  std::make_shared<ForwardFunction>(std::move(decl), std::move(si));
  stackTable.top().insert(f);
}

void SemanticDecl::parseFunctionDeclBegin(std::shared_ptr<FunctionSignature> s) {
  stackTable.pushEmpty(); // for param variable
  for (auto& e : s->getParamList()) {
    stackTable.top().tableVariable.insert(e);
  }
  stackTable.pushEmpty(); // for decl
}

void SemanticDecl::parseFunctionDeclEnd(const Token& decl,
                                        std::shared_ptr<FunctionSignature> s, ptr_Stmt b) {
  if (!s->isProcedure()) {
    auto nameResult = decl.getString();
    if (s->getParamTable().checkContain(nameResult)) {
      auto& v = s->getParamTable().find(nameResult);
      throw AlreadyDefinedException(v->getDeclPoint(), v->getSymbolName());
    }
    auto result = std::make_shared<ParamVar>(nameResult, s->getReturnType(), ParamSpec::NotSpec);
    stackTable.top().tableVariable.insert(result);
    stackTable.top().tableFunction.insert(std::make_shared<Exit>(s->getReturnType(), result));
  } else {
    stackTable.top().tableFunction.insert(std::make_shared<Exit>(s->getReturnType()));
  }
  TypeChecker checkType(stackTable);
  b->accept(checkType);

  auto declTable = stackTable.top();
  stackTable.pop();
  stackTable.pop();
  auto function = std::make_shared<Function>(decl, s, std::move(b), declTable);
  if (!stackTable.top().checkContain(decl.getString())) {
    stackTable.top().tableFunction.insert(function);
    return;
  } else if (stackTable.isFunction(function->getSymbolName()) &&
             stackTable.findFunction(function->getSymbolName())->isForward()) {
    stackTable.top().tableFunction.replace(function);
  } else {
    throw AlreadyDefinedException(decl);
  }
}

void SemanticDecl::parseConstDecl(const Token& decl, ptr_Expr expr) {
  if (stackTable.top().checkContain(decl.getString())) {
    throw AlreadyDefinedException(decl);
  }
  // auto cons = std::make_shared<Const>(decl);
  // TODO
  //cons->value = expr;
}

void SemanticDecl::parseVariableDecl(ListToken listId, ptr_Type type, bool isGlobal) {
  for (auto& e : listId) {
    if (stackTable.top().checkContain(e.getString())) {
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

void SemanticDecl::parseVariableDecl(ListToken id, ptr_Type type, ptr_Expr def, bool isGlobal) {
  auto name = id.back().getString();
  parseVariableDecl(std::move(id), type, isGlobal);
  // TODO
  //auto& var = stackTable.top().tableVariable.find(name);
  //  var->defaultValue = def;
}