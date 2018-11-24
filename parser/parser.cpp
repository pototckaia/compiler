#include "parser.h"
#include "../exception.h"

using namespace pr;

Parser::Parser(const std::string& s) : lexer(s) {
  priority[0] = {tok::TokenType::Minus, tok::TokenType::Plus,
                 tok::TokenType::Or, tok::TokenType::Xor};
  priority[1] = {tok::TokenType::Asterisk, tok::TokenType::Slash,
                 tok::TokenType::And,
                 tok::TokenType::Shr, tok::TokenType::ShiftRight,
                 tok::TokenType::Shl, tok::TokenType::ShiftLeft};
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
  notRequire(tok::TokenType::EndOfFile);
  auto token = lexer.next();

  switch (token->getTokenType()) {
    case tok::TokenType::Not:
    case tok::TokenType::Minus:
    case tok::TokenType::Plus:
    case tok::TokenType::At: {
      return std::make_unique<UnaryOperation>(std::move(token), parseFactor());
    }
    case tok::TokenType::Int:
    case tok::TokenType::Double: {
      return std::make_unique<Literal>(std::move(token));
    }
    case tok::TokenType::Id: {
      return std::make_unique<Variable>(std::move(token));
    }
    case tok::TokenType::OpenParenthesis: {
      auto expr = parseTerm();

      require(tok::TokenType::CloseParenthesis);
      lexer.next();

      return expr;
    }
    default:
      throw ParserException(token->getLine(), token->getColumn());
  }
}

std::unique_ptr<ASTNode> Parser::parseTerm(int p) {
  auto left = getTermOrFactor(p + 1);

  while (true) {
    if (!require(priority[p])) {
      break;
    }
    auto op = lexer.next();
    auto right = getTermOrFactor(p + 1);

    left = std::make_unique<BinaryOperation>(std::move(op), std::move(left), std::move(right));
  }

  return left;
}

std::unique_ptr<pr::ASTNode> Parser::getTermOrFactor(int p) {
  if (p == priority.size()) {
    return parseFactor();
  } else {
    return parseTerm(p);
  }
}

std::unique_ptr<pr::ASTNode> Parser::parse() {
  return getTermOrFactor();
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
    throw ParserException(get->getLine(), get->getColumn());
  }
}