#include "visitor_type.h"

#include "../exception.h"

void CheckLvalue::visit(Literal& l) { lvalue = false; }
void CheckLvalue::visit(BinaryOperation&) { lvalue = false; }
void CheckLvalue::visit(FunctionCall&) { lvalue = false; }
void CheckLvalue::visit(StaticCast&) { lvalue = false; }
void CheckLvalue::visit(Variable&) { lvalue = true; }

void CheckLvalue::visit(ArrayAccess& a) { a.getName()->accept(*this); }
void CheckLvalue::visit(RecordAccess& r) { r.getRecord()->accept(*this); }

void CheckLvalue::visit(UnaryOperation& u) {
  u.getExpr()->accept(*this);
  lvalue = lvalue && u.getOpr()->getTokenType() == tok::TokenType::Caret;
}

void CheckTypeExpressionBase::visit(Int&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(Double&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(Char&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(TPointer&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(String&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(Boolean&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(FunctionSignature&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(Record&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(Pointer&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(StaticArray&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(OpenArray&) { throw SemanticException(0, 0, errorMes); }
void CheckTypeExpressionBase::visit(Alias& a) { a.accept(*this); }
void CheckTypeExpressionBase::visit(ForwardType& a) { a.accept(*this); }

void CheckTypeArrayAccess::visit(Pointer& a) {
  --sizeBounds;
  if (sizeBounds > 1) {
    a.typeBase->accept(*this);
  } else {
    arrayAccess.typeExpression = a.typeBase;
  }
}

void CheckTypeArrayAccess::visit(StaticArray& s) {
  --sizeBounds;
  StaticArray t(s);
  t.bounds.pop_front();
  if (sizeBounds > 1) {
    if (t.bounds.empty()) {
      t.typeElem->accept(*this);
    } else {
      visit(t);
    }
  } else {
    arrayAccess.typeExpression = std::make_shared<StaticArray>(t);
  }
}

void CheckTypeArrayAccess::visit(OpenArray& o) {
  --sizeBounds;
  if (sizeBounds > 1) {
    o.typeElem->accept(*this);
  } else {
    arrayAccess.typeExpression = o.typeElem;
  }
}

void CheckTypeRecordAccess::visit(Record& r) {
  if (!r.fields.checkContain(recordAccess.getField()->getValueString())) {
    throw NotDefinedException(recordAccess.getField());
  }
  recordAccess.typeExpression = r.fields.find(recordAccess.getField()->getValueString())->type;
}

void CheckTypeFunctionCall::visit(FunctionSignature& s) {
  if (s.paramsList.size() != f.getParam().size()) {
    throw SemanticException(0, 0, "Except number of arguments " + std::to_string(s.paramsList.size()) +
      " but find " + std::to_string(f.getParam().size()));
  }
  f.typeExpression = s.returnType;
  auto iterParameter = s.paramsList.begin();
  ListExpr newParam;
  for (; iterParameter != s.paramsList.end(); ++iterParameter) {
    auto parameter = *iterParameter;
    auto argument = std::move(f.listParam.front());
    f.listParam.pop_front();
    if (parameter->spec == ParamSpec::Var ||
       parameter->spec == ParamSpec::Out) {
      CheckLvalue checker;
      argument->accept(checker);
      if (!checker.isLvalue()) {
        throw SemanticException(0, 0, "Except lvalue in argument");
      }
    }
    if (parameter->type->equalsForCheckArgument(argument->typeExpression.get())) {
      newParam.push_back(std::move(argument));
      continue;
    }
    if (parameter->type->isDouble() && argument->typeExpression->isInt()) {
      auto newArgument = std::make_unique<StaticCast>(parameter->type, std::move(argument));
      newParam.push_back(std::move(newArgument));
      continue;
    }
    throw SemanticException(0,0, "Except argument's type " + parameter->type->name +
                            " but find " + argument->typeExpression->name);
  }
  f.listParam = std::move(newParam);
}

void CheckTypeFunctionCall::visit(Read&) {
  f.typeExpression = std::make_shared<Void>();
  for (auto& e : f.getParam()) {
    auto& type = e->typeExpression;
    CheckLvalue checker;
    e->accept(checker);
    if (!checker.isLvalue()) {
      throw SemanticException(0, 0, "Except lvalue in argument");
    }
    if (type->isInt() || type->isDouble() || type->isChar()) {
      continue;
    }
    throw SemanticException(0,0, "Except readable type but find " + type->name);
  }
}

void CheckTypeFunctionCall::visit(Write&) {
  f.typeExpression = std::make_shared<Void>();
  for (auto& e : f.getParam()) {
    auto& type = e->typeExpression;
    if (type->isInt() || type->isDouble() || type->isChar() || type->isString()) {
      continue;
    }
    throw SemanticException(0,0, "Except writeable type but find " + type->name);
  }
}

void CheckTypeFunctionCall::visit(Chr& c) { c.signature->accept(*this); }
void CheckTypeFunctionCall::visit(Ord& c) { c.signature->accept(*this); }
void CheckTypeFunctionCall::visit(Prev& c) { c.signature->accept(*this); }
void CheckTypeFunctionCall::visit(Succ& c) { c.signature->accept(*this); }
void CheckTypeFunctionCall::visit(Trunc& c) { c.signature->accept(*this); }
void CheckTypeFunctionCall::visit(Round& c) { c.signature->accept(*this); }

void CheckTypeFunctionCall::visit(Exit& c) {
  f.typeExpression = std::make_shared<Void>();
  if (c.returnType->isVoid()) {
    if (!f.getParam().empty()) {
      throw SemanticException(0,0, "Expect 0 arguments but find " + std::to_string(f.getParam().size()));
    }
    return;
  }
  if (f.getParam().size() > 1) {
    throw SemanticException(0, 0, "Expect 1 arguments but find " + std::to_string(f.getParam().size()));
  }
  if (!f.getParam().back()->typeExpression->equalsForCheckArgument(c.returnType.get())) {
    throw SemanticException(0, 0, "Expect type " + c.returnType->name + "but find" +
                              f.getParam().back()->typeExpression->name);
  }
}

void CheckTypeFunctionCall::visit(High&) {
 if (f.getParam().empty() || f.getParam().size() > 1) {
   throw SemanticException(0, 0, "Except argument but find " + std::to_string(f.getParam().size()));
 }
 auto& type = f.getParam().back()->typeExpression;
 if (type->isOpenArray() || type->isStaticArray()) {
   f.typeExpression = std::make_shared<Int>();
   return;
 }
  throw SemanticException(0,0, "Except array type but find " + type->name);
}

void CheckTypeFunctionCall::visit(Low&) { High h; visit(h); }


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
  // TODO const
  if (stackTable.isFunction(v.getName()->getValueString())) {
    auto f = stackTable.findFunction(v.getName()->getValueString());
    if (f->isEmbedded()) {
      v.embeddedFunction = stackTable.findFunction(v.getName()->getValueString());
      v.typeExpression = f->signature;
    } else {
      v.typeExpression = f->signature;
    }
    return;
  }

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
  if (leftType == nullptr || rightType == nullptr) {
    throw SemanticException("Cannot " + b.getOpr()->getValueString() + " function");
  }

  std::string mes = "Operation " + b.getOpr()->getValueString() +
    " to types \"" + leftType->name + "\" and \"" + rightType->name + "\" not valid";
  bool isPass;

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
  if (childType== nullptr) {
    throw SemanticException("Cannot " + u.getOpr()->getValueString() + " function");
  }

  bool isPass = false;
  CheckLvalue lvalue;
  std::string mesLvalue = "Except lvalue in operator " + tok::toString(u.getOpr()->getTokenType());
  std::string mes = "Operation " + u.getOpr()->getValueString() +
    " to types \"" + childType->name + "\" not valid";

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
      u.getExpr()->accept(lvalue);
      if (!childType->isTypePointer()) {
        isPass = false;
        break;
      }
      isPass = true;
      u.typeExpression = std::dynamic_pointer_cast<Pointer>(childType)->typeBase;
      break;
    }
    case tok::TokenType::At: {
      u.getExpr()->accept(lvalue);
      if (!lvalue.isLvalue()) {
        throw SemanticException(u.getOpr(), mesLvalue);
      }
      isPass = true;
      u.typeExpression = std::make_shared<Pointer>(childType);
      if (childType->isProcedureType() && stackTable.isFunction(childType->name)) {
        u.typeExpression = childType;
      }
      break;
    }
    default:
      throw std::logic_error("Not valid UnaryOperation tokenType " + tok::toString(u.getOpr()->getTokenType()));
  }

  if (!isPass) {
    throw SemanticException(u.getOpr(), mes);
  }
}

void CheckType::visit(ArrayAccess& a) {
  CheckTypeArrayAccess c(a);
  a.getName()->accept(*this);
  if (a.getName()->typeExpression == nullptr) {
    throw SemanticException("Cannot [] on function");
  }

  for (auto& e : a.getListIndex()) {
    e->accept(*this);
    if (e->typeExpression == nullptr) {
      throw SemanticException("Function not valid index");
    }
    if (!e->typeExpression->isInt()) {
      throw SemanticException(0, 0, "Except \"Integer\", but find " + e->typeExpression->name);
    }
  }
  auto& childType = a.getName()->typeExpression;
  childType->accept(c);
}


void CheckType::visit(RecordAccess& r) {
  r.getRecord()->accept(*this);
  if (r.getRecord()->typeExpression == nullptr) {
    throw SemanticException("Cannot . on function");
  }
  CheckTypeRecordAccess c(r);
  r.getRecord()->typeExpression->accept(c);
}

void CheckType::visit(FunctionCall& f) {
  CheckTypeFunctionCall checker(f);
  f.getName()->accept(*this);
  for (auto& e: f.getParam()) {
    e->accept(*this);
  }
  if (f.getName()->embeddedFunction != nullptr) {
    f.getName()->embeddedFunction->accept(checker);
  } else {
    f.getName()->typeExpression->accept(checker);
  }
}

void CheckType::visit(StaticCast& s) {
  s.expr->accept(*this);
}

void CheckType::visit(AssignmentStmt& a) {
  CheckLvalue lvalue;
  a.getLeft()->accept(lvalue);
  if (!lvalue.isLvalue()) {
    throw SemanticException(0, 0, "Except lvalue in assigment");
  }
  a.getLeft()->accept(*this);
  a.getRight()->accept(*this);
  // TODO

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
