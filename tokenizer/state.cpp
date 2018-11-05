#include "state.h"
#include "state_table.h"


LexerStates::LexerStates() : not_putback_state_({-12, -13, -14, -15, -16,
                                                 -17, -26, -20, -19, -24,
                                                 -25, -23, -28, -29, -30,
                                                 -31, -32, -33, -34, -36,
                                                 -39, -40}),
                           state_table_({STATE_TABLE}), size_state_table_(34) {
  initFinishState();
}

int LexerStates::move(int prev_state, char c) const {
  if (c < 0) { c = 4; }
  if (is_final(prev_state)) {
    throw std::exception();
  }

  if (prev_state >= size_state_table_) {
    throw std::out_of_range("state table out of range");
  }
  return state_table_[prev_state][c];
}

bool LexerStates::is_final(int state) const {
  return state < 0;
}

bool LexerStates::is_putback(int state) const {
  return not_putback_state_.count(state) == 0;
}

bool LexerStates::is_skip(int state, char c) const {
  return state == whitespace_;
}


tok::TokenType LexerStates::to_token_type(int state) const {
  return final_state_.at(state);
}

void LexerStates::initFinishState() {
  final_state_ = {
    {-4, tok::TokenType::Id},
    {-35, tok::TokenType::Id},

    {-2, tok::TokenType::Int}, 
    {-8, tok::TokenType::Int}, 
    {-10, tok::TokenType::Int}, 
    {-16, tok::TokenType::Int}, 
    {-39, tok::TokenType::Int}, 
    
    {-1, tok::TokenType::Double}, 
    {-3, tok::TokenType::String}, 
    
    {-5, tok::TokenType::Assignment}, 
    {-6, tok::TokenType::AssignmentWithPlus}, 
    {-7, tok::TokenType::AssignmentWithMinus}, 
    {-8, tok::TokenType::AssignmentWithAsterisk}, 
    {-9, tok::TokenType::AssignmentWithSlash}, 
    
    {-37, tok::TokenType::Plus}, 
    {-11, tok::TokenType::Minus}, 
    {-12, tok::TokenType::Asterisk}, 
    {-13, tok::TokenType::Slash}, 
    {-14, tok::TokenType::DoubleAsterisk}, 
    {-15, tok::TokenType::At}, 
    {-38, tok::TokenType::Caret}, 
    {-17, tok::TokenType::SymmetricDiff}, 
    {-18, tok::TokenType::ShiftRight}, 
    {-19, tok::TokenType::ShiftLeft}, 
    
    {-20, tok::TokenType::Equals}, 
    {-22, tok::TokenType::StrictLess}, 
    {-23, tok::TokenType::LessOrEquals}, 
    {-21, tok::TokenType::StrictGreater}, 
    {-24, tok::TokenType::GreaterOrEquals}, 
    {-25, tok::TokenType::NotEquals}, 
    
    {-26, tok::TokenType::Colon}, 
    {-31, tok::TokenType::Comma}, 
    {-28, tok::TokenType::Dot}, 
    {-29, tok::TokenType::DoubleDot}, 
    {-27, tok::TokenType::Semicolon}, 

    {-30, tok::TokenType::OpenSquareBracket}, 
    {-32, tok::TokenType::CloseSquareBracket}, 
    {-33, tok::TokenType::OpenParenthesis}, 
    {-34, tok::TokenType::CloseParenthesis}, 
  };
}
