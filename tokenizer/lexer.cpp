#include <algorithm>
#include <iostream>
#include "lexer.h"

#include "token_type.h"
#include "../exception.h"
#include "token_type.h"

using namespace lx;

Lexer::Lexer(const std::string& filename)
  : line(1), column(1),
    readFile(filename, std::ifstream::in) {}

    // TODO rename error message
void Lexer::errorHandler(int state) {
  std::string c(1, curSymbol);
  switch (state) {
    case (-77): {
      throw LexerException(line, beginToken, "Error: Illegal character \"" + c + "\"");
    }
    case (-71): {
      throw LexerException(line, beginToken, "Error: String on new line");
    }
    case (-80): {
      throw LexerException(line, beginToken + 1, "Error: Illegal start integer constant \"" + c + "\"");
    }
    case (-81): {
      throw LexerException(line, numSymbol - 1, "Error: Illegal start char constant \"" + c + "\"");
    }
    case (-82): {
      throw LexerException(line, beginToken, "Error: Illegal double \"" + strToken + "\"");
    }
    case (-83): {
      throw LexerException("Error: Unexpected end of file");
    }
  }
}

// TODO move constant to state_table
void Lexer::checkLenId(int prevState, int newState) {
  bool isIdContinue = (prevState == 2 && newState == 2) || (prevState == 14 && newState == 14);
  if (isIdContinue && valToken.size() > maxLenId) {
    throw LexerException(line, beginToken, "Error: Identifier exceed maximum length");
  }
}

bool Lexer::isEndComment(int prevState, int newState) {
  return newState == 0 && (prevState == 28 || prevState == 36 || prevState == 35);
}

bool Lexer::isPreview(int state) {
  return state < 0 && withoutPreview.count(state) == 0;
}

bool Lexer::isWhitespace(int prevState, int newState) {
  return prevState == 0 && newState == 0;
}

Token Lexer::next() {
  if (readFile.bad()) {
    return Token(line, beginToken, TokenType::EndOfFile);
  }

  strToken = "";
  valToken = "";
  std::string charConstant;

  int prevState = startState;
  int newState = startState;
  int baseIntConvert = 10;

  numSymbol = beginToken = column;

  for (; newState >= 0; prevState = newState) {
    curSymbol = readFile.get();
    if (curSymbol < 0) {
      curSymbol = 4;
    } // hack to end file

    newState = stateTable[prevState][curSymbol];
    std::pair<int, int> pairState(prevState, newState);

    // change base if we can
    if (changeBaseInt.count(pairState) > 0) {
      baseIntConvert = changeBaseInt.at(pairState);
    }

    if (charConstantEnd.count(pairState) > 0) {
      valToken += std::stoi(charConstant, nullptr, baseIntConvert);
      charConstant = "";
    } else if (charConstantAdd.count(pairState) > 0) {
      charConstant += curSymbol;
    } else if (skipSymbol.count(pairState) == 0 && !isPreview(newState)) {
      valToken += curSymbol;
    }

    if (!isWhitespace(prevState, newState) && !isPreview(newState)) {
      strToken += curSymbol;
    }

    if (curSymbol == '\n' && newState >= 0) {
      ++line;
      numSymbol = beginToken = 1;
    } else if (curSymbol != '\n') {
      ++numSymbol;
      if (newState == startState) {
        ++beginToken;
      }
    }

    if (isEndComment(prevState, newState)) {
      beginToken = numSymbol;
      valToken = strToken = "";
    }

    checkLenId(prevState, newState);
  }

  errorHandler(newState);

  if (readFile.bad() || newState == eofState) {
    return Token(line, beginToken, TokenType::EndOfFile);
  }

  if (withoutPreview.count(newState) == 0) {
    readFile.putback(curSymbol);
    --numSymbol;
    if (newState == twicePutbackState) {
      readFile.putback(strToken.back());
      --numSymbol;
      strToken.pop_back();
    }
  }

  column = numSymbol;
  auto tokenType = toTokenType.at(newState);

  if (tokenType ==  TokenType::Id) {
    std::transform(valToken.begin(), valToken.end(), valToken.begin(), ::tolower);
  }

  if (newState == checkIdState && isKeyword(valToken)) {
    tokenType = getKeywordType(valToken);
    return Token(line, beginToken, tokenType, valToken, strToken);
  }

  switch (tokenType) {
    case TokenType::Int: {
      uint64_t value = std::stoll(valToken, nullptr, baseIntConvert);
      return Token(line, beginToken, value, strToken);
    }
    case TokenType::Double: {
      auto value = std::stold(valToken);
      return Token(line, beginToken, value, strToken);
    }
    case TokenType::String:
    case TokenType::Id: {
      return Token(line, beginToken, tokenType, valToken, strToken);
    }
    default: {
      return Token(line, beginToken, tokenType, strToken);
    }
  }

}


template<class T1, class T2>
std::size_t Lexer::pairHash::operator()(const std::pair<T1, T2>& p) const {
  auto h1 = std::hash<T1>{}(p.first);
  auto h2 = std::hash<T2>{}(p.second);
  return h1 ^ h2;
};