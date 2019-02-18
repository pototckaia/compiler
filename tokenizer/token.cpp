#include "token.h"

#include <string>

std::string getPoint(int line, int column, std::string sep) {
	return std::to_string(line) + std::move(sep) +
				 std::to_string(column) + "\t";
}

std::string getPoint(const Token& t, std::string sep) {
	return getPoint(t.getLine(), t.getColumn(), std::move(sep));
}

Token::Token(int line, int column,
					   TokenType tokenType, const std::string& strValue)
    : line(line), column(column),
      tokenType(tokenType),
      strValue(strValue),
      value(strValue) {}

Token::Token(int line, int column, TokenType tokenType)
    : Token(line, column, tokenType, toString(tokenType)) {}

Token::Token(int line, int column,
						 uint64_t value, const std::string& strValue)
		: line(line), column(column),
			tokenType(TokenType::Int),
			strValue(strValue),
			value(value) {}

Token::Token(int line, int column,
						 long double value, const std::string& strValue)
		: line(line), column(column),
			tokenType(TokenType::Double),
			strValue(strValue),
			value(value) {}

Token::Token(int line, int column,
						 TokenType tokenType,
						 const std::string& value,
						 const std::string& strValue)
		: line(line), column(column),
			tokenType(tokenType),
			strValue(strValue),
			value(value) {}


uint64_t Token::getInt() const {
	return std::get<uint64_t>(value);
}

long double Token::getDouble() const {
	return std::get<long double>(value);
}

// helper type for the visitor
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::string Token::getString() const {
	return std::visit(
			overloaded{
					[](uint64_t arg) { return std::to_string(arg); },
					[](long double arg) { return std::to_string(arg); },
					[](std::string arg) { return arg; },},
			value);
}

std::string Token::getTestLine() const {
	return getPoint(*this, "\t") +
				 getGroup(tokenType) + "\t" +
				 "\"" + strValue + "\"\t\t" +
				 "\"" + getString() + "\"";
}