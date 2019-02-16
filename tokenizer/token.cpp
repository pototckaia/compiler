#include "token.h"

#include <string>

Token::Token(int line, int column, TokenType tokenType, const std::string& strValue)
  : line(line), column(column),
    tokenType(tokenType),
    strValue(strValue) {}

std::string Token::toString() const {
  return beginOfString() + "\t\"" + strValue + "\"";
}

std::string Token::beginOfString() const {
  return std::to_string(line) + "\t" + std::to_string(column) + "\t" +
			getGroup(tokenType) + "\t\"" + strValue + "\"\t";
}


StringConstant::StringConstant(int line, int column, TokenType token_type,
                               const std::string& value, const std::string& strValue)
  : Token(line, column, token_type, strValue), value(value) {}

std::string StringConstant::toString() const {
  return Token::beginOfString() + "\t\"" + value + "\"";
}


template<typename T>
NumberConstant<T>::NumberConstant(int line, int column, TokenType tokenType,
                                  T& value, const std::string& strValue)
  : Token(line, column, tokenType, strValue), value(value) {}

template<typename T>
std::string NumberConstant<T>::toString() const {
  return Token::beginOfString() + "\t" + std::to_string(value);
}

std::string getPoint(int line, int column) {
  return std::to_string(line) + "," + std::to_string(column) + "\t";
}

std::string getPoint(const ptr_Token& t) {
  return getPoint(t->getLine(), t->getColumn());
}

template
class NumberConstant<uint64_t>;

template
class NumberConstant<long double>;
