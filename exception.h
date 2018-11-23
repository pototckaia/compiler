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
  explicit ParserException(int line, int column)
    : s(std::to_string(line) + "," + std::to_string(column) + "\t Illegal expression") {}

  explicit ParserException(int line, int column, tok::TokenType exceptType, tok::TokenType getType)
    : s(std::to_string(line) + "," + std::to_string(column) + "\t" +
        "Except: \""  + tok::toString(exceptType) + "\" but find \"" +
        tok::toString(getType)) {}
  ~ParserException() override = default;

  const char* what() const noexcept { return s.c_str(); }

 private:
  std::string s;
};
