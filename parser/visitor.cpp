#include "visitor.h"
#include "token.h"

using namespace pr;

PrintVisitor::PrintVisitor(const std::string& out) : out(out), depth(0) {}

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

void PrintVisitor::visit(pr::AssignmentStmt& a) {
  print("Assigment");

  ++depth;
  print(a.getOpr()->getValueString());
  a.getLeft()->accept(*this);
  a.getRight()->accept(*this);
  --depth;
}

void PrintVisitor::visit(pr::FunctionCallStmt& f) {
  f.getFunctionCall()->accept(*this);
}

void PrintVisitor::visit(pr::BlockStmt& b) {
  print("Block");

  ++depth;
  for (auto& e: b.getBlock()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(pr::IfStmt& i) {
  print("If");

  ++depth;
  i.getCondition()->accept(*this);
  i.getThen()->accept(*this);
  if (i.getElse() != nullptr) {
    i.getElse()->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(pr::WhileStmt& w) {
  print("While");

  ++depth;
  w.getCondition()->accept(*this);
  w.getBlock()->accept(*this);
  --depth;
}

void PrintVisitor::visit(pr::ForStmt& f) {
  print("For");

  ++depth;
  f.getVar()->accept(*this);
  f.getLow()->accept(*this);
  if (f.getDirect())
    print("To");
  else
    print("DownTo");
  f.getHigh()->accept(*this);
  f.getBlock()->accept(*this);
  --depth;
}

void PrintVisitor::visit(pr::BreakStmt&) {
  print("break");
}

void PrintVisitor::visit(pr::ContinueStmt&) {
  print("continue");
}