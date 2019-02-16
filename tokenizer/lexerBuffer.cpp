#include "lexerBuffer.h"

using namespace lx;

LexerBuffer::LexerBuffer(const std::string& fileName)
  : lexer(fileName), buffer() {}


Token LexerBuffer::next() {
  if (!buffer.empty()) {
    auto tok(std::move(*buffer.begin()));
    buffer.pop_front();
    return tok;
  }
  return lexer.next();
}

const Token& LexerBuffer::get() {
  ListToken::iterator it;

  if (!buffer.empty()) {
    it = buffer.begin();
  } else {
    buffer.emplace_back(lexer.next());
    it = --buffer.end();
  }

  return *it;
}

void LexerBuffer::push_back(Token t) {
  buffer.emplace_back(std::move(t));
}

LexerBuffer& LexerBuffer::operator++() {
  next();
  return *this;
}