#pragma once

#include <string>
namespace lxerr {

class LexerException : public std::exception {
 public:
  explicit LexerException(const std::string &ss) : s(ss) {}
  explicit LexerException(int l, int c, const std::string &ss) : s(std::to_string(l) + "\t" +
                                                                   std::to_string(c) + "\t" + ss) {}
  ~LexerException() override = default;
  const char *what() const noexcept { return s.c_str(); }

 private:
  std::string s;
};

} // namespace lxerr