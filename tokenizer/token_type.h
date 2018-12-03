#pragma once

#include <string>
#include <map>

#define TOKEN_TYPE(X, XX) \
  X(Int, "Int") \
  X(Id, "Id") \
  X(Double, "Double") \
  X(String, "String") \
  X(Plus, "+") \
  X(Minus, "-") \
  X(Asterisk, "*") \
  X(Slash, "/") \
  X(DoubleAsterisk, "**") \
  X(At, "@") \
  X(Caret, "^") \
  X(ShiftLeft, "<<") \
  X(ShiftRight, ">>") \
  X(SymmetricDiff, "><") \
  X(Assignment, ":=") \
  X(AssignmentWithPlus, "+=") \
  X(AssignmentWithMinus, "-=") \
  X(AssignmentWithAsterisk, "*=") \
  X(AssignmentWithSlash, "/=") \
  X(Equals, "=") \
  X(StrictLess, "<") \
  X(LessOrEquals, "<=") \
  X(NotEquals, "<>") \
  X(StrictGreater, ">") \
  X(GreaterOrEquals, ">=") \
  X(Colon, ":") \
  X(Comma, ",") \
  X(Dot, ".") \
  X(DoubleDot, "..") \
  X(Semicolon, ";") \
  X(OpenParenthesis, "(") \
  X(CloseParenthesis, ")") \
  X(OpenSquareBracket, "[") \
  X(CloseSquareBracket, "]") \
  X(Comment, "Comment") \
  X(EndOfFile, "EOF")


#define KEYWORD_TYPE(X) \
  X(Absolute, "absolute") \
  X(And, "and") \
  X(Array, "array") \
  X(Asm, "asm") \
  X(Begin, "begin") \
  X(Case, "case") \
  X(Const, "const") \
  X(Constructor, "constructor") \
  X(Destructor, "destructor") \
  X(Div, "div") \
  X(Do, "do") \
  X(Downto, "downto") \
  X(Else, "else") \
  X(End, "end") \
  X(File, "file") \
  X(For, "for") \
  X(Function, "function") \
  X(Goto, "goto") \
  X(If, "if") \
  X(implementation, "implementation") \
  X(In, "in") \
  X(Inherited, "inherited") \
  X(Inline, "inline") \
  X(Interface, "interface") \
  X(Label, "label") \
  X(Mod, "mod") \
  X(Nil, "nil") \
  X(Not, "not") \
  X(Object, "object") \
  X(of, "of") \
  X(Operator, "operator") \
  X(Or, "or") \
  X(Packed, "packed") \
  X(Procedure, "procedure") \
  X(Program, "program") \
  X(Record, "record") \
  X(Reintroduce, "reintroduce") \
  X(Repeat, "repeat") \
  X(Self, "self") \
  X(Set, "set") \
  X(Shl, "shl") \
  X(Shr, "shr") \
  X(KeyString, "string") \
  X(Then, "then") \
  X(To, "to") \
  X(Type, "type") \
  X(Unit, "unit") \
  X(Until, "until") \
  X(Uses, "uses") \
  X(Var, "var") \
  X(While, "while") \
  X(With, "with") \
  X(Xor, "xor")

namespace tok {

#define MAKE_ENUM(E, S) E,
#define MAKE_ENUM_WITH_N(E, S, N) E = N,
enum class TokenType {
  TOKEN_TYPE(MAKE_ENUM, MAKE_ENUM_WITH_N)
  KEYWORD_TYPE(MAKE_ENUM)
};
#undef MAKE_ENUM_WITH_N

#undef MAKE_ENUM

std::string toStringGroup(TokenType);

std::string toString(TokenType t);

TokenType getKeywordType(const std::string &);

bool isKeyword(const std::string &);

} // namespace tok