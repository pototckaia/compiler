#include "token_type.h"

namespace tok {

#define MAKE_CASE_TOKEN(E, S) case TokenType::E: { return S; }
#define MAKE_CASE_KEYWORD(E, S) case TokenType::E: { return "Keyword"; }

std::string toStringGroup(TokenType t) {
  switch (t) {
    TOKEN_TYPE(MAKE_CASE_TOKEN)
    KEYWORD_TYPE(MAKE_CASE_KEYWORD)
  }
}

std::string toString(TokenType t) {
  switch (t) {
    TOKEN_TYPE(MAKE_CASE_TOKEN)
    KEYWORD_TYPE(MAKE_CASE_TOKEN)
  }
}

#undef MAKE_CASE_KEYWORD
#undef MAKE_CASE_TOKEN


#define MAKE_LIST(E, S) {S, TokenType::E},
static std::map<std::string, TokenType> mapKeyword({KEYWORD_TYPE(MAKE_LIST)});
#undef MAKE_LIST

TokenType getKeywordType(const std::string& s) {
  return mapKeyword.at(s);
}

bool isKeyword(const std::string& s) {
  return mapKeyword.count(s) > 0;
}

} // namespace tok

