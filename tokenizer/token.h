#pragma once

#include <string>

#include "token_type.h"

namespace tok {

class TokenBase {
 public:
  TokenBase() = delete;

  TokenBase(int line, int column, tok::TokenType token_type, const std::string& strValue);

  virtual ~TokenBase() = default;

  virtual std::string toString();

  tok::TokenType getTokenType() { return tokenType; }

  virtual std::string getValueString() = 0;

 private:
  int line, column;
  tok::TokenType tokenType;
  std::string strValue;
};

template<typename T>
class Token : public TokenBase {
 public:
  Token() = delete;

  Token(int line, int column, tok::TokenType token_type, T value, const std::string& str_value);

  ~Token() override = default;

  std::string toString() override;

  std::string getValueString() override { return std::to_string(value); }

 private:
  T value;
};

template<>
class Token<std::string> : public TokenBase {
 public:
  Token() = delete;

  Token(int line, int column, tok::TokenType tokenType,
        const std::string& value, const std::string& str_value);

  ~Token() override = default;

  std::string toString() override;

  std::string getValueString() override { return value; }

 private:
  std::string value;
};

template<>
class Token<tok::KeywordType> : public TokenBase {
 public:
  Token() = delete;

  Token(int line, int column, tok::TokenType tokenType,
        tok::KeywordType value, const std::string& str_value);

  ~Token() override = default;

  std::string toString() override;

  std::string getValueString() override { return tok::toString(value); };

 private:
  tok::KeywordType value;
};

} // namespace tok