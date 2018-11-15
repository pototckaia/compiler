#include "parser.h"
#include "../exception.h"

using namespace pr;

Parser::Parser(const std::string& s) : lexer(s) {}

std::unique_ptr<ASTNode> Parser::parseFactor() {
  auto token = lexer.next();

  if (token == nullptr) {
    throw ParserException(token->getLine(), token->getColumn(), "Syntax error");
  }

  switch (token->getTokenType()) {
    case tok::TokenType::Id: {
      return std::make_unique<Variable>(std::move(token));
    }
    case tok::TokenType::Int: case tok::TokenType::Double: {
      return std::make_unique<Literal>(std::move(token));
    }
    case tok::TokenType::OpenParenthesis: {
      auto expr = parseExpr();
      auto t = lexer.next();
      if (t->getTokenType() != tok::TokenType::CloseParenthesis) {
        throw ParserException(t->getLine(), t->getColumn(), "Syntax error: except \")\" but find \"" + t->getValueString() + "\"");
      };
      return expr;
    }
    default:
      throw ParserException(token->getLine(), token->getColumn(), "Syntax error");
  }
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
  auto left = parseFactor();

  while (true) {
    const auto& token = lexer.get();
    if (token == nullptr || (token->getTokenType() != tok::TokenType::Asterisk &&
        token->getTokenType() != tok::TokenType::Slash)) {
      break;
    }
    auto op = lexer.next();
    auto right = parseFactor();

    left = std::make_unique<BinaryOperation>(std::move(op), std::move(left), std::move(right));
  }

  return left;
}

std::unique_ptr<pr::ASTNode> Parser::parseExpr() {
  auto left = parseTerm();

  while (true) {
    const auto& token = lexer.get();
    if (token == nullptr || (token->getTokenType() != tok::TokenType::Minus &&
        token->getTokenType() != tok::TokenType::Plus)) {
      break;
    }
    auto op = lexer.next();
    auto right = parseTerm();

    left = std::make_unique<BinaryOperation>(std::move(op), std::move(left), std::move(right));
  }

  return left;
}

std::unique_ptr<pr::ASTNode> Parser::parse() {
  return parseExpr();
}