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
  explicit ParserException(const std::string& ss) : s(ss) {}
  explicit ParserException(int line, int column, const std::string& ss) : s(std::to_string(line) + ", " +
                                                                            std::to_string(column) + " " + ss) {}
  ~ParserException() override = default;

  const char* what() const noexcept { return s.c_str(); }

 private:
  std::string s;
};
