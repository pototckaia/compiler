#pragma once

#include <string>

#include "token.h"

class CompilerException : public std::exception {
 public:
  explicit CompilerException(std::string ss) : s(std::move(ss)) {}
  explicit CompilerException(int line, int column, std::string ss)
    : s(getPoint(line, column) + std::move(ss)) {}
  explicit CompilerException(const Token& t, std::string ss)
    : CompilerException(t.getLine(), t.getColumn(), std::move(ss)) {}

  const char* what() const noexcept override { return s.c_str(); }
  ~CompilerException() override = default;

 private:
  std::string s;
};

class LexerException : public CompilerException {
 public:
  using CompilerException::CompilerException;
  explicit LexerException(int line, int column, std::string ss) :
    CompilerException(std::to_string(line) + "\t" + std::to_string(column) + "\t" + std::move(ss)) {}
};

class SemanticException : public CompilerException {
 public:
  using CompilerException::CompilerException;
};

class ParserException : public CompilerException {
 public:
  explicit ParserException(int line, int column, std::string s)
    : CompilerException(line, column, "Illegal expression: " + std::move(s)) {}
  explicit ParserException(int line, int column, TokenType t)
    : ParserException(line, column, toString(t)) {}
  explicit ParserException(int line, int column, std::string except, std::string get)
    : CompilerException(line, column,
      "Expect: \""  + std::move(except) + "\" but find \"" + std::move(get) + "\"") {}
  explicit ParserException(int line, int column, TokenType exceptType, TokenType getType)
    : ParserException(line, column, toString(exceptType), toString(getType)) {}
};

class AlreadyDefinedException : public CompilerException {
 public:
  explicit AlreadyDefinedException(int line, int column, std::string n)
    : CompilerException(line, column, "Already defined \"" + std::move(n) + "\"") {}
  explicit AlreadyDefinedException(const Token& t)
    : CompilerException(getPoint(t) + "Already defined \"" + t.getString() + "\"") {}
};

class NotDefinedException: public CompilerException {
 public:
  explicit NotDefinedException(std::string ss)
   : CompilerException("Not defined \"" + std::move(ss) + "\"") {};
  explicit NotDefinedException(const Token& t)
   : CompilerException(getPoint(t) + "Not defined \"" + t.getString() + "\"") {};
};