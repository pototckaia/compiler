#include "visitor.h"
#include "token.h"

using namespace pr;

PrintVisitor::PrintVisitor(std::ofstream& out) : out(out), depth(0) {}

void PrintVisitor::visit(pr::Literal& l) {
  out << std::string(depth, '\t') << l.getValue()->getValueString() << std::endl;
}

void PrintVisitor::visit(pr::Variable& v) {
  out << std::string(depth, '\t') << v.getName()->getValueString() << std::endl;
}

void PrintVisitor::visit(pr::BinaryOperation& b) {
  out << std::string(depth, '\t') << b.getOpr()->getValueString() << std::endl;

  ++depth;
  b.getLeft()->accept(*this);
  b.getRight()->accept(*this);
  --depth;
}
