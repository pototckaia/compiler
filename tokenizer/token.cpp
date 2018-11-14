#include "token.h"

#include <string>

using namespace tok;

TokenBase::TokenBase(int line, int column, tok::TokenType tokenType, const std::string& strValue)
  : line(line), column(column),
    tokenType(tokenType),
    strValue(strValue) {}

std::string TokenBase::toString() {
  return std::to_string(line) + "\t" + std::to_string(column) + "\t" +
         tok::toString(tokenType) + "\t\"" + strValue + "\"\t";
}

template<typename T>
Token<T>::Token(int line, int column, tok::TokenType tokenType,
                T value, const std::string& strValue)
  : TokenBase(line, column, tokenType, strValue), value(value) {}

template<typename T>
std::string Token<T>::toString() {
  return TokenBase::toString() + "\t" + std::to_string(value);
}

Token<std::string>::Token(int line, int column, tok::TokenType tokenType,
                          const std::string& value, const std::string& str_value)
  : TokenBase(line, column, tokenType, str_value), value(value) {}

std::string Token<std::string>::toString() {
  return TokenBase::toString() + "\t\"" + value + "\"";
}


Token<tok::KeywordType>::Token(int line, int column, tok::TokenType tokenType,
                               tok::KeywordType value, const std::string& str_value)
  : TokenBase(line, column, tokenType, str_value), value(value) {}

std::string Token<tok::KeywordType>::toString() {
  return TokenBase::toString() + "\t" + tok::toString(value);
}

template
class Token<long long>;

template
class Token<long double>;
