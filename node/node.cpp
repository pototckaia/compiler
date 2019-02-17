#include "node.h"
#include "visitor.h"

ASTNode::ASTNode()
	: declPoint(-1, -1, TokenType::Non) {}

ASTNode::ASTNode(int line, int column)
	: declPoint(line, column, TokenType::Non) {}

ASTNode::ASTNode(const Token& t)
	: declPoint(t) {}

void ASTNode::setDeclPoint(const Token& t) {
	declPoint = t;
}

int ASTNode::getDeclLine() { return declPoint.getLine(); }
int ASTNode::getDeclColumn() { return declPoint.getColumn(); }

Expression::Expression(ptr_Type t)
  : type(std::move(t)) {}

Variable::Variable(const Token& n)
  : ASTNode(n), name(n) {}

Variable::Variable(const Token& n, ptr_Type t)
  : ASTNode(n), Expression(std::move(t)), name(n) {}

Literal::Literal(const Token& v)
  : ASTNode(v), value(v) {}

Literal::Literal(const Token& v, ptr_Type t)
	: ASTNode(v), Expression(std::move(t)), value(v) {};

BinaryOperation::BinaryOperation(const Token& op, ptr_Expr left, ptr_Expr right)
  : ASTNode(op),
    op(op), left(std::move(left)), right(std::move(right)) {}

UnaryOperation::UnaryOperation(Token opr, ptr_Expr expr)
  : ASTNode(opr.getLine(), opr.getColumn()), opr(std::move(opr)), expr(std::move(expr)) {}

ArrayAccess::ArrayAccess(const Token& d, ptr_Expr name, ListExpr i)
  : ASTNode(d), nameArray(std::move(name)), listIndex(std::move(i)) {}

FunctionCall::FunctionCall(const Token& d, ptr_Expr nameFunction, ListExpr listParam)
  : ASTNode(d), nameFunction(std::move(nameFunction)),  listParam(std::move(listParam)) {}

FunctionCallStmt::FunctionCallStmt(ptr_Expr e) : functionCall(std::move(e)) {}

Cast::Cast(ptr_Type to, ptr_Expr expr)
 : Expression(std::move(to)), expr(std::move(expr)) {}
Cast::Cast(FunctionCall f)
  : ASTNode(f.getDeclPoint()), Expression(std::move(f.getNodeType())),
    expr(std::move(f.listParam.back())) {}

RecordAccess::RecordAccess(const Token& d, ptr_Expr record, Token field)
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

