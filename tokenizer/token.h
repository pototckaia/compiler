#pragma once

#include <string>

#include "token_type.h"

namespace tok {

class TokenBase {
 public:
  TokenBase() = delete;
  TokenBase(int line, int column, tok::TokenType token_type, const std::string& strValue);
  virtual ~TokenBase() = default;

  virtual std::string toString() const;

  int getLine() const { return line; }
  int getColumn() const { return column; }
  tok::TokenType getTokenType() const { return tokenType; }

  virtual std::string getValueString() const { return strValue; }

 protected:
  std::string beginOfString() const;

 private:
  int line, column;
  tok::TokenType tokenType;
  std::string strValue;
};

template<typename T>
class NumberConstant : public TokenBase {
 public:
  NumberConstant() = delete;
  NumberConstant(int line, int column, tok::TokenType token_type,
                 T& value, const std::string& strValue);
  ~NumberConstant() = default;

  std::string toString() const override;

  std::string getValueString() const override { return std::to_string(value); }

 private:
  T value;
};

class StringConstant : public TokenBase {
 public:
  StringConstant() = delete;

  StringConstant(int line, int column, tok::TokenType token_type,
                 const std::string& value, const std::string& strValue);
  ~StringConstant() = default;

  std::string toString() const override;

  std::string getValueString() const override { return value; }

 private:
  std::string value;
};

} // namespace tok