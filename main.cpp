#include <iostream>
#include <fstream>

#include "lexer.h"
#include "lexer_exception.h"

int main() {

  std::string file_name = "input.txt";;
  std::ofstream wfile;
  wfile.open("out.txt", std::ifstream::out);

  Lexer l(file_name);


  while(!l.is_end()) {
    try {
      auto token = l.next();
      if (token->token_type_ == tok::TokenType::Comment) {
        continue;
      }
      wfile << token->to_string() << std::endl;
    } catch (std::exception &e) {
      wfile << e.what() << std::endl;
    }
  }

  wfile.close();
}