#include <iostream>
#include <fstream>
#include <algorithm>

#include "lexer.h"
#include "lexer_exception.h"

int main(int argc, char *argv[]) {
  std::string inputFileName, outputFileName;

  if (argc < 2 || argc > 5 || argc == 3) {
    std::cerr << "Error in params" << std::endl;
    return 0;
  }

  if (argc == 2) {
    inputFileName = argv[1];
    outputFileName = inputFileName;

    auto typeFile = std::find_if(outputFileName.rbegin(), outputFileName.rend(), [](int ch){ return ch == '.'; });
    outputFileName.erase(typeFile.base() - 1, outputFileName.end());
    outputFileName += ".out";
  } else if (argv[1][0] == '-' || argv[2][0] == '-') {
    int inIndex = 1, outIndex = 3;

    if (argv[1][0] == '-') {
      inIndex = 3, outIndex = 2;
    }

    inputFileName = argv[inIndex];
    outputFileName = argv[outIndex];
  } else {
    std::cerr << "Error in params" << std::endl;
    return 0;
  }


  std::ofstream wfile;
  wfile.open(outputFileName, std::ifstream::out);

  Lexer lex(inputFileName);

  try {
    for (auto token = lex.next(); token != nullptr; token = lex.next()) {
      wfile << token->toString() << std::endl;
    }
  } catch(lxerr::LexerException& e) {
    wfile << e.what() << std::endl;
  }

  wfile.close();
}