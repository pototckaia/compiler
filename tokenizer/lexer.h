#pragma once

#include <memory>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "token.h"
#include "state_table.h"

namespace lx {

class Lexer {
 public:
  explicit Lexer(const std::string&);

  ~Lexer() = default;

  std::unique_ptr<TokenBase> next();

 private:
  inline void errorHandler(int state);

  inline void checkLenId(int prevState, int newState);

  inline bool isEndComment(int prevState, int newState);

  inline bool isWhitespace(int prevState, int newState);

  inline bool isPreview(int);

  struct pairHash {
    template<class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const;
  };

  int line, column;
  int curSymbol;
  int beginToken, numSymbol;

  std::string strToken, valToken;

  std::ifstream readFile;

  const int startState = 0;
  const int eofState = EOF_STATE;
  const int checkIdState = CHECK_ID;
  const int twicePutbackState = TWICE_PUT_BACK;
  const int stateTable[38][128] = STATE_TABLE;

  const int maxLenId = 144;


  const std::unordered_set<int> withoutPreview = WITHOUT_PREVIEW;
  const std::unordered_set<std::pair<int, int>, pairHash> skipSymbol = SKIP_SYMBOL;
  const std::unordered_set<std::pair<int, int>, pairHash> charConstantAdd = CHAR_CONSTANT;
  const std::unordered_set<std::pair<int, int>, pairHash> charConstantEnd = CHAR_CONSTANT_END;
  const std::unordered_map<int, TokenType> toTokenType = FROM_FINAL_STATE_TO_TOKEN;
  const std::unordered_map<std::pair<int, int>, int, pairHash> changeBaseInt = CHANGE_BASE;
};

}; // namespace lx