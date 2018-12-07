#include <iostream>
#include "parser.h"
#include "../exception.h"

using namespace pr;

Parser::Parser(const std::string& s)
  : lexer(s) {
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

void Parser::parseListId() {
  requireAndSkip(tok::TokenType::Id);
  while (match(tok::TokenType::Comma)) {
    ++lexer;
    requireAndSkip(tok::TokenType::Id);
  }
}

ListExpr Parser::parseActualParameter() {
  ListExpr list;
  if (match(tok::TokenType::CloseParenthesis)) {
    return list;
  }
  return parseListExpression();
}

void Parser::parseFormalParameterList() {
  requireAndSkip(tok::TokenType::OpenParenthesis);
  if (match(tok::TokenType::CloseParenthesis)) {
    ++lexer;
    return ;
  }
  parseFormalParamSection();
  while (match(tok::TokenType::Comma)) {
    ++lexer;
    parseFormalParamSection();
  }
  requireAndSkip(tok::TokenType::CloseParenthesis);
  return;
}

ptr_Expr Parser::parseFactor() {
  auto token = lexer.next();
  auto type = token->getTokenType();
  switch (type) {
    case tok::TokenType::Int:
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

ptr_Stmt Parser::parseMainBlock() {
  auto main = parseCompound();
  requireAndSkip(tok::TokenType::Dot);
  return main;
}

void Parser::parseType() {
  switch (lexer.get()->getTokenType()) {
    case tok::TokenType::Id: {
      ++lexer;
      return;
    }
    case tok::TokenType::Array: {
      parseArrayType();
      return;
    }
    case tok::TokenType::Record: {
      parseRecordType();
      return;
    }
    case tok::TokenType::Function: {
      ++lexer;
      parseFunctionSignature();
      return;
    }
    case tok::TokenType::Procedure: {
      ++lexer;
      parseFunctionSignature(true);
      return;
    }
    case tok::TokenType::Caret: {
      ++lexer;
      parseType();
      return;
    }
    default : {
      auto& g = lexer.get();
      throw ParserException(g->getLine(), g->getColumn(), g->getTokenType());
    }
  }
}

void Parser::parseRangeType() {
  require(tok::TokenType::Int);
  ++lexer;
  requireAndSkip(tok::TokenType::DoubleDot);
  require(tok::TokenType::Int);
  ++lexer;
  return;
}

void Parser::parseArrayType() {
  requireAndSkip(tok::TokenType::Array);
  if (!match(tok::TokenType::Of)) {
    requireAndSkip(tok::TokenType::OpenSquareBracket);
    parseRangeType();
    while (match(tok::TokenType::Comma)) {
      ++lexer;
      parseRangeType();
    }
    requireAndSkip(tok::TokenType::CloseSquareBracket);
  }
  requireAndSkip(tok::TokenType::Of);
  parseType();

  return;
}

void Parser::parseRecordType() {
  requireAndSkip(tok::TokenType::Record);
  while (!match(tok::TokenType::End)) {
    parseIdListAndType();
    if (!match(tok::TokenType::Semicolon)) {
      break;
    }
    ++lexer;
  }
  requireAndSkip(tok::TokenType::End);
  return;
}

void Parser::parseFunctionSignature(bool isProcedure) {
  if (isProcedure) {
    if (match(tok::TokenType::OpenParenthesis)) {
      parseFormalParameterList();
    }
  }
  else {
    if (!match(tok::TokenType::Colon)) {
      parseFormalParameterList();
    }
    requireAndSkip(tok::TokenType::Colon);
    parseType();
  }

}

void Parser::parseIdListAndType() {
  parseListId();
  requireAndSkip(tok::TokenType::Colon);
  parseType();
}

void Parser::parseFormalParamSection() {
  switch (lexer.get()->getTokenType()) {
    case tok::TokenType::Var: {
      ++lexer;
      parseIdListAndType();
      return;
    }
    case tok::TokenType::Const: {
      ++lexer;
      parseIdListAndType();
      return;
    }
    case tok::TokenType::Out: {
      ++lexer;
      parseIdListAndType();
      return;
    }
    default: {
      parseIdListAndType();
      return;
    }
  }
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