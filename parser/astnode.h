#pragma once

#include <memory>
#include <list>

#include "token.h"

namespace pr {

class ASTNode;
class Expression;
class ASTNodeStmt;

using ptr_Node = std::shared_ptr<pr::ASTNode>;

using ptr_Token = std::unique_ptr<tok::TokenBase>;
using ptr_Expr = std::unique_ptr<pr::Expression>;
using ptr_Stmt = std::unique_ptr<pr::ASTNodeStmt>;

using ListExpr = std::list<ptr_Expr>;
using ListStmt = std::list<ptr_Stmt>;

class Visitor;

class ASTNode {
 public:
  ASTNode() = default;
  virtual ~ASTNode() = default;
  virtual void accept(Visitor&) = 0;
};

} // pr
