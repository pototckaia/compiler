#pragma once

#include <string>

#include "token_type.h"


class TokenBase {
 public:
  TokenBase() = delete;
  TokenBase(int line, int column, tok::TokenType token_type, const std::string& str_value);
  virtual ~TokenBase() = default;

  virtual std::string to_string();

 public:
  int line_, column_;
  tok::TokenType token_type_;
  std::string str_value_;
};


class OperatorOrId : public TokenBase {
 public:
  OperatorOrId() = delete;
  OperatorOrId(int line, int column, tok::TokenType token_type, const std::string& value, const std::string& str_value);

  std::string to_string() override;

 public:
  std::string value_;
};


class Integer : public TokenBase {
 public:
  Integer() = delete;
  Integer(int line, int column, tok::TokenType token_type, const std::string& str_value);

  std::string to_string() override;

 public:
  long long int value_;
};

class Double : public TokenBase {
 public:
  Double() = delete;
  Double(int line, int column, tok::TokenType, const std::string& str_value);

  std::string to_string() override;
 private:
  long double value_;
};

class Keyword : public TokenBase {
 public:
  Keyword() = delete;
  Keyword(int line, int column, tok::TokenType token_type, tok::KeywordType keyword_type, const std::string& str_value);

  std::string to_string() override;

 private:
  tok::KeywordType value_;
};

class ConstString : public TokenBase {
 public:
  ConstString() = delete;
  ConstString(int line, int column, tok::TokenType token_type, const std::string& str_value);

  std::string to_string() override;

 public:
  std::string value_;
};
