#include "parser.h"
#include "../exception.h"

using namespace pr;

Parser::Parser(const std::string& s)
  : lexer(s) {
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


ListExpr Parser::parseListExpression() {
  ListExpr list;
  list.push_back(parseBinaryOperator());
  while(lexer.get()->getTokenType() == tok::TokenType::Comma) {
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

  while (require(priority[p])) {
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
  if (!require(priority[p])) {
      return getExprByPriority(p + 1);
  }
  auto op = lexer.next();
  return std::make_unique<UnaryOperation>(std::move(op), parseUnaryOperator(p));
}

ptr_Expr Parser::parseBinaryOperator(int p) {
  auto left = getExprByPriority(p + 1);

  while (require(priority[p])) {
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

std::unique_ptr<pr::ASTNode> Parser::parse() {
  return parseBinaryOperator();
}

bool Parser::require(std::list<tok::TokenType>& listType) {
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

void Parser::notRequire(tok::TokenType type) {
  auto& get = lexer.get();
  if (get->getTokenType() == type) {
    throw ParserException(get->getLine(), get->getColumn(), type);
  }
}