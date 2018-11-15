#include "visitor.h"
#include "token.h"

using namespace pr;

PrintVisitor::PrintVisitor(const std::string& outName) : out(outName), depth(0) {}

void PrintVisitor::visit(pr::Literal& l) {
  std::string indent;
  for (int i = 0; i < depth; ++i) {
    indent += "\t";
  }

  out << indent << l.getValue()->getValueString() << std::endl;
}

void PrintVisitor::visit(pr::Variable& v) {
  std::string indent;
  for (int i = 0; i < depth; ++i) {
    indent += "\t";
  }

  out << indent << v.getName()->getValueString() << std::endl;
}

void PrintVisitor::visit(pr::BinaryOperation& b) {
  std::string indent;
  for (int i = 0; i < depth; ++i) {
    indent += "\t";
  }

  out << indent << b.getOpr()->getValueString() << std::endl;

  ++depth;
  b.getLeft()->accept(*this);
  b.getRight()->accept(*this);
  --depth;
}
