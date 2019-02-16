#include "node.h"
#include "visitor.h"


Variable::Variable(std::unique_ptr<Token> n)
  : ASTNode(n->getLine(), n->getColumn()), name(std::move(n)) {}
Variable::Variable(std::unique_ptr<Token> n, ptr_Type t)
  : ASTNode(-1, -1), Expression(std::move(t)), name(std::move(n)) {}

Literal::Literal(std::unique_ptr<Token> v)
  : ASTNode(v->getLine(), v->getColumn()), value(std::move(v)) {}

BinaryOperation::BinaryOperation(std::unique_ptr<Token> op,
                                 ptr_Expr left,
                                 ptr_Expr right)
  : ASTNode(op->getLine(), op->getColumn()),
    opr(std::move(op)), left(std::move(left)), right(std::move(right)) {

}

UnaryOperation::UnaryOperation(std::unique_ptr<Token> opr, ptr_Expr expr)
  : ASTNode(opr->getLine(), opr->getColumn()), opr(std::move(opr)), expr(std::move(expr)) {}

ArrayAccess::ArrayAccess(const ptr_Token& d, ptr_Expr name, ListExpr i)
  : ASTNode(d), nameArray(std::move(name)), listIndex(std::move(i)) {}

FunctionCall::FunctionCall(const ptr_Token& d, ptr_Expr nameFunction, ListExpr listParam)
  : ASTNode(d), nameFunction(std::move(nameFunction)),  listParam(std::move(listParam)) {}

AssignmentStmt::AssignmentStmt(ptr_Token op, ptr_Expr l, ptr_Expr r)
  : ASTNode(op), BinaryOperation(std::move(op), std::move(l), std::move(r)) {}

FunctionCallStmt::FunctionCallStmt(ptr_Expr e) : functionCall(std::move(e)) {}

Cast::Cast(ptr_Type to, ptr_Expr expr)
 : Expression(std::move(to)), expr(std::move(expr)) {}
Cast::Cast(FunctionCall f)
  : ASTNode(f.line, f.column), Expression(std::move(f.type)),
    expr(std::move(f.listParam.back())) {}

RecordAccess::RecordAccess(const ptr_Token& d, ptr_Expr record, std::unique_ptr<Token> field)
  : ASTNode(d), record(std::move(record)), field(std::move(field)) {}

BlockStmt::BlockStmt(ListStmt block) : stmts(std::move(block)) {}

IfStmt::IfStmt(ptr_Expr cond, ptr_Stmt then_)
  : condition(std::move(cond)), then_stmt(std::move(then_)) {}

IfStmt::IfStmt(ptr_Expr cond, ptr_Stmt then_, ptr_Stmt else_)
  : condition(std::move(cond)),
    then_stmt(std::move(then_)) , else_stmt(std::move(else_)) {}

WhileStmt::WhileStmt(ptr_Expr cond, ptr_Stmt block)
  : condition(std::move(cond)), block(std::move(block)) {}

ForStmt::ForStmt(std::unique_ptr<Variable> v,
                 ptr_Expr l, ptr_Expr h, bool d,
                 ptr_Stmt block)
  : var(std::move(v)), block(std::move(block)),
    low(std::move(l)), high(std::move(h)), direct(d) {}


void Variable::accept(Visitor& v) { v.visit(*this); }
void Literal::accept(Visitor& v) { v.visit(*this); }
void BinaryOperation::accept(Visitor& v) { v.visit(*this); }
void UnaryOperation::accept(Visitor& v) { v.visit(*this); }
void ArrayAccess::accept(Visitor& v) { v.visit(*this); }
void FunctionCall::accept(Visitor& v) { v.visit(*this); }
void Cast::accept(Visitor& v) { v.visit(*this); }
void RecordAccess::accept(Visitor& v) { v.visit(*this); }
void AssignmentStmt::accept(Visitor& v) { v.visit(*this); }
void FunctionCallStmt::accept(Visitor& v) { v.visit(*this); }
void BlockStmt::accept(Visitor& v)  { v.visit(*this); }
void IfStmt::accept(Visitor& v)  { v.visit(*this); }
void WhileStmt::accept(Visitor& v)  { v.visit(*this); }
void ForStmt::accept(Visitor& v)  { v.visit(*this); }
void BreakStmt::accept(Visitor& v)  { v.visit(*this); }
void ContinueStmt::accept(Visitor& v)  { v.visit(*this); }

