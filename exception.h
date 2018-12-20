#pragma once

#include <string>

#include "token.h"


class LexerException : public std::exception {
 public:
  explicit LexerException(const std::string& ss) : s(ss) {}
  explicit LexerException(int l, int c, const std::string& ss) : s(std::to_string(l) + "\t" +
                                                                   std::to_string(c) + "\t" + ss) {}
  ~LexerException() override = default;

  const char* what() const noexcept { return s.c_str(); }

 private:
  std::string s;
};

class ParserException : public std::exception {
 public:
  explicit ParserException(int line, int column, std::string s)
    : s(tok::getPoint(line, column) + " Illegal expression: " + s) {}

  explicit ParserException(int line, int column, tok::TokenType t)
    : ParserException(line, column, tok::toString(t)) {}

  explicit ParserException(int line, int column, std::string except, std::string get)
    : s(tok::getPoint(line, column) + "Except: \""  + except +
        "\" but find \"" + get + "\"") {}

  explicit ParserException(int line, int column, tok::TokenType exceptType, tok::TokenType getType)
    : ParserException(line, column, tok::toString(exceptType), tok::toString(getType)) {}

  ~ParserException() override = default;

  const char* what() const noexcept { return s.c_str(); }

 private:
  std::string s;
};

class AlreadyDefinedException : public std::exception {
 public:
  explicit AlreadyDefinedException(std::string n) : s(a + "\"" + n + "\"") {};
  explicit AlreadyDefinedException(int line, int column, std::string n)
    : s(tok::getPoint(line, column) + a + "\"" + n + "\"") {}
  explicit AlreadyDefinedException(const tok::ptr_Token& t)
    : s(getPoint(t) + a + "\"" + t->getValueString() + "\"") {}

  ~AlreadyDefinedException() override = default;
  const char* what() const noexcept { return s.c_str(); }

 private:
  const std::string a = "Already defined ";
  std::string s;
};

class NotDefinedException: public std::exception {
 public:
  explicit NotDefinedException(const std::string& s)
   : s(a + "\"" + s + "\"") {};

  explicit NotDefinedException(const tok::ptr_Token& t)
   : s(getPoint(t) + a + "\"" + t->getValueString() + "\"") {};

  ~NotDefinedException() override = default;
  const char* what() const noexcept { return s.c_str(); }

 private:
  const std::string a = "Not defined ";
  std::string s;
};

class SemanticException : public std::exception {
 public:
  explicit SemanticException(int line, int column, std::string n)
    : s(tok::getPoint(line, column) + " " + n) {}

  const char* what() const noexcept { return s.c_str(); }
 private:
  std::string s;
};