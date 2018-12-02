#include <iostream>
#include <memory>
#include <fstream>
#include <algorithm>

#include "cxxopts.hpp"

#include "lexer.h"
#include "exception.h"
#include "parser.h"
#include "visitor.h"


void lexerTest(const std::string& inputFileName,const std::string& outputFileName) {
  std::ofstream out;
  out.open(outputFileName, std::ifstream::out);

  lx::Lexer lex(inputFileName);

  try {
    for (auto token = lex.next(); token->getTokenType() != tok::TokenType::EndOfFile; token = lex.next()) {
      out << token->toString() << std::endl;
    }
  } catch(LexerException& e) {
    out << e.what() << std::endl;
  }

  out.close();
}

void parserTest(const std::string& inputFileName, const std::string& outputFileName) {
  std::ofstream out(outputFileName);

  pr::Parser p(inputFileName);
  pr::PrintVisitor v(out);

  try {
    auto tree = p.parse();
    tree->accept(v);
    out << outputFileName;
  } catch(ParserException& e) {
    out << e.what();
  }
}

void parseCommandArgs(int args, char* argv[]);

int main(int argc, char *argv[]) {
  parseCommandArgs(argc, argv);
}

void parseCommandArgs(int args, char* argv[]) {
	cxxopts::Options options(argv[0]);

	std::string input, output;

	options
		.add_options()
        ("i,input", "Input file", cxxopts::value<std::string>(input))
				("o,output", "Output file", cxxopts::value<std::string>(output))
        ("l,lexer", "Generate a stream of tokens", cxxopts::value<bool>())
        ("e,expression", "Build Ast-tree simple pascal expression", cxxopts::value<bool>())
        ("p,parser", "Build Ast-tree pascal program", cxxopts::value<bool>())
	;

	try {
    auto result = options.parse(args, argv);

    if (!result.count("i") || input.empty()) {
      std::cout << "Error command argument: Not specify a input name";
      return;
    }

    if (!result.count("o")) {
      output = input;
      auto typeFile = std::find_if(output.rbegin(), output.rend(), [](int ch){ return ch == '.'; });
      output.erase(typeFile.base() - 1, output.end());
      output += ".out";
    }

    if (result.count("l")) {
      lexerTest(input, output + "lexer");
    }

    if (result.count("e")) {
      parserTest(input, output + "expression");
    }

    if (result.count("p")) {
      parserTest(input, output + "parser");
    }

  } catch (const cxxopts::OptionException& e) {
    std::cout << "Error command argument: " << e.what() << std::endl;
	  return;
	}
}