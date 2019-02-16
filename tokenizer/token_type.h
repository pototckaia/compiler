#pragma once

#include <map>
#include <string>

#define TOKEN_TYPE(X) \
  X(Int,                   "Int") \
  X(Id,                    "Id") \
  X(Double,                "Double") \
  X(String,                "String") \
  X(Plus,                   "+") \
  X(Minus,                  "-") \
  X(Asterisk,               "*") \
  X(Slash,                  "/") \
  X(DoubleAsterisk,         "**") \
  X(At,                     "@") \
  X(Caret,                  "^") \
  X(ShiftLeft,              "<<") \
  X(ShiftRight,             ">>") \
  X(SymmetricDiff,          "><") \
  X(Assignment,             ":=") \
  X(AssignmentWithPlus,     "+=") \
  X(AssignmentWithMinus,    "-=") \
  X(AssignmentWithAsterisk, "*=") \
  X(AssignmentWithSlash,    "/=") \
  X(Equals,                 "=") \
  X(StrictLess,             "<") \
  X(LessOrEquals,           "<=") \
  X(NotEquals,              "<>") \
  X(StrictGreater,          ">") \
  X(GreaterOrEquals,        ">=") \
  X(Colon,                  ":") \
  X(Comma,                  ",") \
  X(Dot,                    ".") \
  X(DoubleDot,              "..") \
  X(Semicolon,              ";") \
  X(OpenParenthesis,        "(") \
  X(CloseParenthesis,       ")") \
  X(OpenSquareBracket,      "[") \
  X(CloseSquareBracket,     "]") \
  X(EndOfFile,              "EOF")


#define KEYWORD_TYPE(X) \
  X(And,       "and") \
  X(Array,     "array") \
  X(Begin,     "begin") \
  X(Break,     "break") \
  X(Const,     "const") \
  X(Continue,  "continue") \
  X(Div,       "div") \
  X(Do,        "do") \
  X(Downto,    "downto") \
  X(Else,      "else") \
  X(End,       "end") \
  X(False,     "false")\
  X(For,       "for") \
  X(Function,  "function") \
  X(Goto,      "goto") \
  X(If,        "if") \
  X(Label,     "label") \
  X(Mod,       "mod") \
  X(Nil,       "nil") \
  X(Not,       "not") \
  X(Of,        "of") \
  X(Or,        "or") \
  X(Out,       "out")\
  X(Procedure, "procedure") \
  X(Program,   "program") \
  X(Record,    "record") \
  X(Repeat,    "repeat") \
  X(Shl,       "shl") \
  X(Shr,       "shr") \
  X(Then,      "then") \
  X(To,        "to") \
  X(True,      "true")\
  X(Type,      "type") \
  X(Until,     "until") \
  X(Var,       "var") \
  X(While,     "while") \
  X(With,      "with") \
  X(Xor,       "xor") \

namespace tok {

#define MAKE_ENUM(E, S) E,
enum class TokenType {
  TOKEN_TYPE(MAKE_ENUM)
  KEYWORD_TYPE(MAKE_ENUM)
};
#undef MAKE_ENUM

std::string getGroup(TokenType);

std::string toString(TokenType);

TokenType getKeywordType(const std::string&);

bool isKeyword(const std::string&);

} // namespace tok