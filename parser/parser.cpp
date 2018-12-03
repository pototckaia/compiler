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
  while(match({tok::TokenType::Comma})) {
    lexer.next();
    list.push_back(parseBinaryOperator());
  }
  return list;
}

ListExpr Parser::parseActualParameter() {
  ListExpr list;
  if (lexer.get()->getTokenType() == tok::TokenType::CloseParenthesis) {
    return list;
  }
  return parseListExpression();
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
      require(tok::TokenType::CloseParenthesis);
      lexer.next();
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
        lexer.next();
        require(tok::TokenType::Id);
        left = std::make_unique<RecordAccess>(std::move(left), lexer.next());
        break;
      }
      case tok::TokenType::OpenParenthesis: {
        lexer.next();
        auto list = parseActualParameter();
        require(tok::TokenType::CloseParenthesis);
        lexer.next();
        left = std::make_unique<FunctionCall>(std::move(left), std::move(list));
        break;
      }
      case tok::TokenType::OpenSquareBracket: {
        lexer.next();
        auto list = parseListExpression();
        require(tok::TokenType::CloseSquareBracket);
        lexer.next();
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
  require(tok::TokenType::Begin);
  ++lexer;
  ListStmt list;
  while (!match({tok::TokenType::End})) {
    list.push_back(parseStatement());
    if (!match({tok::TokenType::Semicolon})) { break; }
    ++lexer;
  }
  require(tok::TokenType::End);
  ++lexer;
  return std::make_unique<BlockStmt>(std::move(list));
}

ptr_Stmt Parser::parseIf() {
  require(tok::TokenType::If);
  ++lexer;
  auto cond = parseExpression();
  require(tok::TokenType::Then);
  ++lexer;
  auto then = parseStatement();
  if (lexer.get()->getTokenType() == tok::TokenType::Else) {
    ++lexer;
    return std::make_unique<IfStmt>(std::move(cond), std::move(then), parseStatement());
  }
  return std::make_unique<IfStmt>(std::move(cond), std::move(then));
}

ptr_Stmt Parser::parseWhile() {
  require(tok::TokenType::While);
  ++lexer;
  auto cond = parseExpression();
  require(tok::TokenType::Do);
  ++lexer;
  return std::make_unique<WhileStmt>(std::move(cond), parseStatementForLoop());
}

ptr_Stmt Parser::parseFor() {
  require(tok::TokenType::For);
  ++lexer;
  require(tok::TokenType::Id);
  auto var = std::make_unique<Variable>(lexer.next());
  require(tok::TokenType::Assignment);
  ++lexer;
  auto low = parseExpression();
  require({tok::TokenType::To, tok::TokenType::Downto}, "\"to\" or \"downto\"");
  bool dir = match({tok::TokenType::To});
  ++lexer;
  auto high = parseExpression();
  require(tok::TokenType::Do);
  ++lexer;
  return std::make_unique<ForStmt>(std::move(var), std::move(low),
                                   std::move(high), dir, parseStatementForLoop());
}

ptr_Stmt Parser::parseStatementForLoop() {
  if (match({tok::TokenType::Id})) {
    auto idName = lexer.get()->getValueString();

    if (idName == "break") {
      ++lexer;
      return std::make_unique<BreakStmt>();
    } else if (idName == "continue") {
      ++lexer;
      return std::make_unique<ContinueStmt>();
    } else {
      return parseStatement();
    }

  } else {
    return parseStatement();
  }
}

ptr_Stmt Parser::parseMainBlock() {
  auto main = parseCompound();
  require(tok::TokenType::Dot);
  ++lexer;
  return main;
}

bool Parser::match(const std::list<tok::TokenType>& listType) {
  auto& get = lexer.get();
  if (get == nullptr) { return false; }
  for (auto& exceptType: listType) {
    if (get->getTokenType() == exceptType) { return true; }
  }
  return false;
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