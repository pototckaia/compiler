#pragma once

#include <memory>
#include <list>
#include <string>

#include "token.h"
#include "lexer.h"

namespace lx {

class LexerBuffer {
 public:
  explicit LexerBuffer(const std::string& fileName);

  std::unique_ptr<tok::TokenBase> next();

  const std::unique_ptr<tok::TokenBase>& get();

  void push_back(std::unique_ptr<tok::TokenBase>&& token);

 private:
  Lexer lexer;
  std::list<std::unique_ptr<tok::TokenBase>> buffer;
};

} // namespace lexer