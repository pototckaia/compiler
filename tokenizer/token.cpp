#include "token.h"

#include <string>

TokenBase::TokenBase(int line, int column, tok::TokenType token_type, const std::string &str_value)
    : line_(line), column_(column), token_type_(token_type), str_value_(str_value) {}

std::string TokenBase::to_string() {
  return "(" + std::to_string(line_) + "," + std::to_string(column_) + ")" + "\t" +
         tok::to_string(token_type_) + "\t" +
         '"' + str_value_ + '"' + "\t";
}

OperatorOrId::OperatorOrId(int line, int column, tok::TokenType token_type, const std::string& value,
                           const std::string &str_value)
    : TokenBase(line, column, token_type, str_value), value_(str_value) {}

std::string OperatorOrId::to_string() {
      return TokenBase::to_string() + "\t" + value_;
}

Integer::Integer(int line, int column, tok::TokenType token_type, const std::string &str_value)
    : TokenBase(line, column, token_type, str_value),
      value_() {
  std::size_t pos = 0;
  std::string s = str_value;
  if (token_type != tok::IntBase10) {
    s.erase(s.begin());
  };
  int base = (int) token_type;
  long int v = std::stol(s, &pos, base);
  value_ = v;
}


std::string Integer::to_string() {
  return TokenBase::to_string() + "\t" + std::to_string(value_);
}

Double::Double(int line, int column, tok::TokenType token_type, const std::string &str_value)
    : TokenBase(line, column, token_type, str_value),
      value_(std::stold(str_value)) {}

std::string Double::to_string() {
  return TokenBase::to_string() + "\t" + std::to_string(value_);
}

Keyword::Keyword(int line, int column, tok::TokenType token_type, tok::KeywordType keyword_type,
                 const std::string &str_value)
    : TokenBase(line, column, token_type, str_value),
      value_(keyword_type) {}

std::string Keyword::to_string() {
  return TokenBase::to_string() + "\t" + tok::KeywordHelper::to_string(value_);
}

ConstString::ConstString(int line, int column, tok::TokenType token_type, const std::string &str_value)
    : TokenBase(line, column, token_type, str_value), value_() {
  int state = 0;

  std::string char_const = "";
  int base;

  for (char e : str_value_) {
    if (state == 0) {
      switch(e) {
        case '\'': { state = 1; break; }
        case '#': {state = 3; char_const = ""; break;}
      }
      continue;
    }
    if (state == 1) {
      switch(e) {
        case '\'' : { state = 2; break; }
        default: { value_ += e; break; }
      }
      continue;
    }
    if (state == 2) {
      switch (e) {
        case '\'': { value_ += e;  state = 1; break; }
        case '#': { state = 3; char_const = ""; break; }
      }
      continue;
    }
    if (state == 3) {
      switch (e) {
        case '\'': {
          state = 1;
          std::size_t pos = 0;
          char c = std::stoi(char_const, &pos, base);
          value_ += c;
          break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7' :
        case '8':
        case '9': {
          char_const += e;
          base = 10;
          break;
        }
        case '%': {
          base = 2;
          state = 4;
          char_const = "";
          break;
        }
        case '$': {
          base = 16;
          state = 5;
          char_const = "";
          break;
        }
        case '&': {
          base = 8;
          state = 6;
          char_const = "";
          break;
        }
      }
      continue;
    }
    if (state == 4) {
      switch (e) {
        case '0':
        case '1': {
          char_const += e;
          break;
        }
        case '\'': {
          std::size_t pos = 0;
          char c = std::stoi(char_const, &pos, base);
          value_ += c;
          break;
        }
      }
      continue;
    }
    if (state == 5) {
      switch (e) {
        case '0': case '1': case '2':
        case '3': case '4': case '5':
        case '6': case '7': case '8':
        case '9': case 'a': case 'b':
        case 'c': case 'd': case 'e':
        case 'f': case 'A': case 'B':
        case 'C': case 'D': case 'E':
        case 'F': {
          char_const += e;
          break;
        }
        case '\'': {
          std::size_t pos = 0;
          char c = std::stoi(char_const,  &pos, base);
          value_ += c;
          break;
        }
      }
      continue;
    }
    if (state == 6) {
      switch (e) {
        case '0': case '1': case '2':
        case '3': case '4': case '5':
        case '6': case '7': {
          char_const += e;
          break;
        }
        case '\'': {
          std::size_t pos = 0;
          char c = std::stoi(char_const,  &pos, base);
          value_ += c;
          break;
        }
      }
    }
    continue;
  }

  if (state !=  2) {
    std::size_t pos = 0;
    char c = std::stoi(char_const, &pos, base);
    value_ += c;
  }

}

std::string ConstString::to_string() {
  return TokenBase::to_string() + "\t" +  "\"" + value_ + "\"";
}

