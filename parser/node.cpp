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

RecordAccess::RecordAccess(ptr_Expr record, std::unique_ptr<tok::TokenBase> field)
  : record(std::move(record)), field(std::move(field)) {}

AssignmentStmt::AssignmentStmt(std::unique_ptr<tok::TokenBase> op,
                               pr::ptr_Expr left, pr::ptr_Expr right)
  : BinaryOperation(std::move(op), std::move(left), std::move(right)) {}

void Variable::accept(pr::Visitor& v) { v.visit(*this); }
void Literal::accept(pr::Visitor& v) { v.visit(*this); }
void BinaryOperation::accept(pr::Visitor& v) { v.visit(*this); }
void UnaryOperation::accept(pr::Visitor& v) { v.visit(*this); }
void ArrayAccess::accept(pr::Visitor& v) { v.visit(*this); }
void FunctionCall::accept(pr::Visitor& v) { v.visit(*this); }
void RecordAccess::accept(pr::Visitor& v) { v.visit(*this); }
void AssignmentStmt::accept(pr::Visitor& v) { v.visit(*this); }

