#include "visitor.h"
#include "token.h"

using namespace pr;

PrintVisitor::PrintVisitor(std::ofstream& out) : out(out), depth(0) {}

void PrintVisitor::print(const std::string& s) {
  out << std::string(depth, '\t') << s << std::endl;
}

void PrintVisitor::visit(pr::Literal& l) {
  print(l.getValue()->getValueString());
}

void PrintVisitor::visit(pr::Variable& v) {
  print(v.getName()->getValueString());
}

void PrintVisitor::visit(pr::BinaryOperation& b) {
  print(b.getOpr()->getValueString());

  ++depth;
  b.getLeft()->accept(*this);
  b.getRight()->accept(*this);
  --depth;
}

void PrintVisitor::visit(pr::UnaryOperation& u) {
  print(u.getOpr()->getValueString());

  ++depth;
  u.getExpr()->accept(*this);
  --depth;
}

void PrintVisitor::visit(pr::ArrayAccess& a) {
  print("Array Access");

  ++depth;
  a.getName()->accept(*this);
  for (auto& e: a.getListIndex()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(pr::FunctionCall& f) {
  print("Function Call");

  ++depth;
  f.getName()->accept(*this);
  for (auto& e: f.getParam()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(pr::RecordAccess& r) {
  print("Record Access");

  ++depth;
  r.getRecord()->accept(*this);
  print(r.getField()->getValueString());
  --depth;
}