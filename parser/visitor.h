#pragma once

#include <fstream>

#include "node.h"

namespace pr {


class Visitor {
 public:
  virtual void visit(Variable&) = 0;
  virtual void visit(Literal&) = 0;
  virtual void visit(BinaryOperation&) = 0;
};

class PrintVisitor : public Visitor {
 public:
  explicit PrintVisitor(const std::string& outName);

  void visit(Variable&) override;
  void visit(Literal&) override;
  void visit(BinaryOperation&) override;

 private:
  std::ofstream out;
  int depth;
};

} // namespace pr