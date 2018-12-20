#include "parser.h"

#include <iostream>
#include <utility>

#include "../exception.h"

using namespace pr;

Parser::Parser(const std::string& s)
  : lexer(s),
    stackTable(createGlobalTable()) {
  assigment = {tok::TokenType::Assignment, tok::TokenType::AssignmentWithMinus,
               tok::TokenType::AssignmentWithPlus,
               tok::TokenType::AssignmentWithAsterisk,
               tok::TokenType::AssignmentWithSlash};
  priority[0] = {tok::TokenType::StrictLess, tok::TokenType::StrictGreater,
                 tok::TokenType::NotEquals, tok::TokenType::Equals,
                 tok::TokenType::LessOrEquals, tok::TokenType::GreaterOrEquals};
  priority[1] = {tok::TokenType::Minus, tok::TokenType::Plus,
                 tok::TokenType::Or, tok::TokenType::Xor};
  priority[2] = {tok::TokenType::Asterisk, tok::TokenType::Slash,
                 tok::TokenType::Div, tok::TokenType::Mod,
                 tok::TokenType::And,
                 tok::TokenType::Shr, tok::TokenType::ShiftRight, // TODO: replace one tokenType
                 tok::TokenType::Shl, tok::TokenType::ShiftLeft};
  priority[3] = {tok::TokenType::Not, tok::TokenType::Minus,
                 tok::TokenType::Plus, tok::TokenType::At};
  priorityLeftUnary  = 3;
  priority[4] = {tok::TokenType::Caret, tok::TokenType::Dot,
                 tok::TokenType::OpenSquareBracket,
                 tok::TokenType::OpenParenthesis};
  priorityAccess = 4;
}

ptr_Node Parser::parseProgram() {
  return parseMainBlock();
}

ptr_Expr Parser::parseExpression() {
  return parseBinaryOperator();
}


ListExpr Parser::parseListExpression() {
  ListExpr list;
  list.push_back(parseBinaryOperator());
  while(match(tok::TokenType::Comma)) {
    ++lexer;
    list.push_back(parseBinaryOperator());
  }
  return list;
}

ListToken Parser::parseListId() {
  ListToken list;
  require(tok::TokenType::Id);
  list.push_back(lexer.next());
  while (match(tok::TokenType::Comma)) {
    ++lexer;
    require(tok::TokenType::Id);
    list.push_back(lexer.next());
  }
  return list;
}

ListExpr Parser::parseActualParameter() {
  ListExpr list;
  if (match(tok::TokenType::CloseParenthesis)) {
    return list;
  }
  return parseListExpression();
}

ListSymbol Parser::parseFormalParameterList() {
  requireAndSkip(tok::TokenType::OpenParenthesis);
  if (match(tok::TokenType::CloseParenthesis)) {
    ++lexer;
    return ListSymbol();
  }
  TableSymbol paramTable;
  ListSymbol paramList(parseFormalParamSection(paramTable));
  while (match(tok::TokenType::Comma)) {
    ++lexer;
    paramList.splice(paramList.end(), parseFormalParamSection(paramTable));
  }
  requireAndSkip(tok::TokenType::CloseParenthesis);
  return paramList;
}

ptr_Expr Parser::parseFactor() {
  auto token = lexer.next();
  auto type = token->getTokenType();
  switch (type) {
    case tok::TokenType::Int:
    case tok::TokenType::String:
    case tok::TokenType::Double:
    case tok::TokenType::Nil: {
      return std::make_unique<Literal>(std::move(token));
    }
    case tok::TokenType::Id: {
      return std::make_unique<Variable>(std::move(token));
    }
    case tok::TokenType::OpenParenthesis: {
      auto expr = parseBinaryOperator();
      requireAndSkip(tok::TokenType::CloseParenthesis);
      return expr;
    }
    default:
      throw ParserException(token->getLine(), token->getColumn(), type);
  }
}

ptr_Expr Parser::parseAccess(int p) {
  auto left = getExprByPriority(p + 1);

  while (match(priority[p])) {
    switch (lexer.get()->getTokenType()) {
      case tok::TokenType::Dot: {
        ++lexer;
        require(tok::TokenType::Id);
        left = std::make_unique<RecordAccess>(std::move(left), lexer.next());
        break;
      }
      case tok::TokenType::OpenParenthesis: {
        ++lexer;
        auto list = parseActualParameter();
        requireAndSkip(tok::TokenType::CloseParenthesis);
        left = std::make_unique<FunctionCall>(std::move(left), std::move(list));
        break;
      }
      case tok::TokenType::OpenSquareBracket: {
        ++lexer;
        auto list = parseListExpression();
        requireAndSkip(tok::TokenType::CloseSquareBracket);
        left = std::make_unique<ArrayAccess>(std::move(left), std::move(list));
        break;
      }
      case tok::TokenType::Caret: {
        left = std::make_unique<UnaryOperation>(lexer.next(), std::move(left));
        break;
      }
      default:
        break;
    }
  }

  return left;
}

