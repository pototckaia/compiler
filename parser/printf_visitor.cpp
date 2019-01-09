#include "visitor.h"
#include "token.h"

PrintVisitor::PrintVisitor(const std::string& out) : out(out), depth(0) {}

void PrintVisitor::print(const std::string& e) {
  out << std::string(depth, '\t') << e << std::endl;
}

void PrintVisitor::print(const ptr_Type& e) {
  print("Type expression");
  ++depth;
  e->accept(*this);
  --depth;
}

// Expression

void PrintVisitor::visit(Literal& l) {
  print(l.getValue()->getValueString());
  print(l.type);
}

void PrintVisitor::visit(Variable& v) {
  print(v.getName()->getValueString());
  if (v.type != nullptr)
    print(v.type);
  else
    v.embeddedFunction->accept(*this);
}

void PrintVisitor::visit(BinaryOperation& b) {
  print(b.getOpr()->getValueString());
  ++depth;
  print(b.type);
  b.getRight()->accept(*this);
  b.getLeft()->accept(*this);
  --depth;
}

void PrintVisitor::visit(UnaryOperation& u) {
  print(u.getOpr()->getValueString());
  ++depth;
  print(u.type);
  u.getExpr()->accept(*this);
  --depth;
}

void PrintVisitor::visit(ArrayAccess& a) {
  print("Array Access");
  ++depth;
  a.getName()->accept(*this);
  print(a.type);;
  for (auto& e: a.getListIndex()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(FunctionCall& f) {
  print("Function Call");
  ++depth;
  f.getName()->accept(*this);
  if (f.type == nullptr) {
    f.embeddedFunction->accept(*this);
  } else {
    print(f.type);
  }
  for (auto& e: f.getParam()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(RecordAccess& r) {
  print("Record Access");
  ++depth;
  r.getRecord()->accept(*this);
  print(r.getField()->getValueString());
  print(r.type);
  --depth;
}

void PrintVisitor::visit(Cast& t) {
  print("Cast");
  t.type->accept(*this);
  ++depth;
  t.expr->accept(*this);
  --depth;
}

// Stmt

void PrintVisitor::visit(AssignmentStmt& a) {
  print("Assigment");
  ++depth;
  print(a.getOpr()->getValueString());
  a.getLeft()->accept(*this);
  a.getRight()->accept(*this);
  --depth;
}

void PrintVisitor::visit(FunctionCallStmt& f) {
  f.getFunctionCall()->accept(*this);
}

void PrintVisitor::visit(BlockStmt& b) {
  print("Block");
  ++depth;
  for (auto& e: b.getBlock()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(IfStmt& i) {
  print("If");
  ++depth;
  i.getCondition()->accept(*this);
  i.getThen()->accept(*this);
  if (i.getElse() != nullptr) {
    i.getElse()->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(WhileStmt& w) {
  print("While");
  ++depth;
  w.getCondition()->accept(*this);
  w.getBlock()->accept(*this);
  --depth;
}

void PrintVisitor::visit(ForStmt& f) {
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

void PrintVisitor::visit(BreakStmt&) {
  print("break");
}

void PrintVisitor::visit(ContinueStmt&) {
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

void PrintVisitor::visit(Void& v) {
  print(v.name);
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
  print(p.typeBase->name);
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
  visit(r.getTable());
  --depth;
}

void PrintVisitor::visit(FunctionSignature& s) {
  if (s.isProcedure()) {
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
    print("Return:");
    ++depth;
    s.returnType->accept(*this);
    --depth;
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
  print("Function or Procedure: " + f.name);
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