#include "parser.h"

#include <iostream>
#include <utility>
#include <algorithm>
#include <bits/move.h>
#include <stack>

#include "../exception.h"
#include "type_checker.h"


Parser::Parser(const std::string& s)
  : lexer(s),
    semanticDecl() {
  assigment = {TokenType::Assignment, TokenType::AssignmentWithMinus,
               TokenType::AssignmentWithPlus,
               TokenType::AssignmentWithAsterisk,
               TokenType::AssignmentWithSlash};
  priority[0] = {TokenType::StrictLess, TokenType::StrictGreater,
                 TokenType::NotEquals, TokenType::Equals,
                 TokenType::LessOrEquals, TokenType::GreaterOrEquals};
  priority[1] = {TokenType::Minus, TokenType::Plus,
                 TokenType::Or, TokenType::Xor};
  priority[2] = {TokenType::Asterisk, TokenType::Slash,
                 TokenType::Div, TokenType::Mod,
                 TokenType::And,
                 TokenType::Shr, TokenType::ShiftRight, // TODO: replace one tokenType
                 TokenType::Shl, TokenType::ShiftLeft};
  priority[3] = {TokenType::Not, TokenType::Minus,
                 TokenType::Plus, TokenType::At};
  priorityLeftUnary  = 3;
  priority[4] = {TokenType::Caret, TokenType::Dot,
                 TokenType::OpenSquareBracket,
                 TokenType::OpenParenthesis};
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
  while(match(TokenType::Comma)) {
    ++lexer;
    list.push_back(parseBinaryOperator());
  }
  return list;
}

std::list<Token> Parser::parseListId() {
  std::list<Token> list;
  require(TokenType::Id);
  list.push_back(lexer.next());
  while (match(TokenType::Comma)) {
    ++lexer;
    require(TokenType::Id);
    list.push_back(lexer.next());
  }
  return list;
}

ListExpr Parser::parseActualParameter() {
  ListExpr list;
  if (match(TokenType::CloseParenthesis)) {
    return list;
  }
  return parseListExpression();
}

ListParam Parser::parseFormalParameterList() {
  requireAndSkip(TokenType::OpenParenthesis);
  if (match(TokenType::CloseParenthesis)) {
    ++lexer;
    return ListParam();
  }
  TableSymbol<ptr_Var> paramTable;
  ListParam paramList(parseFormalParamSection(paramTable));
  while (match(TokenType::Comma)) {
    ++lexer;
    paramList.splice(paramList.end(), parseFormalParamSection(paramTable));
  }
  requireAndSkip(TokenType::CloseParenthesis);
  return paramList;
}

ptr_Expr Parser::parseFactor() {
  auto token = lexer.next();
  auto type = token.getTokenType();
  switch (type) {
    case TokenType::Int:
    case TokenType::String:
    case TokenType::Double:
    case TokenType::Nil:
    case TokenType::False:
    case TokenType::True: {
      return std::make_unique<Literal>(std::move(token));
    }
    case TokenType::Id: {
      return std::make_unique<Variable>(std::move(token));
    }
    case TokenType::OpenParenthesis: {
      auto expr = parseBinaryOperator();
      requireAndSkip(TokenType::CloseParenthesis);
      return expr;
    }
    default:
      throw ParserException(token.getLine(), token.getColumn(), type);
  }
}