ptr_Expr Parser::parseUnaryOperator(int p) {
  if (!match(priority[p])) {
      return getExprByPriority(p + 1);
  }
  auto op = lexer.next();
  return std::make_unique<UnaryOperation>(std::move(op), parseUnaryOperator(p));
}

ptr_Expr Parser::parseBinaryOperator(int p) {
  auto left = getExprByPriority(p + 1);

  while (match(priority[p])) {
    auto op = lexer.next();
    auto right = getExprByPriority(p + 1);
    left = std::make_unique<BinaryOperation>(std::move(op), std::move(left), std::move(right));
  }
  return left;
}

ptr_Expr Parser::getExprByPriority(int p) {
  if (p == priority.size()) {
    return parseFactor();
  } else if (p == priorityLeftUnary) {
    return parseUnaryOperator(p);
  } else if (p == priorityAccess) {
    return parseAccess(p);
  } else {
    return parseBinaryOperator(p);
  }
}

ptr_Stmt Parser::parseStatement() {
  switch (lexer.get()->getTokenType()) {
    case tok::TokenType::Begin: {
      return parseCompound();
    }
    case tok::TokenType::If: {
      return parseIf();
    }
    case tok::TokenType::While: {
      return parseWhile();
    }
    case tok::TokenType::For: {
      return parseFor();
    }
    case tok::TokenType::Break: case tok::TokenType::Continue: {
      auto t = lexer.next();
      if (isInsideLoop) {
        if (t->getTokenType() == tok::TokenType::Break) {
          return std::make_unique<BreakStmt>();
        }
        else {
          return std::make_unique<ContinueStmt>();
        }
      }
      throw ParserException(t->getLine(), t->getColumn(), t->getValueString() + " out of cycle");
    }
    default: {
      auto right = parseExpression();
      if (match(assigment)) {
        auto op = lexer.next();
        return std::make_unique<AssignmentStmt>(std::move(op), std::move(right), parseExpression());
      }
      return std::make_unique<FunctionCallStmt>(std::move(right));
    }
  }
}

ptr_Stmt Parser::parseCompound() {
  requireAndSkip(tok::TokenType::Begin);
  ListStmt list;
  while (!match(tok::TokenType::End)) {
    list.push_back(parseStatement());
    if (!match(tok::TokenType::Semicolon)) { break; }
    ++lexer;
  }
  requireAndSkip(tok::TokenType::End);
  return std::make_unique<BlockStmt>(std::move(list));
}

ptr_Stmt Parser::parseIf() {
  requireAndSkip(tok::TokenType::If);
  auto cond = parseExpression();
  requireAndSkip(tok::TokenType::Then);
  auto then = parseStatement();
  if (match(tok::TokenType::Else)) {
    ++lexer;
    return std::make_unique<IfStmt>(std::move(cond), std::move(then), parseStatement());
  }
  return std::make_unique<IfStmt>(std::move(cond), std::move(then));
}

ptr_Stmt Parser::parseWhile() {
  requireAndSkip(tok::TokenType::While);
  auto cond = parseExpression();
  requireAndSkip(tok::TokenType::Do);
  bool isFirstLoop = !isInsideLoop;
  isInsideLoop = true;
  auto block = parseStatement();
  isInsideLoop = !isFirstLoop;
  return std::make_unique<WhileStmt>(std::move(cond), std::move(block));
}

ptr_Stmt Parser::parseFor() {
  requireAndSkip(tok::TokenType::For);
  require(tok::TokenType::Id);
  auto var = std::make_unique<Variable>(lexer.next());
  requireAndSkip(tok::TokenType::Assignment);
  auto low = parseExpression();
  require({tok::TokenType::To, tok::TokenType::Downto}, "\"to\" or \"downto\"");
  bool dir = match(tok::TokenType::To);
  ++lexer;
  auto high = parseExpression();
  requireAndSkip(tok::TokenType::Do);
  bool isFirstLoop = !isInsideLoop;
  isInsideLoop = true;
  auto block = parseStatement();
  isInsideLoop = !isFirstLoop;
  return std::make_unique<ForStmt>(std::move(var), std::move(low),
                                   std::move(high), dir, std::move(block));
}

ptr_Symbol Parser::parseMainBlock() {
  auto main = std::make_shared<MainFunction>();
  parseDecl(true);
  main->decl = stackTable.top();

  stackTable.pop();
  if (!stackTable.isEmpty()) {
    throw std::logic_error("Table Symbol not empty by the end");
  }
  main->body = parseCompound();
  requireAndSkip(tok::TokenType::Dot);
  return main;
}

