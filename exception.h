#pragma once

#include <string>

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
    : s(std::to_string(line) + "," + std::to_string(column) + "\t Illegal expression: " + s) {}

  explicit ParserException(int line, int column, tok::TokenType t)
    : ParserException(line, column, tok::toString(t)) {}

  explicit ParserException(int line, int column, std::string except, std::string get)
    : s(std::to_string(line) + "," + std::to_string(column) + "\t" +
        "Except: \""  + except + "\" but find \"" + get + "\"") {}

  explicit ParserException(int line, int column, tok::TokenType exceptType, tok::TokenType getType)
    : ParserException(line, column, tok::toString(exceptType), tok::toString(getType)) {}

  ~ParserException() override = default;

  const char* what() const noexcept { return s.c_str(); }

 private:
  std::string s;
};
