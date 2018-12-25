#include "visitor_type.h"

#include "../exception.h"

CheckType::CheckType(StackTable s) : stackTable(std::move(s)) {}

void CheckType::visit(Literal& l) {
  switch (l.getValue()->getTokenType()) {
    case tok::TokenType::Int: {
      l.typeExpression = std::make_shared<Int>();
      break;
    }
    case tok::TokenType::Double: {
      l.typeExpression = std::make_shared<Double>();
      break;
    }
    case tok::TokenType::Nil: {
      l.typeExpression = std::make_shared<TPointer>();
      break;
    }
    case tok::TokenType::String: {
      if (l.getValue()->getValueString().size() > 1) {
        l.typeExpression = std::make_shared<String>();
      } else {
        l.typeExpression = std::make_shared<Char>();
      }
      break;
    }
    case tok::TokenType::False:
    case tok::TokenType::True: {
      l.typeExpression = std::make_shared<Boolean>();
      break;
    }
    default:
      throw std::logic_error("Error TokenType in Literal");
  }
}

void CheckType::visit(Variable& v) {
  if (!stackTable.isVar(v.getName()->getValueString())) {
    throw NotDefinedException(v.getName());
  }
  v.typeExpression = stackTable.findVar(v.getName()->getValueString())->type;
}

bool CheckType::implicitCastInt(BinaryOperation& b) {
  auto& leftType = b.getLeft()->typeExpression;
  auto& rightType = b.getRight()->typeExpression;
  auto TypeDouble = std::make_shared<Double>();

  if (leftType->isInt() && rightType->isDouble()) {
    b.left = std::make_unique<StaticCast>(TypeDouble, std::move(b.left));
    b.typeExpression = TypeDouble;
    return true;
  } else if (leftType->isDouble() && rightType->isInt()) {
    b.right = std::make_unique<StaticCast>(TypeDouble, std::move(b.right));
    b.typeExpression = TypeDouble;
    return true;
  }
  return false;
}

bool CheckType::checkTypePlusMinus(BinaryOperation& b) {
  auto& leftType = b.getLeft()->typeExpression;
  auto& rightType = b.getRight()->typeExpression;

  auto notValidType = [](ptr_Type t) -> bool {
    return !(t->isInt() || t->isDouble() || t->isPurePointer() || t->isTypePointer());
  };

  if (notValidType(leftType) || notValidType(rightType)) {
    return false;
  }

  if (implicitCastInt(b)) { return true; }

  if (b.getOpr()->getTokenType() == tok::TokenType::Plus) {
    if (leftType->isPointer() && rightType->isPointer()) {
      return false;
    }

    if (leftType->equals(rightType.get())) {
      b.typeExpression = leftType;
      return true;
    }

    if (leftType->isPointer() && rightType->isInt()) {
      b.typeExpression = leftType;
      return true;
    } else if (leftType->isInt() && rightType->isPointer()) {
      b.typeExpression = rightType;
      return true;
    }

  } else {
    if (leftType->isPointer() && rightType->isPointer()) {
      b.typeExpression = std::make_shared<Int>();
      return true;
    }

    if (leftType->equals(rightType.get())) {
      b.typeExpression = leftType;
      return true;
    }

    if (leftType->isPointer() && rightType->isInt()) {
      b.typeExpression = leftType;
      return true;
    }
  }
  return false;
}

void CheckType::visit(BinaryOperation& b) {
  b.getLeft()->accept(*this);
  b.getRight()->accept(*this);

  auto& leftType = b.getLeft()->typeExpression;
  auto& rightType = b.getRight()->typeExpression;

  std::string mes = "Operation " + b.getOpr()->getValueString() +
    " to types \"" + leftType->name + "\" and \"" + rightType->name + "\" not valid";
  bool isPass = true;

  switch(b.getOpr()->getTokenType()) {
    case tok::TokenType::Plus:
    case tok::TokenType::Minus: { // + -
      isPass = checkTypePlusMinus(b);
      break;
    }
    case tok::TokenType::Asterisk:
    case tok::TokenType::Slash: { // *
      if (implicitCastInt(b)) {
        isPass = true;
        break;
      }
      isPass = (leftType->isDouble() || leftType->isInt()) && leftType->equals(rightType.get());
      b.typeExpression = leftType;
      if (b.getOpr()->getTokenType() == tok::TokenType::Slash) {
        b.typeExpression = std::make_shared<Double>();
      }
      break;
    }
    case tok::TokenType::Div:
    case tok::TokenType::Mod:
    case tok::TokenType::ShiftLeft:
    case tok::TokenType::Shl:
    case tok::TokenType::ShiftRight:
    case tok::TokenType::Shr: {
      isPass = leftType->isInt() && rightType->isInt();
      b.typeExpression = leftType;
      break;
    }
    case tok::TokenType::And:
    case tok::TokenType::Or:
    case tok::TokenType::Xor: {
      isPass = (leftType->isInt() || leftType->isBool()) && leftType->equals(rightType.get());
      b.typeExpression = leftType;
      break;
    }
    case tok::TokenType::Equals:
    case tok::TokenType::NotEquals: {
      b.typeExpression = std::make_shared<Boolean>();
      if (leftType->isString() || rightType->isString()) {
        isPass = false;
        break;
      }
      if (leftType->equals(rightType.get())) {
        isPass = true;
        break;
      }
      isPass = implicitCastInt(b);
      break;
    }
    case tok::TokenType::StrictLess:
    case tok::TokenType::StrictGreater:
    case tok::TokenType::LessOrEquals:
    case tok::TokenType::GreaterOrEquals: {
      isPass = ((leftType->isInt() || leftType->isDouble() || leftType->isChar()) &&
        leftType->equals(rightType.get())) || implicitCastInt(b);
      b.typeExpression = std::make_shared<Boolean>();
      break;
    }
    default: {
      throw std::logic_error("Not valid BinaryOperation tokenType " + tok::toString(b.getOpr()->getTokenType()));
    }
  }

  if(!isPass) {
    throw SemanticException(b.getOpr(), mes);
  }
}