void Parser::parseDecl(bool isMainBlock) {
  while(true) {
    switch (lexer.get()->getTokenType()) {
      case tok::TokenType::Var: {
        parseVarDecl();
        break;
      }
      case tok::TokenType::Const: {
        parseConstDecl();
        break;
      }
      case tok::TokenType::Type: {
        parseTypeDecl();
        break;
      }
      case tok::TokenType::Function:
      case tok::TokenType::Procedure: {
        auto t = lexer.next();
        if (!isMainBlock) {
          throw ParserException(t->getLine(), t->getColumn(), tok::toString(t->getTokenType()));
        }
        bool isP = t->getTokenType() == tok::TokenType::Procedure;
        lexer.push_back(std::move(t));
        parseFunctionDecl(isP);
        break;
      }
      default: {
        return;
      }
    }
  }
}

void Parser::parseVarDecl() {
  requireAndSkip(tok::TokenType::Var);
  require(tok::TokenType::Id);
  while (match(tok::TokenType::Id)) {
    parseVariableDecl();
    requireAndSkip(tok::TokenType::Semicolon);
  }
}

void Parser::parseVariableDecl() {
  auto listId = parseListId();
  requireAndSkip(tok::TokenType::Colon);
  parseType();
  if (match(tok::TokenType::Equals)) {
    if (listId.size() > 1) {
      throw ParserException(lexer.get()->getLine(), lexer.get()->getColumn(), lexer.get()->getTokenType());
    }
    ++lexer;
    parseExpression();
  }
}

void Parser::parseConstDecl() {
  requireAndSkip(tok::TokenType::Const);
  require(tok::TokenType::Id);
  while (match(tok::TokenType::Id)) {
    ++lexer;
    requireAndSkip(tok::TokenType::Equals);
    parseExpression();
    requireAndSkip(tok::TokenType::Semicolon);
  }
}

void Parser::parseTypeDecl() {
  requireAndSkip(tok::TokenType::Type);
  require(tok::TokenType::Id);
  while (match(tok::TokenType::Id)) {
    auto id = lexer.next();
    auto alias = std::make_shared<Alias>(id);
    requireAndSkip(tok::TokenType::Equals);
    alias->type = parseType(true);
    requireAndSkip(tok::TokenType::Semicolon);

    if (!stackTable.checkContain(alias->name)) {
      stackTable.top().tableType.insert(alias);

    } else if (stackTable.top().tableType.checkContain(alias->name) &
               stackTable.top().tableType.find(alias->name)->isForward) {
      stackTable.top().tableType.replace(alias);

    } else {
      throw AlreadyDefinedException(id);
    }
  }
  stackTable.top().resolveForwardType();
}

void Parser::parseFunctionDecl(bool isProcedure) {
  require({tok::TokenType::Function, tok::TokenType::Procedure}, "\"procedure\" or \"function\"");
  ++lexer;
  require(tok::TokenType::Id);
  ++lexer;
  parseFunctionSignature(isProcedure);
  requireAndSkip(tok::TokenType::Semicolon);
  if (match(tok::TokenType::Id)) {
    auto id(lexer.next());
    if (id->getValueString() == "forward") {
    }
    else  {
      throw ParserException(id->getLine(), id->getColumn(), "forward", id->getValueString());
    }
  } else {
    parseDecl();
    parseCompound();
  }
  requireAndSkip(tok::TokenType::Semicolon);
}

ptr_Symbol Parser::parseType(bool isTypeDecl) {
  switch (lexer.get()->getTokenType()) {
    case tok::TokenType::Id: {
      return parseSimpleType();
    }
    case tok::TokenType::Array: {
      return parseArrayType();
    }
    case tok::TokenType::Record: {
      return parseRecordType();
    }
    case tok::TokenType::Function: {
      ++lexer;
      return parseFunctionSignature();
    }
    case tok::TokenType::Procedure: {
      ++lexer;
      return parseFunctionSignature(true);
    }
    case tok::TokenType::Caret: {
      return parsePointer(isTypeDecl);
    }
    default : {
      auto& g = lexer.get();
      throw ParserException(g->getLine(), g->getColumn(), g->getTokenType());
    }
  }
}

ptr_Symbol Parser::parseSimpleType() {
  require(tok::TokenType::Id);
  auto t = lexer.next();
  if (stackTable.isType(t->getValueString())) {
    return stackTable.findType(t->getValueString());
  } else {
    throw NotDefinedException(t);
  }
}

ptr_Symbol Parser::parsePointer(bool isTypeDecl) {
  auto p = std::make_shared<Pointer>(lexer.get());
  requireAndSkip(tok::TokenType::Caret);
  require(tok::TokenType::Id);
  auto token = lexer.next();

  if (stackTable.isType(token->getValueString())) {
    p->typeBase = stackTable.findType(token->getValueString());
    return p;

  } else if (!stackTable.checkContain(token->getValueString()) && isTypeDecl) {
    auto forward = std::make_shared<ForwardType>(token);
    stackTable.top().insert(forward);
    p->typeBase = forward;
    return p;

  } else {
    throw NotDefinedException(token);
  }
}

