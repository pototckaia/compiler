#pragma once

#include <string>

struct IllegalCharacter : public std::exception
{

  std::string s;
  IllegalCharacter(int line, int col, char ss) : s("("+ std::to_string(line) + ", " + std::to_string(col) + " ) Error: Illegal Character \"" + ss + "\" ") {}
  ~IllegalCharacter() throw () {} // Updated
  const char* what() const throw() { return s.c_str(); }
};

struct IllegalStringConstant : public std::exception
{

  std::string s;
  IllegalStringConstant(int line, int col, std::string ss) : s("("+ std::to_string(line) + ", " + std::to_string(col) + ") Error: Illegal String Constant \"" + ss + "\" ") {}
  ~IllegalStringConstant() throw () {} // Updated
  const char* what() const throw() { return s.c_str(); }
};