#pragma once

#include <string>
#include <stdexcept>
#include <variant>

#include "token_type.h"

std::string getPoint(int line, int column);
std::string getPoint(const Token&);

class Token {
 public:
  Token() = delete;
  Token(int line, int column, TokenType, std::string strValue);
  Token(int line, int column, uint64_t, std::string strValue);
  Token(int line, int column, double, std::string strValue);
  Token(int line, int column, std::string, std::string strValue);

  std::string toString() const;
  int getLine() const;
  int getColumn() const;
  TokenType getTokenType() const;
  bool is(TokenType t) const;

  uint64_t getInt() const;
  long double getDouble() const;
  std::string getString() const;

 private:
  int line, column;
  TokenType tokenType;
  std::string strValue;
  std::variant<double, uint64_t, std::string> value;
};