std::pair<int, int> Parser::parseRangeType() {
  require(tok::TokenType::Int);
  int low = lexer.next()->getInt();
  requireAndSkip(tok::TokenType::DoubleDot);
  require(tok::TokenType::Int);
  int high = lexer.next()->getInt();
  return std::make_pair(low, high);
}

std::shared_ptr<Symbol> Parser::parseArrayType() {
  auto staticArray = std::make_shared<StaticArray>(lexer.get());

  requireAndSkip(tok::TokenType::Array);
  requireAndSkip(tok::TokenType::OpenSquareBracket);

  staticArray->bounds.push_back(parseRangeType());
  while (match(tok::TokenType::Comma)) {
    ++lexer;
    staticArray->bounds.push_back(parseRangeType());
  }
  requireAndSkip(tok::TokenType::CloseSquareBracket);
  requireAndSkip(tok::TokenType::Of);
  staticArray->typeElem = parseType();
  return staticArray;
}

std::shared_ptr<Symbol> Parser::parseRecordType() {
  auto record = std::make_shared<Record>(lexer.get());

  requireAndSkip(tok::TokenType::Record);
  while (!match(tok::TokenType::End)) {
    ListToken listId = parseListId();
    requireAndSkip(tok::TokenType::Colon);
    auto type = parseType();

    for (auto& e : listId) {
     if (record->fields.checkContain(e->getValueString())) {
       throw AlreadyDefinedException(e);
     }
     record->fields.insert(std::make_shared<LocalVar>(e, type));
    }

    if (!match(tok::TokenType::Semicolon)) {
      break;
    }
    ++lexer;
  }
  requireAndSkip(tok::TokenType::End);
  return record;
}

ptr_Symbol Parser::parseParameterType() {
  switch (lexer.get()->getTokenType()) {
    case tok::TokenType::Id: {
      return parseSimpleType();
    }
    case tok::TokenType::Array: {
      auto openArray = std::make_shared<OpenArray>(lexer.next());
      requireAndSkip(tok::TokenType::Of);
      openArray->typeElem =  parseParameterType();
      return openArray;
    }
    default: {
      auto g = lexer.next();
      throw ParserException(g->getLine(), g->getColumn(),
        tok::TokenType::CloseParenthesis, g->getTokenType());
    }
  }
}

ptr_Symbol Parser::parseFunctionSignature(bool isProcedure) {
  auto proc = std::make_shared<FunctionSignature>(lexer.get());
  if (isProcedure) {
    if (match(tok::TokenType::OpenParenthesis)) {
      proc->setParamsList(parseFormalParameterList());
    }
  }
  else {
    if (!match(tok::TokenType::Colon)) {
      proc->setParamsList(parseFormalParameterList());
    }
    requireAndSkip(tok::TokenType::Colon);
    proc->returnType = parseSimpleType();
  }
  return proc;
}

ListSymbol Parser::parseFormalParamSection(TableSymbol& paramTable) {
  ParamSpec paramSpec;
  switch (lexer.get()->getTokenType()) {
    case tok::TokenType::Var: {
      ++lexer;
      paramSpec = ParamSpec::Var;
      break;
    }
    case tok::TokenType::Const: {
      ++lexer;
      paramSpec = ParamSpec::Const;
      break;
    }
    case tok::TokenType::Out: {
      ++lexer;
      paramSpec = ParamSpec::Out;
      break;
    }
    default: {
      paramSpec = ParamSpec::NotSpec;
      break;
    }
  }
  auto listId = parseListId();
  ListSymbol paramList;
  requireAndSkip(tok::TokenType::Colon);
  auto type = parseParameterType();
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

bool Parser::match(const std::list<tok::TokenType>& listType) {
  for (auto& exceptType: listType) {
    if (match(exceptType)) { return true; }
  }
  return false;
}

bool Parser::match(tok::TokenType t) {
  return t == lexer.get()->getTokenType();
}

void Parser::require(tok::TokenType type) {
  auto& get = lexer.get();
  if (get->getTokenType() != type) {
    throw ParserException(get->getLine(), get->getColumn(), type, get->getTokenType());
  }
}

void Parser::require(const std::list<tok::TokenType>& listType, const std::string& s) {
  if (!match(listType)) {
    auto& g = lexer.get();
    throw ParserException(g->getLine(), g->getColumn(), s, tok::toString(g->getTokenType()));
  }
}

void Parser::requireAndSkip(tok::TokenType t) {
  require(t);
  ++lexer;
}