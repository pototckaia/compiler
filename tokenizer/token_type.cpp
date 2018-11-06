#include "token_type.h"

namespace tok {

#define MAKE_CASE_TOKEN(E, S) case TokenType::E: { return S; }
#define MAKE_CASE_WITH_N(E, S, ...) case E: { return S; }

std::string toString(TokenType t) {
  switch (t) {
    TOKEN_TYPE(MAKE_CASE_TOKEN, MAKE_CASE_WITH_N)
  }
}
#undef MAKE_CASE_WITH_N
#undef MAKE_CASE_TOKEN


#define MAKE_LIST(E, S) {S, KeywordType::E},
static std::map<std::string, KeywordType> mapKeyword({KEYWORD_TYPE(MAKE_LIST)});
#undef MAKE_LIST


#define MAKE_CASE_KEYWORD(E, S) case KeywordType::E: { return S; }
std::string toString(KeywordType t) {
  switch (t) {
    KEYWORD_TYPE(MAKE_CASE_KEYWORD)
  }
}
#undef MAKE_CASE_KEYWORD


KeywordType getKeywordType(const std::string &s) {
  return mapKeyword.at(s);
}

bool isKeyword(const std::string &s) {
  return mapKeyword.count(s) > 0;
}

} // namespace tok

