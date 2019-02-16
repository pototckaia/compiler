#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <list>

#include "token_type.h"

class TokenBase;

using ptr_Token = std::unique_ptr<TokenBase>;
using ListToken = std::list<ptr_Token>;

std::string getPoint(int line, int column);

std::string getPoint(const ptr_Token& t);

class TokenBase {
 public:
  TokenBase() = delete;
  TokenBase(int line, int column, TokenType token_type, const std::string& strValue);
  virtual ~TokenBase() = default;

  virtual std::string toString() const;

  int getLine() const { return line; }
  int getColumn() const { return column; }
  TokenType getTokenType() const { return tokenType; }

  virtual uint64_t getInt() const { throw std::logic_error("Token type not Int"); }
  virtual long double getDouble() const { throw std::logic_error("Token type not Double"); }
  virtual std::string getValueString() const { return strValue; }
  bool is(TokenType t) { return tokenType == t; }

 protected:
  std::string beginOfString() const;

 private:
  int line, column;
  TokenType tokenType;
  std::string strValue;
};

template<typename T>
class NumberConstant : public TokenBase {
 public:
  NumberConstant() = delete;
  NumberConstant(int line, int column, TokenType token_type,
                 T& value, const std::string& strValue);
  ~NumberConstant() = default;

  std::string toString() const override;

  std::string getValueString() const override { return std::to_string(value); }

  uint64_t getInt() const override {
    if (getTokenType() == TokenType::Int) { return value; }
    return TokenBase::getInt();
  }

  long double getDouble() const override {
    if (getTokenType() == TokenType::Double) { return value; }
    return TokenBase::getDouble();
  }

 private:
  T value;
};

class StringConstant : public TokenBase {
 public:
  StringConstant() = delete;

  StringConstant(int line, int column, TokenType token_type,
                 const std::string& value, const std::string& strValue);
  ~StringConstant() = default;

  std::string toString() const override;

  std::string getValueString() const override { return value; }

 private:
  std::string value;
};