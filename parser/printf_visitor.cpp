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
  print(l.getSubToken().getString());
  print(l.getNodeType());
}

void PrintVisitor::visit(Variable& v) {
  print(v.getSubToken().getString());
  if (v.getNodeType() != nullptr)
    print(v.getNodeType());
  else
    v.getEmbeddedFunction()->accept(*this);
}

void PrintVisitor::visit(BinaryOperation& b) {
  print(b.getOp().getString());
  ++depth;
  print(b.getNodeType());
	b.getSubRight()->accept(*this);
	b.getSubLeft()->accept(*this);
  --depth;
}

void PrintVisitor::visit(UnaryOperation& u) {
  print(u.getOp().getString());
  ++depth;
  print(u.getNodeType());
	u.getSubNode()->accept(*this);
  --depth;
}

void PrintVisitor::visit(ArrayAccess& a) {
  print("Array Access");
  ++depth;
	a.getSubNode()->accept(*this);
  print(a.getNodeType());;
  for (auto& e: a.getListIndex()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(FunctionCall& f) {
  print("Function Call");
  ++depth;
  f.getSubNode()->accept(*this);
  if (f.getNodeType() == nullptr) {
    f.getEmbeddedFunction()->accept(*this);
  } else {
    print(f.getNodeType());
  }
  for (auto& e: f.getListParam()) {
    e->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(RecordAccess& r) {
  print("Record Access");
  ++depth;
  r.getSubNode()->accept(*this);
  print(r.getField().getString());
  print(r.getNodeType());
  --depth;
}

void PrintVisitor::visit(Cast& t) {
  print("Cast");
  t.getNodeType()->accept(*this);
  ++depth;
  t.getSubNode()->accept(*this);
  --depth;
}

// Stmt

void PrintVisitor::visit(AssignmentStmt& a) {
  print("Assigment");
  ++depth;
  print(a.getOp().getString());
	a.getSubLeft()->accept(*this);
	a.getSubRight()->accept(*this);
  --depth;
}

void PrintVisitor::visit(FunctionCallStmt& f) {
  f.getSubNode()->accept(*this);
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
  i.getSubThen()->accept(*this);
  if (i.getSubElse() != nullptr) {
    i.getSubElse()->accept(*this);
  }
  --depth;
}

void PrintVisitor::visit(WhileStmt& w) {
  print("While");
  ++depth;
  w.getCondition()->accept(*this);
  w.getSubNode()->accept(*this);
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
  f.getSubNote()->accept(*this);
  --depth;
}

void PrintVisitor::visit(BreakStmt&) {
  print("break");
}

void PrintVisitor::visit(ContinueStmt&) {
  print("continue");
}


void PrintVisitor::visit(Int& i) {
  print(i.getSymbolName());
}

void PrintVisitor::visit(Double& i) {
  print(i.getSymbolName());
}

void PrintVisitor::visit(Char& i) {
  print(i.getSymbolName());
}

void PrintVisitor::visit(Boolean& i) {
  print(i.getSymbolName());
}

void PrintVisitor::visit(TPointer& i) {
  print(i.getSymbolName());
}

void PrintVisitor::visit(String& s) {
  print(s.getSymbolName());
}

void PrintVisitor::visit(Void& v) {
  print(v.getSymbolName());
}

void PrintVisitor::visit(Alias& a) {
  print("Type alias: " + a.getSymbolName());
  print("Decl point: " + getPoint(a.getDeclPoint()));
  ++depth;
  a.getRefType()->accept(*this);
  --depth;
}

void PrintVisitor::visit(Pointer& p) {
  print("Pointer");
  print("Decl point: " + getPoint(p.getDeclPoint()));
  ++depth;
  print(p.getPointerBase()->getSymbolName());
  --depth;
}

void PrintVisitor::visit(StaticArray& a) {
  print("Static array");
  print("Decl point: " + getPoint(a.getDeclPoint()));
  ++depth;

  print("Size");
  ++depth;
  for (auto& e : a.getBounds()) {
    print("low: " + std::to_string(e.first) + " high: " + std::to_string(e.second));
  }
  --depth;

  a.getRefType()->accept(*this);
  --depth;
}

void PrintVisitor::visit(OpenArray& o) {
  print("Open array");
  print("Decl point: " + getPoint(o.getDeclPoint()));
  ++depth;
  o.typeElem->accept(*this);
  --depth;
}

void PrintVisitor::visit(Record& r) {
  print("Record");
  print("Decl point: " + getPoint(r.getDeclPoint()));
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
  print("Decl point: " + getPoint(s.getDeclPoint()));
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
  print("Forward type " + f.getSymbolName());
  print("Decl point: " + getPoint(f.getDeclPoint()));
  ++depth;
  f.getRefType()->accept(*this);
  --depth;
}

void PrintVisitor::visit(LocalVar& l) {
  print("Local variable: " + l.getSymbolName());
  print("Decl point: " + getPoint(l.getDeclPoint()));
  ++depth;
  l.getVarType()->accept(*this);
  --depth;
}

void PrintVisitor::visit(GlobalVar& l) {
  print("Global variable: " + l.getSymbolName());
  print("Decl point: " + getPoint(l.getDeclPoint()));
  ++depth;
  l.getVarType()->accept(*this);
  --depth;
}

void PrintVisitor::visit(Const& c) {
  print("Const value: " + c.getSymbolName());
  print("Decl point: " + getPoint(c.getDeclPoint()));
  ++depth;
  c.value->accept(*this);
  --depth;
}

void PrintVisitor::visit(ParamVar& p) {
  print("Param value: " + p.getSymbolName());
  print("Decl point: " + getPoint(p.getDeclPoint()));
  ++depth;
  print(toString(p.spec));
  p.getVarType()->accept(*this);
  --depth;
}

void PrintVisitor::visit(ForwardFunction& f) {
  print("Forward function: " + f.getSymbolName());
  print("Decl point: " + getPoint(f.getDeclPoint()));
  ++depth;
  f.getSignature()->accept(*this);
  f.getFunction()->accept(*this);
  --depth;
}

void PrintVisitor::visit(Function& f) {
  print("Function or Procedure: " + f.getSymbolName());
  print("Decl point: " + getPoint(f.getDeclPoint()));
  ++depth;
  f.getSignature()->accept(*this);
  visit(f.getTable());
  f.getBody()->accept(*this);
  --depth;
}

void PrintVisitor::visit(MainFunction& m) {
  visit(m.getTable());
  m.getBody()->accept(*this);
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
  e.getReturnType()->accept(*this);
  --depth;
}