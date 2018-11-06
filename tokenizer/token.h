#pragma once

#include <string>

#include "token_type.h"


class TokenBase {
 public:
  TokenBase() = delete;
  TokenBase(int line, int column, tok::TokenType token_type, const std::string& strValue);
  virtual ~TokenBase() = default;

  virtual std::string toString();

 private:
  int line, column;
  tok::TokenType tokenType;
  std::string strValue;
};

template <typename T>
class Token : public TokenBase {
 public:
  Token() = delete;
  Token(int line, int column, tok::TokenType token_type, T value, const std::string& str_value);
  ~Token() override = default;

  std::string toString() override;

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

 private:
  tok::KeywordType value;
};