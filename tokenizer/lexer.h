#pragma once

#include <memory>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "token.h"


class Lexer {
 public:
  explicit Lexer(const std::string&);
  ~Lexer() = default;

  std::unique_ptr<TokenBase> next();

 private:
  void errorHandler(int state);

  inline bool isEndComment(int prevState, int newState);
  inline bool isWhitespace(int prevState, int newState);
  inline bool isPreview(int);

  struct pairHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const;
  };

  int line, column;
  int beginToken, numSymbol;
  int curSymbol;
  const int startState, eofState, checkIdState, twicePutbackState;
  const int maxLenId = 144;
  int stateTable[38][128];

  std::string strToken;
  std::ifstream writeFile;

  std::unordered_set<int> withoutPreview;
  std::unordered_set<std::pair<int, int>, pairHash> skipSymbol;
  std::unordered_set<std::pair<int, int>, pairHash> charConstantAdd, charConstantEnd;
  std::unordered_map<int, tok::TokenType> toTokenType;
  std::unordered_map<std::pair<int, int>, int, pairHash> changeBaseInt;
};