void CheckType::visit(UnaryOperation& u) {
  u.getExpr()->accept(*this);
  auto& childType = u.getExpr()->typeExpression;
  bool isPass = false;
  std::string mes = "Operation " + u.getOpr()->getValueString() +
    " to types \"" + childType->name + "\" not valid";;

  switch (u.getOpr()->getTokenType()) {
    case tok::TokenType::Plus:
    case tok::TokenType::Minus: {
      isPass = childType->isInt() || childType->isDouble();
      u.typeExpression = childType;
      break;
    }
    case tok::TokenType::Not: {
      isPass = childType->isInt() || childType->isBool();
      u.typeExpression = childType;
      break;
    }
    case tok::TokenType::Caret: {
      break;
    }
    case tok::TokenType::At: {
      break;
    }
    default:
      throw std::logic_error("Not valid UnaryOperation tokenType " + tok::toString(u.getOpr()->getTokenType()));
  }

  if (!isPass) {
    throw SemanticException(u.getOpr(), mes);
  }
}

void CheckType::visit(ArrayAccess&) {}
void CheckType::visit(RecordAccess&) {}
void CheckType::visit(FunctionCall&) {}
void CheckType::visit(StaticCast&) {}

void CheckType::visit(AssignmentStmt& a) {
  a.getLeft()->accept(*this);
  a.getRight()->accept(*this);
}

void CheckType::visit(FunctionCallStmt& f) {
  f.getFunctionCall()->accept(*this);
}

void CheckType::visit(BlockStmt& b) {
  for (auto& e: b.getBlock()) {
    e->accept(*this);
  }
}

void CheckType::visit(IfStmt& i) {
  i.getCondition()->accept(*this);
  i.getThen()->accept(*this);
  if (i.getElse() != nullptr) {
    i.getElse()->accept(*this);
  }
  if (!i.getCondition()->typeExpression->isBool()) {
    throw SemanticException(1, 1, "Expect type bool, but find " + i.getCondition()->typeExpression->name);
  }
}

void CheckType::visit(WhileStmt& w) {
  w.getCondition()->accept(*this);
  w.getBlock()->accept(*this);
  if (!w.getCondition()->typeExpression->isBool()) {
    throw SemanticException(1, 1, "Expect type bool, but find " + w.getCondition()->typeExpression->name);
  }
}

void CheckType::visit(ForStmt& f) {
  f.getVar()->accept(*this);
  f.getLow()->accept(*this);
  f.getHigh()->accept(*this);
  f.getBlock()->accept(*this);
  if (!(f.getVar()->typeExpression->isInt() && f.getLow()->typeExpression->isInt() &&
        f.getHigh()->typeExpression->isInt())) {
    throw SemanticException(1, 1, "Loop variable must be type int");
  }
}

void CheckType::visit(ForwardFunction& f) {
  f.function->accept(*this);
}

void CheckType::visit(Function&) {}
void CheckType::visit(MainFunction&) {}
void CheckType::visit(Read&) {}
void CheckType::visit(Write&) {}
void CheckType::visit(Trunc&) {}
void CheckType::visit(Round&) {}
void CheckType::visit(Succ&) {}
void CheckType::visit(Prev&) {}
void CheckType::visit(Chr&) {}
void CheckType::visit(Ord&) {}
void CheckType::visit(High&) {}
void CheckType::visit(Low&) {}