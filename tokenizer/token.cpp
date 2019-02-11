#include "token.h"


TokenBase::TokenBase(int line, int column, 
					 tok::TokenType tokenType, const std::string& strValue)
  : line(line), column(column),
    tokenType(tokenType),
    value(toString(tokenType)),
    strValue(strValue) {}

TokenBase::TokenBase(int line, int column, 
					 tok::TokenType tokenType, const std::string& strValue)
  : line(line), column(column),
    tokenType(tokenType),
    value(toString(tokenType)),
    strValue(strValue) {}

std::string TokenBase::toString() const {
  return beginOfString() + "\t\"" + strValue + "\"";
}

std::string TokenBase::beginOfString() const {
  return std::to_string(line) + "\t" + std::to_string(column) + "\t" +
         tok::toStringGroup(tokenType) + "\t\"" + strValue + "\"\t";
}


std::string tok::getPoint(int line, int column) {
  return std::to_string(line) + "," + std::to_string(column) + "\t";
}

std::string tok::getPoint(const tok::ptr_Token& t) {
  return tok::getPoint(t->getLine(), t->getColumn());
}


  int getLine() const { return line; }
  int getColumn() const { return column; }
  TokenType getTokenType() const { return tokenType; }
  bool is(TokenType t) { return tokenType == t; }

  uint64_t getInt() const { return std::get<uint64_t>(value); }
  long double getDouble() const { return std::get<double>(value); }
  std::string getString() const { return std::get<std::string>(value); }
