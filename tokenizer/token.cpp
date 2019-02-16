#include "token.h"

std::string getPoint(int line, int column, std::string sep) {
  return std::to_string(line) + std::move(sep) +
         std::to_string(column) + "\t";
}

std::string getPoint(const Token& t, std::string sep) {
  return getPoint(t.getLine(), t.getColumn(), std::move(sep));
}

Token::Token(int line, int column,
					   TokenType tokenType, std::string strValue)
  : line(line), column(column),
    tokenType(tokenType),
    strValue(std::move(strValue)),
    value(std::move(strValue)) {}

Token::Token(int line, int column, TokenType t)
	: Token(line, column, t, toString(t)) {}

Token::Token(int line, int column,
					   uint64_t value, std::string strValue)
  : line(line), column(column),
    tokenType(TokenType::Int),
    strValue(std::move(strValue)),
    value(value) {}

Token::Token(int line, int column,
             long double value, std::string strValue)
    : line(line), column(column),
      tokenType(TokenType::Double),
      strValue(std::move(strValue)),
      value(value) {}

Token::Token(int line, int column,
             TokenType tokenType,
             std::string value, std::string strValue)
    : line(line), column(column),
      tokenType(tokenType),
      strValue(std::move(strValue)),
      value(std::move(value)) {}

int Token::getLine() const { return line; }
int Token::getColumn() const { return column; }
TokenType Token::getTokenType() const { return tokenType; }
bool Token::is(TokenType t) const { return tokenType == t; }
std::string Token::getStrValue() const { return strValue; }

uint64_t Token::getInt() const { return std::get<uint64_t>(value); }
long double Token::getDouble() const { return std::get<long double>(value); }


// helper type for the visitor
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::string Token::getString() const {
	return std::visit(
			overloaded {
					[](uint64_t arg) { return std::to_string(arg); },
					[](long double arg) { return std::to_string(arg); },
					[](std::string arg) { return arg; },},
			value);
}

std::string Token::getTestLine() const {
	return getPoint(*this, "\t") +
				 getGroup(tokenType) + "\t" +
				 "\"" + strValue + "\"\t" +
				 getString();
}