#include "token_type.h"

namespace tok {

#define MAKE_CASE_TOKEN(E, S) case TokenType::E: { return S; }
#define MAKE_CASE_WITH_N(E, S, ...) case E: { return S; }

std::string to_string(TokenType t) {
  switch (t) {
    TOKEN_TYPE(MAKE_CASE_TOKEN, MAKE_CASE_WITH_N)
    default: {
      return "Unknown";
    }
  }
}
#undef MAKE_CASE_WITH_N
#undef MAKE_CASE_TOKEN

#define MAKE_CASE_KEYWORD(E, S) case KeywordType::E: { return S; }
std::string KeywordHelper::to_string(KeywordType t) {
  switch (t) {
    KEYWORD_TYPE(MAKE_CASE_KEYWORD)
    default:
      return "Unknown";
  }
}
#undef MAKE_CASE_KEYWORD


#define MAKE_LIST(E, S) {S, E},
KeywordHelper::KeywordHelper() : map_({ KEYWORD_TYPE(MAKE_LIST) }){}
#undef MAKE_LIST

KeywordType KeywordHelper::get_keyword(const std::string& s) const {
  return map_.at(s);
}

bool KeywordHelper::is_keyword(const std::string & s) const {
  return map_.count(s) > 0;
}

} // namespace tok