ptr_Expr Parser::parseAccess(int p) {
  auto left = getExprByPriority(p + 1);

  while (match(priority[p])) {
    switch (lexer.get().getTokenType()) {
      case TokenType::Dot: {
        auto d = lexer.next();
        require(TokenType::Id);
        left = std::make_unique<RecordAccess>(d, std::move(left), lexer.next());
        break;
      }
      case TokenType::OpenParenthesis: {
        auto d = lexer.next();
        auto list = parseActualParameter();
        requireAndSkip(TokenType::CloseParenthesis);

        left = semanticDecl.parseFunctionCall(d, std::move(left), std::move(list));
        break;
      }
      case TokenType::OpenSquareBracket: {
        auto d = lexer.next();
        auto list = parseListExpression();
        requireAndSkip(TokenType::CloseSquareBracket);
        left = std::make_unique<ArrayAccess>(d, std::move(left), std::move(list));
        break;
      }
      case TokenType::Caret: {
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

// TODO replace this
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
  switch (lexer.get().getTokenType()) {
    case TokenType::Begin: {
      return parseCompound();
    }
    case TokenType::If: {
      return parseIf();
    }
    case TokenType::While: {
      return parseWhile();
    }
    case TokenType::For: {
      return parseFor();
    }
    case TokenType::Break: case TokenType::Continue: {
      auto t = lexer.next();
      if (isInsideLoop) {
        if (t.getTokenType() == TokenType::Break) {
          return std::make_unique<BreakStmt>();
        }
        else {
          return std::make_unique<ContinueStmt>();
        }
      }
      throw ParserException(t.getLine(), t.getColumn(), t.getString() + " out of cycle");
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
  requireAndSkip(TokenType::Begin);
  ListStmt list;
  while (!match(TokenType::End)) {
    list.push_back(parseStatement());
    if (!match(TokenType::Semicolon)) { break; }
    ++lexer;
  }
  requireAndSkip(TokenType::End);
  return std::make_unique<BlockStmt>(std::move(list));
}

ptr_Stmt Parser::parseIf() {
  requireAndSkip(TokenType::If);
  auto cond = parseExpression();
  requireAndSkip(TokenType::Then);
  auto then = parseStatement();
  if (match(TokenType::Else)) {
    ++lexer;
    return std::make_unique<IfStmt>(std::move(cond), std::move(then), parseStatement());
  }
  return std::make_unique<IfStmt>(std::move(cond), std::move(then));
}

ptr_Stmt Parser::parseWhile() {
  requireAndSkip(TokenType::While);
  auto cond = parseExpression();
  requireAndSkip(TokenType::Do);
  bool isFirstLoop = !isInsideLoop;
  isInsideLoop = true;
  auto block = parseStatement();
  isInsideLoop = !isFirstLoop;
  return std::make_unique<WhileStmt>(std::move(cond), std::move(block));
}

ptr_Stmt Parser::parseFor() {
  requireAndSkip(TokenType::For);
  require(TokenType::Id);
  auto var = std::make_unique<Variable>(lexer.next());
  requireAndSkip(TokenType::Assignment);
  auto low = parseExpression();
  require({TokenType::To, TokenType::Downto}, "\"to\" or \"downto\"");
  bool dir = match(TokenType::To);
  ++lexer;
  auto high = parseExpression();
  requireAndSkip(TokenType::Do);
  bool isFirstLoop = !isInsideLoop;
  isInsideLoop = true;
  auto block = parseStatement();
  isInsideLoop = !isFirstLoop;
  return std::make_unique<ForStmt>(std::move(var), std::move(low),
                                   std::move(high), dir, std::move(block));
}

std::shared_ptr<MainFunction> Parser::parseMainBlock() {
  auto main = std::make_shared<MainFunction>();
  parseDecl(true);
  auto body = parseCompound();
  requireAndSkip(TokenType::Dot);
  return semanticDecl.parseMainBlock(std::move(body));
}

void Parser::parseDecl(bool isMainBlock) {
  while(true) {
    switch (lexer.get().getTokenType()) {
      case TokenType::Var: {
        parseVarDecl(isMainBlock);
        break;
      }
      case TokenType::Const: {
        parseConstDecl();
        break;
      }
      case TokenType::Type: {
        parseTypeDecl();
        break;
      }
      case TokenType::Function:
      case TokenType::Procedure: {
        auto t = lexer.next();
        if (!isMainBlock) {
          throw ParserException(t.getLine(), t.getColumn(), toString(t.getTokenType()));
        }
        bool isP = t.getTokenType() == TokenType::Procedure;
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

void Parser::parseVarDecl(bool isMainBlock) {
  requireAndSkip(TokenType::Var);
  require(TokenType::Id);
  while (match(TokenType::Id)) {
    parseVariableDecl(isMainBlock);
    requireAndSkip(TokenType::Semicolon);
  }
}

void Parser::parseVariableDecl(bool isMainBlock) {
  auto listId = parseListId();
  requireAndSkip(TokenType::Colon);
  auto type = parseType();
  if (match(TokenType::Equals)) {
    if (listId.size() > 1) {
      throw ParserException(lexer.get().getLine(), lexer.get().getColumn(), lexer.get().getTokenType());
    }
    ++lexer;
    auto def = parseExpression();
    semanticDecl.parseVariableDecl(std::move(listId), type, std::move(def), isMainBlock);
    return;
  }
  semanticDecl.parseVariableDecl(std::move(listId), type, isMainBlock);
}

void Parser::parseConstDecl() {
  requireAndSkip(TokenType::Const);
  require(TokenType::Id);
  while (match(TokenType::Id)) {
    auto id = lexer.next();
    requireAndSkip(TokenType::Equals);
    auto expression = parseExpression();
    requireAndSkip(TokenType::Semicolon);
    semanticDecl.parseConstDecl(id, std::move(expression));
  }
}

void Parser::parseTypeDecl() {
  requireAndSkip(TokenType::Type);
  require(TokenType::Id);
  while (match(TokenType::Id)) {
    auto id = lexer.next();
    requireAndSkip(TokenType::Equals);
    auto type = parseType(true);
    requireAndSkip(TokenType::Semicolon);
    semanticDecl.parseTypeDecl(std::move(id), std::move(type));
  }
  semanticDecl.parseTypeDeclEnd();
}

void Parser::parseFunctionDecl(bool isProcedure) {
  require({TokenType::Function, TokenType::Procedure}, "\"procedure\" or \"function\"");
  ++lexer;
  require(TokenType::Id);
  auto declPoint =lexer.next();
  auto signature = parseFunctionSignature(isProcedure);
  requireAndSkip(TokenType::Semicolon);
  if (match(TokenType::Id)) {
    auto idType = lexer.next();
    if (idType.getString() == "forward") {
      semanticDecl.parseFunctionForward(std::move(declPoint), std::move(signature));
    }
    else  {
      throw ParserException(idType.getLine(), idType.getColumn(), "forward", idType.getString());
    }
  } else {
    semanticDecl.parseFunctionDeclBegin(signature);
    parseDecl();
    auto body = parseCompound();
    semanticDecl.parseFunctionDeclEnd(declPoint, signature, std::move(body));
  }
  requireAndSkip(TokenType::Semicolon);
}

ptr_Type Parser::parseType(bool isTypeDecl) {
  switch (lexer.get().getTokenType()) {
    case TokenType::Id: {
      return parseSimpleType();
    }
    case TokenType::Array: {
      return parseArrayType();
    }
    case TokenType::Record: {
      return parseRecordType();
    }
    case TokenType::Function: {
      ++lexer;
      return parseFunctionSignature();
    }
    case TokenType::Procedure: {
      ++lexer;
      return parseFunctionSignature(true);
    }
    case TokenType::Caret: {
      return parsePointer(isTypeDecl);
    }
    default : {
      auto& g = lexer.get();
      throw ParserException(g.getLine(), g.getColumn(), g.getTokenType());
    }
  }
}

ptr_Type Parser::parseSimpleType() {
  require(TokenType::Id);
  return semanticDecl.parseSimpleType(lexer.next());
}

ptr_Type Parser::parsePointer(bool isTypeDecl) {
  require(TokenType::Caret);
  auto declPoint = lexer.next();
  require(TokenType::Id);
  auto token = lexer.next();
  return semanticDecl.parsePointer(std::move(declPoint), std::move(token), isTypeDecl);
}

std::pair<int, int> Parser::parseRangeType() {
  require(TokenType::Int);
  int low = lexer.next().getInt();
  requireAndSkip(TokenType::DoubleDot);
  require(TokenType::Int);
  int high = lexer.next().getInt();
  return std::make_pair(low, high);
}

ptr_Type Parser::parseArrayType() {
  require(TokenType::Array);
  auto declPoint = lexer.next();
  requireAndSkip(TokenType::OpenSquareBracket);
  StaticArray::BoundsType bounds;
  bounds.push_back(parseRangeType());
  while (match(TokenType::Comma)) {
    ++lexer;
    bounds.push_back(parseRangeType());
  }
  requireAndSkip(TokenType::CloseSquareBracket);
  requireAndSkip(TokenType::Of);
  auto typeElem = parseType();
  return semanticDecl.parseArrayType(std::move(declPoint), std::move(bounds), std::move(typeElem));
}

ptr_Type Parser::parseRecordType() {
  require(TokenType::Record);
  auto declPoint = lexer.next();
  std::list<std::pair<std::unique_ptr<std::list<Token>>, ptr_Type>> listVar;
  while (!match(TokenType::End)) {
    auto listId = std::make_unique<std::list<Token>>(parseListId());
    requireAndSkip(TokenType::Colon);
    auto type = parseType();
    auto pair = std::make_pair(std::move(listId), type);
    listVar.push_back(std::move(pair));
    if (!match(TokenType::Semicolon)) {
      break;
    }
    ++lexer;
  }
  requireAndSkip(TokenType::End);
  return semanticDecl.parseRecordType(std::move(declPoint), std::move(listVar));
}

ptr_Type Parser::parseParameterType() {
  switch (lexer.get().getTokenType()) {
    case TokenType::Id: {
      return parseSimpleType();
    }
    case TokenType::Array: {
      auto declPoint = lexer.next();
      requireAndSkip(TokenType::Of);
      auto typeElem =  parseSimpleType();
      return semanticDecl.parseOpenArray(std::move(declPoint), std::move(typeElem));
    }
    default: {
      auto g = lexer.next();
      throw ParserException(g.getLine(), g.getColumn(),
        TokenType::CloseParenthesis, g.getTokenType());
    }
  }
}

std::shared_ptr<FunctionSignature> Parser::parseFunctionSignature(bool isProcedure) {
  ListParam param;
  int line = lexer.get().getLine();
  int column = lexer.get().getColumn();
  if (isProcedure) {
    if (match(TokenType::OpenParenthesis)) {
      param = parseFormalParameterList();
    }
    return semanticDecl.parseFunctionSignature(line, column, std::move(param), std::make_shared<Void>());
  }
  else {
    if (!match(TokenType::Colon)) {
      param = parseFormalParameterList();
    }
    requireAndSkip(TokenType::Colon);
    auto returnType = parseSimpleType();
    return semanticDecl.parseFunctionSignature(line, column, std::move(param), std::move(returnType));
  }
}

ListParam Parser::parseFormalParamSection(TableSymbol<ptr_Var>& paramTable) {
  ParamSpec paramSpec;
  switch (lexer.get().getTokenType()) {
    case TokenType::Var: {
      ++lexer;
      paramSpec = ParamSpec::Var;
      break;
    }
    case TokenType::Const: {
      ++lexer;
      paramSpec = ParamSpec::Const;
      break;
    }
    case TokenType::Out: {
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
  requireAndSkip(TokenType::Colon);
  auto type = parseParameterType();
  return semanticDecl.parseFormalParamSection(paramTable, paramSpec, std::move(listId), std::move(type));
}

bool Parser::match(const std::list<TokenType>& listType) {
  for (auto& exceptType: listType) {
    if (match(exceptType)) { return true; }
  }
  return false;
}

bool Parser::match(TokenType t) {
  return t == lexer.get().getTokenType();
}

void Parser::require(TokenType type) {
  auto& get = lexer.get();
  if (get.getTokenType() != type) {
    throw ParserException(get.getLine(), get.getColumn(), type, get.getTokenType());
  }
}

void Parser::require(const std::list<TokenType>& listType, const std::string& s) {
  if (!match(listType)) {
    auto& g = lexer.get();
    throw ParserException(g.getLine(), g.getColumn(), s, toString(g.getTokenType()));
  }
}

void Parser::requireAndSkip(TokenType t) {
  require(t);
  ++lexer;
}