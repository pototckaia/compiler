#include "token.h"

#include <string>

using namespace tok;

TokenBase::TokenBase(int line, int column, tok::TokenType tokenType, const std::string& strValue)
  : line(line), column(column),
    tokenType(tokenType),
    strValue(strValue) {}

std::string TokenBase::toString() const {
  return beginOfString() + "\t\"" + strValue + "\"";
}

std::string TokenBase::beginOfString() const {
  return std::to_string(line) + "\t" + std::to_string(column) + "\t" +
         tok::toStringGroup(tokenType) + "\t\"" + strValue + "\"\t";
}


StringConstant::StringConstant(int line, int column, tok::TokenType token_type,
                               const std::string& value, const std::string& strValue)
  : TokenBase(line, column, token_type, strValue), value(value) {}

std::string StringConstant::toString() const {
  return TokenBase::beginOfString() + "\t\"" + value + "\"";
}


template<typename T>
NumberConstant<T>::NumberConstant(int line, int column, tok::TokenType tokenType,
                                  T& value, const std::string& strValue)
  : TokenBase(line, column, tokenType, strValue), value(value) {}

template<typename T>
std::string NumberConstant<T>::toString() const {
  return TokenBase::beginOfString() + "\t" + std::to_string(value);
}

std::string tok::getPoint(int line, int column) {
  return std::to_string(line) + "," + std::to_string(column) + "\t";
}

std::string tok::getPoint(const tok::ptr_Token& t) {
  return tok::getPoint(t->getLine(), t->getColumn());
}

template
class tok::NumberConstant<long long int>;

template
class tok::NumberConstant<long double>;
