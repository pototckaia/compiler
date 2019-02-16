#include "lexerBuffer.h"

using namespace lx;

LexerBuffer::LexerBuffer(const std::string& fileName)
  : lexer(fileName), buffer() {}


std::unique_ptr<Token> LexerBuffer::next() {
  if (!buffer.empty()) {
    auto tok(std::move(*buffer.begin()));
    buffer.pop_front();
    return tok;
  }
  return lexer.next();
}

const std::unique_ptr<Token>& LexerBuffer::get() {
  std::list<std::unique_ptr<Token>>::iterator it;

  if (!buffer.empty()) {
    it = buffer.begin();
  } else {
    buffer.emplace_back(std::move(lexer.next()));
    it = --buffer.end();
  }

  return *it;
}

void LexerBuffer::push_back(std::unique_ptr<Token> t) {
  buffer.emplace_back(std::move(t));
}
LexerBuffer& LexerBuffer::operator++() {
  next();
  return *this;
}