#include "node.h"
#include "visitor.h"

using namespace pr;

Variable::Variable(std::unique_ptr<tok::TokenBase> n) : name(std::move(n)) {}

Literal::Literal(std::unique_ptr<tok::TokenBase> v) : value(std::move(v)) {}

BinaryOperation::BinaryOperation(std::unique_ptr<tok::TokenBase> op,
                                 ptr_Expr left,
                                 ptr_Expr right)
  : opr(std::move(op)), left(std::move(left)), right(std::move(right)) {}

UnaryOperation::UnaryOperation(std::unique_ptr<tok::TokenBase> opr, ptr_Expr expr)
  : opr(std::move(opr)), expr(std::move(expr)) {}

ArrayAccess::ArrayAccess(ptr_Expr name, ListExpr i)
  : nameArray(std::move(name)), listIndex(std::move(i)) {}

FunctionCall::FunctionCall(ptr_Expr nameFunction, ListExpr listParam)
  : nameFunction(std::move(nameFunction)),  listParam(std::move(listParam)) {}

FunctionCallStmt::FunctionCallStmt(pr::ptr_Expr e) : functionCall(std::move(e)) {}

StaticCast::StaticCast(ptr_Type to, pr::ptr_Expr expr)
 : Expression(std::move(to)), expr(std::move(expr)) {}


RecordAccess::RecordAccess(ptr_Expr record, std::unique_ptr<tok::TokenBase> field)
  : record(std::move(record)), field(std::move(field)) {}

BlockStmt::BlockStmt(pr::ListStmt block) : stmts(std::move(block)) {}

IfStmt::IfStmt(pr::ptr_Expr cond, pr::ptr_Stmt then_)
  : condition(std::move(cond)), then_stmt(std::move(then_)) {}

IfStmt::IfStmt(pr::ptr_Expr cond, pr::ptr_Stmt then_, pr::ptr_Stmt else_)
  : condition(std::move(cond)),
    then_stmt(std::move(then_)) , else_stmt(std::move(else_)) {}

WhileStmt::WhileStmt(pr::ptr_Expr cond, pr::ptr_Stmt block)
  : condition(std::move(cond)), block(std::move(block)) {}

ForStmt::ForStmt(std::unique_ptr<pr::Variable> v,
                 pr::ptr_Expr l, pr::ptr_Expr h, bool d,
                 pr::ptr_Stmt block)
  : var(std::move(v)), block(std::move(block)),
    low(std::move(l)), high(std::move(h)), direct(d) {}


void Variable::accept(pr::Visitor& v) { v.visit(*this); }
void Literal::accept(pr::Visitor& v) { v.visit(*this); }
void BinaryOperation::accept(pr::Visitor& v) { v.visit(*this); }
void UnaryOperation::accept(pr::Visitor& v) { v.visit(*this); }
void ArrayAccess::accept(pr::Visitor& v) { v.visit(*this); }
void FunctionCall::accept(pr::Visitor& v) { v.visit(*this); }
void StaticCast::accept(pr::Visitor& v) { v.visit(*this); }
void RecordAccess::accept(pr::Visitor& v) { v.visit(*this); }
void AssignmentStmt::accept(pr::Visitor& v) { v.visit(*this); }
void FunctionCallStmt::accept(pr::Visitor& v) { v.visit(*this); }
void BlockStmt::accept(pr::Visitor& v)  { v.visit(*this); }
void IfStmt::accept(pr::Visitor& v)  { v.visit(*this); }
void WhileStmt::accept(pr::Visitor& v)  { v.visit(*this); }
void ForStmt::accept(pr::Visitor& v)  { v.visit(*this); }
void BreakStmt::accept(pr::Visitor& v)  { v.visit(*this); }
void ContinueStmt::accept(pr::Visitor& v)  { v.visit(*this); }

