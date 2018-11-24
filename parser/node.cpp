#include "node.h"
#include "visitor.h"

using namespace pr;

Variable::Variable(std::unique_ptr<tok::TokenBase> n) : name(std::move(n)) {}

Literal::Literal(std::unique_ptr<tok::TokenBase> v) : value(std::move(v)) {}

BinaryOperation::BinaryOperation(std::unique_ptr<tok::TokenBase> op,
                                 std::unique_ptr<ASTNode> left,
                                 std::unique_ptr<ASTNode> right)
  : opr(std::move(op)), left(std::move(left)), right(std::move(right)) {}

UnaryOperation::UnaryOperation(std::unique_ptr<tok::TokenBase> opr, std::unique_ptr<pr::ASTNode> expr)
  : opr(std::move(opr)), expr(std::move(expr)) {}

void Variable::accept(pr::Visitor& v) { v.visit(*this); }
void Literal::accept(pr::Visitor& v) { v.visit(*this); }
void BinaryOperation::accept(pr::Visitor& v) { v.visit(*this); }
void UnaryOperation::accept(pr::Visitor& v) { v.visit(*this); }