#pragma once

#include <string>
#include <stdexcept>
#include <list>
#include <variant>

#include "token_type.h"

class Token {
 public:
	Token() = delete;
	Token(int line, int column, TokenType);
	Token(int line, int column, TokenType, const std::string& strValue);
	Token(int line, int column, uint64_t, const std::string& strValue);
	Token(int line, int column, long double, const std::string& strValue);
	Token(int line, int column, TokenType, const std::string& value,
				const std::string& strValue);

  std::string getTestLine() const;

  int getLine() const { return line; }
  int getColumn() const { return column; }
  std::string getStrValue() const { return strValue; }
  TokenType getTokenType() const { return tokenType; }
	bool is(TokenType t) const { return tokenType == t; }

  uint64_t getInt() const;
  long double getDouble() const;
  std::string getString() const;

 private:
  int line, column;
  TokenType tokenType;
  std::string strValue;
	std::variant<long double, uint64_t, std::string> value;
};

using ListToken = std::list<Token>;

std::string getPoint(int line, int column, std::string sep = ",");
std::string getPoint(const Token&, std::string sep = ",");
