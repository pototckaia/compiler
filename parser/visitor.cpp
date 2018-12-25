#include "visitor.h"
#include "token.h"

using namespace pr;

PrintVisitor::PrintVisitor(const std::string& out) : out(out), depth(0) {}

void PrintVisitor::print(const std::string& s) {
  out << std::string(depth, '\t') << s << std::endl;
}

// Expression

void PrintVisitor::visit(pr::Literal& l) {
  print(l.getValue()->getValueString());
  l.typeExpression->accept(*this);
}

void PrintVisitor::visit(pr::Variable& v) {
  print(v.getName()->getValueString());
  v.typeExpression->accept(*this);
}

void PrintVisitor::visit(pr::BinaryOperation& b) {
  print(b.getOpr()->getValueString());
  ++depth;
  b.typeExpression->accept(*this);
  b.getRight()->accept(*this);
  b.getLeft()->accept(*this);
  --depth;
}

void PrintVisitor::visit(pr::UnaryOperation& u) {
  print(u.getOpr()->getValueString());
  ++depth;
  u.typeExpression->accept(*this);
  u.getExpr()->accept(*this);
  --depth;
}

void PrintVisitor::visit(pr::ArrayAccess& a) {
  print("Array Access");
  ++depth;
  a.getName()->accept(*this);
  a.typeExpression->accept(*this);
  for (auto& e: a.getListIndex()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(pr::FunctionCall& f) {
  print("Function Call");
  ++depth;
  f.getName()->accept(*this);
  f.typeExpression->accept(*this);
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
  r.typeExpression->accept(*this);
  --depth;
}

void PrintVisitor::visit(StaticCast& t) {
  print("Static cast");
  t.typeExpression->accept(*this);
  t.expr->accept(*this);
}

// Stmt

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


void PrintVisitor::visit(Int& i) {
  print(i.name);
}

void PrintVisitor::visit(Double& i) {
  print(i.name);
}

void PrintVisitor::visit(Char& i) {
  print(i.name);
}

void PrintVisitor::visit(Boolean& i) {
  print(i.name);
}

void PrintVisitor::visit(TPointer& i) {
  print(i.name);
}

void PrintVisitor::visit(String& s) {
  print(s.name);
}

void PrintVisitor::visit(Alias& a) {
  print("Type alias: " + a.name);
  print("Decl point: " + tok::getPoint(a.line, a.column));
  ++depth;
  a.type->accept(*this);
  --depth;
}

void PrintVisitor::visit(Pointer& p) {
  print("Pointer");
  print("Decl point: " + tok::getPoint(p.line, p.column));
  ++depth;
  p.typeBase->accept(*this);
  --depth;
}

void PrintVisitor::visit(StaticArray& a) {
  print("Static array");
  print("Decl point: " + tok::getPoint(a.line, a.column));
  ++depth;

  print("Size");
  ++depth;
  for (auto& e : a.bounds) {
    print("low: " + std::to_string(e.first) + " high: " + std::to_string(e.second));
  }
  --depth;

  a.typeElem->accept(*this);
  --depth;
}

void PrintVisitor::visit(OpenArray& o) {
  print("Open array");
  print("Decl point: " + tok::getPoint(o.line, o.column));
  ++depth;
  o.typeElem->accept(*this);
  --depth;
}

void PrintVisitor::visit(Record& r) {
  print("Record");
  print("Decl point: " + tok::getPoint(r.line, r.column));
  ++depth;
  visit(r.fields);
  --depth;
}

void PrintVisitor::visit(FunctionSignature& s) {
  if (s.returnType == nullptr) {
    print("Procedure");
  } else {
    print ("Function");
  }
  print("Decl point: " + tok::getPoint(s.line, s.column));
  ++depth;
  for (auto& e : s.paramsList) {
   e->accept(*this);
  }
  if (!s.isProcedure()) {
    s.returnType->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(ForwardType& f) {
  print("Forward type " + f.name);
  print("Decl point: " + tok::getPoint(f.line, f.column));
  ++depth;
  f.type->accept(*this);
  --depth;
}

void PrintVisitor::visit(LocalVar& l) {
  print("Local variable: " + l.name);
  print("Decl point: " + tok::getPoint(l.line, l.column));
  ++depth;
  l.type->accept(*this);
  --depth;
}

void PrintVisitor::visit(GlobalVar& l) {
  print("Global variable: " + l.name);
  print("Decl point: " + tok::getPoint(l.line, l.column));
  ++depth;
  l.type->accept(*this);
  --depth;
}

void PrintVisitor::visit(Const& c) {
  print("Const value: " + c.name);
  print("Decl point: " + tok::getPoint(c.line, c.column));
  ++depth;
  c.value->accept(*this);
  --depth;
}

void PrintVisitor::visit(ParamVar& p) {
  print("Param value: " + p.name);
  print("Decl point: " + tok::getPoint(p.line, p.column));
  ++depth;
  print(toString(p.spec));
  p.type->accept(*this);
  --depth;
}

void PrintVisitor::visit(ForwardFunction& f) {
  print("Forward function: " + f.name);
  print("Decl point: " + tok::getPoint(f.line, f.column));
  ++depth;
  f.signature->accept(*this);
  f.function->accept(*this);
  --depth;
}

void PrintVisitor::visit(Function& f) {
  print("Function decl: " + f.name);
  print("Decl point: " + tok::getPoint(f.line, f.column));
  ++depth;
  f.signature->accept(*this);
  visit(f.localVar);
  f.body->accept(*this);
  --depth;
}

void PrintVisitor::visit(MainFunction& m) {
  visit(m.decl);
  m.body->accept(*this);
}

void PrintVisitor::visit(TableSymbol<ptr_Type>& t) {
  print("Table Symbol Type");
  ++depth;
  for (auto& e : t) {
    print(e.first);
    ++depth;
    e.second->accept(*this);
    --depth;
  }
  --depth;
}

void PrintVisitor::visit(TableSymbol<ptr_Var>& t) {
  print("Table Symbol Variable");
  ++depth;
  for (auto& e : t) {
    print(e.first);
    ++depth;
    e.second->accept(*this);
    --depth;
  }
  --depth;
}

void PrintVisitor::visit(TableSymbol<std::shared_ptr<Const>>& t) {
  print("Table Symbol Const");
  ++depth;
  for (auto& e : t) {
    print(e.first);
    ++depth;
    e.second->accept(*this);
    --depth;
  }
  --depth;
}

void PrintVisitor::visit(TableSymbol<std::shared_ptr<SymFun>>& t) {
  print("Table Symbol Function");
  ++depth;
  for (auto& e : t) {
    print(e.first);
    ++depth;
    e.second->accept(*this);
    --depth;
  }
  --depth;
}

void PrintVisitor::visit(Tables& t) {
  print("Decl");
  ++depth;
  visit(t.tableType);
  visit(t.tableConst);
  visit(t.tableVariable);
  visit(t.tableFunction);
  --depth;
}

void PrintVisitor::visit(Read&) {
  print("Read");
}

void PrintVisitor::visit(Write&) {
  print("Write");
}

void PrintVisitor::visit(Trunc&) {
  print("Trunc");
}

void PrintVisitor::visit(Round&) {
  print("Round");
}

void PrintVisitor::visit(Succ&) {
  print("Succ");
}

void PrintVisitor::visit(Prev&) {
  print("Prev");
}

void PrintVisitor::visit(Chr&) {
  print("Chr");
}

void PrintVisitor::visit(Ord&) {
  print("Ord");
}

void PrintVisitor::visit(High&) {
  print("High");
}

void PrintVisitor::visit(Low&) {
  print("Low");
}

void PrintVisitor::visit(Exit& e) {
  print("Exit");
  ++depth;
  e.returnType->accept(*this);
  --depth;
}