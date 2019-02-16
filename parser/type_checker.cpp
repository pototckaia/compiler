#include "type_checker.h"

#include "../exception.h"

bool LvalueChecker::is(ptr_Expr& e) {
  LvalueChecker checkLvalue;
  e->accept(checkLvalue);
  return checkLvalue.isLvalue();
}

void LvalueChecker::visit(Literal& l) { lvalue = false; }

void LvalueChecker::visit(Variable&) { lvalue = true; }

void LvalueChecker::visit(FunctionCall& f) { lvalue = false; }

void LvalueChecker::visit(BinaryOperation& f) { lvalue = false; }

void LvalueChecker::visit(ArrayAccess& a) { a.getName()->accept(*this); }

void LvalueChecker::visit(RecordAccess& r) { r.getRecord()->accept(*this); }

void LvalueChecker::visit(Cast& f) {
  f.getExpr()->accept(*this);
  lvalue = f.getType()->isPointer() ||
           (lvalue && f.getType()->equals(f.getExpr()->getType().get()));
}

void LvalueChecker::visit(UnaryOperation& u) {
  lvalue = u.getOpr().is(TokenType::Caret);
}

void BaseTypeChecker::visit(Int&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(Double&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(Char&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(TPointer&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(String&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(Boolean&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(FunctionSignature&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(Record&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(Pointer&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(StaticArray&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(OpenArray&) { throw SemanticException(errorMes); }

void BaseTypeChecker::visit(Alias& a) { a.getRefType()->accept(*this); }

void BaseTypeChecker::visit(ForwardType& a) { a.getRefType()->accept(*this); }

void ArrayAccessChecker::make(ArrayAccess& a, ptr_Type& t) {
  ArrayAccessChecker c(a);
  t->accept(c);
}

void ArrayAccessChecker::visit(Pointer& a) {
  if (sizeBounds == 1) {
    arrayAccess.setType(a.getPointerBase());
    return;
  } else if (sizeBounds > 1) {
    --sizeBounds;
    a.getPointerBase()->accept(*this);
  } else if (sizeBounds < 1) {
    throw std::logic_error("Check Array Access size bounds < 1");
  }
}

void ArrayAccessChecker::visit(StaticArray& s) {
  uint64_t bounds = sizeBounds;
  uint64_t boundsType = s.getBounds().size();
  if (bounds == boundsType) {
    arrayAccess.setType(s.getRefType());
    return;
  } else if (boundsType > bounds) {
    auto copy = std::make_shared<StaticArray>(s);
    for (int i = 0; i < bounds; ++i) {
      copy->getBounds().pop_front();
    }
//    auto iterEnd = copy->bounds.begin();
//    std::advance(iterEnd, boundsType - bounds);
//    copy->bounds.erase(copy->bounds.begin(), iterEnd);
    arrayAccess.setType(std::move(copy));
    return;
  } else if (boundsType < bounds) {
    sizeBounds = bounds - boundsType;
    s.getRefType()->accept(*this);
  }
}

void ArrayAccessChecker::visit(OpenArray& o) {
  if (sizeBounds == 1) {
    arrayAccess.setType(o.getRefType());
    return;
  } else if (sizeBounds > 1) {
    --sizeBounds;
    o.getRefType()->accept(*this);
  } else if (sizeBounds < 1) {
    throw std::logic_error("Check Array Access size bounds < 1");
  }
}

void RecordAccessChecker::make(RecordAccess& r, ptr_Type& t) {
  RecordAccessChecker c(r);
  t->accept(c);
}

void RecordAccessChecker::visit(Record& r) {
  if (!r.getTable().checkContain(recordAccess.getField().getString())) {
    throw NotDefinedException(recordAccess.getField());
  }
  recordAccess.setType(r.getTable().find(recordAccess.getField().getString())->getType());
}

void FunctionCallChecker::make(FunctionCall& f, const ptr_Symbol& s) {
  FunctionCallChecker c(f);
  s->accept(c);
}


void FunctionCallChecker::visit(FunctionSignature& s) {
  if (s.getParamList().size() != f.getParam().size()) {
    throw SemanticException(f.getLine(), f.getColumn(),
                            "Expect number of arguments " + std::to_string(s.getParamList().size()) +
                            " but find " + std::to_string(f.getParam().size()));
  }
  f.setType(s.getReturnType());
  auto iterParameter = s.getParamList().begin();
  ListExpr newParam;
  for (; iterParameter != s.getParamList().end(); ++iterParameter) {
    auto parameter = *iterParameter;
    auto argument = std::move(f.getParam().front());
    f.getParam().pop_front();
    if (parameter->getSpec() == ParamSpec::Var ||
        parameter->getSpec() == ParamSpec::Out) {
      if (!LvalueChecker::is(argument)) {
        throw SemanticException(argument->getLine(), argument->getColumn(), "Expect lvalue in argument");
      }
    }
    if (parameter->getType()->equalsForCheckArgument(argument->getType().get())) {
      newParam.push_back(std::move(argument));
      continue;
    }
    if ((parameter->getType()->isDouble() && argument->getType()->isInt()) ||
        (parameter->getType()->isPurePointer() && argument->getType()->isTypePointer())) {
      auto newArgument = std::make_unique<Cast>(parameter->getType(), std::move(argument));
      newParam.push_back(std::move(newArgument));
      continue;
    }
    throw SemanticException(argument->getLine(), argument->getColumn(),
                            "Expect argument's type \"" + parameter->getType()->getName() +
                            "\" but find \"" + argument->getType()->getName() + "\"");
  }
  f.setParam(newParam);
}

void FunctionCallChecker::visit(Read&) {
  f.setType(std::make_shared<Void>());
  for (auto& e : f.getParam()) {
    auto& type = e->getType();
    if (!LvalueChecker::is(e)) {
      throw SemanticException(e->getLine(), e->getColumn(), "Expect lvalue in argument");
    }
    if (type->isInt() || type->isDouble() || type->isChar()) {
      continue;
    }
    throw SemanticException(e->getLine(), e->getColumn(), "Expect readable type but find " + type->getName());
  }
}

void FunctionCallChecker::visit(Write&) {
  f.setType(std::make_shared<Void>());
  for (auto& e : f.getParam()) {
    auto& type = e->getType();
    if (type->isInt() || type->isDouble() || type->isChar() || type->isString() || type->isPointer()) {
      continue;
    }
    throw SemanticException(e->getLine(), e->getColumn(), "Expect writeable type but find " + type->getName());
  }
}

void FunctionCallChecker::visit(Chr& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Ord& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Prev& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Succ& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Trunc& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Round& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Exit& c) {
  f.setType(std::make_shared<Void>());
  if (c.getReturnType()->isVoid()) {
    if (!f.getParam().empty()) {
      throw SemanticException(f.getLine(), f.getColumn(),
                              "Expect 0 argument but find " + std::to_string(f.getParam().size()));
    }
    return;
  }
  if (f.getParam().size() > 1) {
    throw SemanticException(f.getLine(), f.getColumn(),
                            "Expect 1 argument but find " + std::to_string(f.getParam().size()));
  }
  if (!f.getParam().back()->getType()->equalsForCheckArgument(c.getReturnType().get())) {
    throw SemanticException(f.getLine(), f.getColumn(),
                            "Expect type " + c.getReturnType()->getName() + "but find" +
                            f.getParam().back()->getType()->getName());
  }
}

void FunctionCallChecker::visit(High&) {
  if (f.getParam().empty() || f.getParam().size() > 1) {
    throw SemanticException(f.getLine(), f.getColumn(), "Expect argument but find " + std::to_string(f.getParam().size()));
  }
  auto& type = f.getParam().back()->getType();
  if (type->isOpenArray() || type->isStaticArray()) {
    f.setType(std::make_shared<Int>());
    return;
  }
  throw SemanticException(f.getLine(), f.getColumn(), "Expect array type but find " + type->getName());
}

void FunctionCallChecker::visit(Low&) {
  High h;
  visit(h);
}


TypeChecker::TypeChecker(StackTable& s) : stackTable(s) {}

bool TypeChecker::isImplicitType(ptr_Type& typeLeft, ptr_Type& typeRight) {
  return ((typeRight->isInt() && typeLeft->isDouble()) ||
          (typeRight->isTypePointer() && typeLeft->isPurePointer()));
}

void TypeChecker::visit(Literal& l) {
  if (isMustFunctionCall) {
    throw SemanticException(l.getLine(), l.getColumn(), "Expect function call ");
  }
  switch (l.getValue().getTokenType()) {
    case TokenType::Int: {
      l.setType(std::make_shared<Int>());
      break;
    }
    case TokenType::Double: {
      l.setType(std::make_shared<Double>());
      break;
    }
    case TokenType::Nil: {
      l.setType(std::make_shared<TPointer>());
      break;
    }
    case TokenType::String: {
      if (l.getValue().getString().size() > 1) {
        l.setType(std::make_shared<String>());
      } else {
        l.setType(std::make_shared<Char>());
      }
      break;
    }
    case TokenType::False:
    case TokenType::True: {
      l.setType(std::make_shared<Boolean>());
      break;
    }
    default:
      throw std::logic_error("Error TokenType in Literal");
  }
}

void TypeChecker::visit(Variable& v) {
  if (isMustFunctionCall) {
    throw SemanticException(v.getLine(), v.getColumn(), "Expect function call");
  }

  if (stackTable.isFunction(v.getName().getString())) {
    auto f = stackTable.findFunction(v.getName().getString());
    if (f->isEmbedded()) {
      v.setEmbeddedFunction(stackTable.findFunction(v.getName().getString()));
      v.setType(nullptr);
      return;
    } else if (stackTable.top().tableVariable.checkContain(f->getName()) &&
               !wasFunctionCall) {
      // for variable result function - foo and foo()
      v.setType(f->getSignature()->getReturnType());
      return;
    }
    f->getSignature()->setName(f->getName());
    v.setType(f->getSignature());
    return;
  }

  // TODO const
  if (stackTable.isConst(v.getVarName())) {
    v.setType(stackTable.findConst(v.getVarName())->getType());
    return;
  }

  if (!stackTable.isVar(v.getVarName())) {
    throw NotDefinedException(v.getName());
  }
  v.setType(stackTable.findVar(v.getVarName())->getType());
}

bool TypeChecker::setCast(BinaryOperation& b, bool isAssigment) {
  auto& leftType = b.getLeft()->getType();
  auto& rightType = b.getRight()->getType();

  if (isImplicitType(leftType, rightType)) {
    b.setRight(std::make_unique<Cast>(leftType, std::move(b.getRight())));
    b.setType(leftType);
    return true;
  } else if (!isAssigment && isImplicitType(rightType, leftType)) {
    b.setLeft(std::make_unique<Cast>(rightType, std::move(b.getLeft())));
    b.setType(rightType);
    return true;
  }
  return false;
}

bool TypeChecker::checkTypePlusMinus(BinaryOperation& b, bool isAssigment) {
  auto notValidType = [](ptr_Type t) -> bool {
    return !(t->isInt() || t->isDouble() || t->isPurePointer() || t->isTypePointer());
  };

  if (notValidType(b.getLeft()->getType()) || notValidType(b.getRight()->getType())) {
    return false;
  }
  setCast(b, isAssigment);
  auto& leftType = b.getLeft()->getType();
  auto& rightType = b.getRight()->getType();

  if ((b.getOpr().is(TokenType::Plus) ||
       b.getOpr().is(TokenType::AssignmentWithPlus)) &&
      leftType->isPointer() && rightType->isPointer()) {
    return false;
  } else if ((b.getOpr().is(TokenType::Minus) ||
              b.getOpr().is(TokenType::AssignmentWithMinus)) &&
             leftType->isPointer() && rightType->isPointer()) {
    b.setType(std::make_shared<Int>());
    return true;
  }

  if (leftType->equals(rightType.get())) {
    b.setType(leftType);
    return true;
  }

  auto m = [](const ptr_Expr& r, uint64_t s) -> ptr_Expr {
    auto c = std::make_unique<BinaryOperation>(
      Token(-1, -1, TokenType::Asterisk),
      std::move(r),
      std::make_unique<Literal>(Token(-1, -1, s, ""),
                                std::make_shared<Int>()));
    c->setType(std::make_unique<Int>());
    return c;
  };

  if (leftType->isTypePointer() && rightType->isInt()) {
    // маштабирование указателя
    b.setType(leftType);
    b.setRight(m(b.getRight(), leftType->getPointerBase()->size()));
    return true;
  } else if (leftType->isInt() && rightType->isTypePointer() && b.getOpr().is(TokenType::Plus)) {
    b.setType(rightType);
    b.setLeft(m(b.getLeft(), rightType->getPointerBase()->size()));
    return true;
  }

  return false;
}

bool TypeChecker::checkTypeSlashAsterisk(BinaryOperation& b, bool isAssigment) {
  setCast(b, isAssigment);
  auto& leftType = b.getLeft()->getType();
  auto& rightType = b.getRight()->getType();

  bool isPass = (leftType->isDouble() || leftType->isInt()) && leftType->equals(rightType.get());
  b.setType(leftType);
  if (b.getOpr().is(TokenType::Slash) ||
      b.getOpr().is(TokenType::AssignmentWithSlash)) {
    if (leftType->isInt() && rightType->isInt()) {
      isPass = !isAssigment;
      b.setLeft(std::make_unique<Cast>(std::make_shared<Double>(), std::move(b.getLeft())));
      b.setRight(std::make_unique<Cast>(std::make_shared<Double>(), std::move(b.getRight())));
    }
    b.setType(std::make_shared<Double>());
  }
  return isPass;
}

void TypeChecker::visit(BinaryOperation& b) {
  if (isMustFunctionCall) {
    throw SemanticException(b.getLine(), b.getColumn(), "Expect function call but find " + b.getOpr().getString());
  }
  wasFunctionCall = false;

  b.getLeft()->accept(*this);
  b.getRight()->accept(*this);

  auto& leftType = b.getLeft()->getType();
  auto& rightType = b.getRight()->getType();
  if (leftType == nullptr || rightType == nullptr) {
    throw SemanticException(b.getLine(), b.getColumn(), "Cannot " + b.getOpr().getString() + " function");
  }

  std::string mes = "Operation " + b.getOpr().getString() +
                    " to types \"" + leftType->getName() + "\" and \"" + rightType->getName() + "\" not valid";
  bool isPass;

  switch (b.getOpr().getTokenType()) {
    case TokenType::Plus:
    case TokenType::Minus: { // + -
      isPass = checkTypePlusMinus(b);
      break;
    }
    case TokenType::Asterisk:
    case TokenType::Slash: { // *
      isPass = checkTypeSlashAsterisk(b);
      break;
    }
    case TokenType::Div:
    case TokenType::Mod:
    case TokenType::ShiftLeft:
    case TokenType::Shl:
    case TokenType::ShiftRight:
    case TokenType::Shr: {
      isPass = leftType->isInt() && rightType->isInt();
      b.setType(leftType);
      break;
    }
    case TokenType::And:
    case TokenType::Or:
    case TokenType::Xor: {
      isPass = (leftType->isInt() || leftType->isBool()) && leftType->equals(rightType.get());
			b.setType(leftType);
      break;
    }
    case TokenType::Equals:
    case TokenType::NotEquals: {
      if (leftType->isPointer() && rightType->isPointer()) {
        isPass = setCast(b, false) || leftType->equals(rightType.get());
				b.setType(std::make_shared<Boolean>());
        break;
      }
    }
    case TokenType::StrictLess:
    case TokenType::StrictGreater:
    case TokenType::LessOrEquals:
    case TokenType::GreaterOrEquals: {
      isPass = ((leftType->isInt() || leftType->isDouble() || leftType->isChar()) &&
                leftType->equals(rightType.get())) ||
               setCast(b, false);
      b.setType(std::make_shared<Boolean>());
      break;
    }
    default: {
      throw std::logic_error("Not valid BinaryOperation tokenType " + toString(b.getOpr().getTokenType()));
    }
  }

  if (!isPass) {
    throw SemanticException(b.getLine(), b.getColumn(), mes);
  }
}

void TypeChecker::visit(UnaryOperation& u) {
  if (isMustFunctionCall) {
    throw SemanticException(u.getLine(), u.getColumn(), "Expect function call but find " + u.getOpr().getString());
  }
  wasFunctionCall = false;

  u.getExpr()->accept(*this);
  auto& childType = u.getExpr()->getType();
  if (childType == nullptr) {
    throw SemanticException(u.getLine(), u.getColumn(),
                            "Cannot " + u.getOpr().getString() + " embedded function");
  }

  bool isPass = false;
  std::string mesLvalue = "Expect lvalue in operator " + toString(u.getOpr().getTokenType());
  std::string mes = "Operation " + u.getOpr().getString() +
                    " to types \"" + childType->getName() + "\" not valid";

  switch (u.getOpr().getTokenType()) {
    case TokenType::Plus:
    case TokenType::Minus: {
      isPass = childType->isInt() || childType->isDouble();
      u.setType(childType);
      break;
    }
    case TokenType::Not: {
      isPass = childType->isInt() || childType->isBool();
      u.setType(childType);
      break;
    }
    case TokenType::Caret: {
      if (!childType->isTypePointer()) {
        isPass = false;
        break;
      }
      isPass = true;
      u.setType(childType->getPointerBase());
      break;
    }
    case TokenType::At: {
      if (!LvalueChecker::is(u.getExpr())) {
        throw SemanticException(u.getLine(), u.getColumn(), mesLvalue);
      }
      isPass = true;
      u.setType(std::make_shared<Pointer>(childType));
      if (childType->isProcedureType() && stackTable.isFunction(childType->getName())) {
        u.setType(childType);
      }
      break;
    }
    default:
      throw std::logic_error("Not valid UnaryOperation tokenType " +
                             toString(u.getOpr().getTokenType()));
  }

  if (!isPass) {
    throw SemanticException(u.getLine(), u.getColumn(), mes);
  }
}

void TypeChecker::visit(ArrayAccess& a) {
  if (isMustFunctionCall) {
    throw SemanticException(a.getLine(), a.getColumn(), "Expect function call but find []");
  }
  wasFunctionCall = false;

  if (!LvalueChecker::is(a.getName())) {
    throw SemanticException(a.getName()->getLine(), a.getName()->getColumn(), "Expect lvalue in []");
  }
  a.getName()->accept(*this);
  if (a.getName()->getType() == nullptr) {
    throw SemanticException(a.getName()->getLine(), a.getName()->getColumn(), "Cannot [] on function");
  }

  for (auto& e : a.getListIndex()) {
    e->accept(*this);
    if (e->getType() == nullptr) {
      throw SemanticException(e->getLine(), e->getColumn(), "Function not valid index");
    }
    if (!e->getType()->isInt()) {
      throw SemanticException(e->getLine(), e->getColumn(), "Expect \"Integer\", but find " + e->getType()->getName());
    }
  }
  auto& childType = a.getName()->getType();
  ArrayAccessChecker::make(a, childType);
}

void TypeChecker::visit(RecordAccess& r) {
  if (isMustFunctionCall) {
    throw SemanticException(r.getLine(), r.getColumn(), "Expect function call but find .");
  }
  wasFunctionCall = false;

  if (!LvalueChecker::is(r.getRecord())) {
    throw SemanticException(r.getRecord()->getLine(), r.getRecord()->getColumn(), "Expect lvalue in .");
  }
  r.getRecord()->accept(*this);
  if (r.getRecord()->getType() == nullptr) {
    throw SemanticException(r.getRecord()->getLine(), r.getRecord()->getColumn(), "Cannot . on function");
  }
  RecordAccessChecker::make(r, r.getRecord()->getType());
}

void TypeChecker::visit(FunctionCall& f) {
  if (isMustFunctionCall) {
    isMustFunctionCall = false;
  }
  if (!LvalueChecker::is(f.getName())) {
    throw SemanticException(f.getName()->getLine(), f.getName()->getColumn(), "Expect lvalue in ()");
  }
  wasFunctionCall = true;
  f.getName()->accept(*this);
  wasFunctionCall = false;
  for (auto& e: f.getParam()) {
    e->accept(*this);
  }
  if (f.getName()->getEmbeddedFunction() != nullptr) {
    FunctionCallChecker::make(f, f.getName()->getEmbeddedFunction());
  } else {
    FunctionCallChecker::make(f, f.getName()->getType());
  }
}

void TypeChecker::visit(Cast& s) {
  if (isMustFunctionCall) {
    throw SemanticException("Expect function call but find cast");
  }
  wasFunctionCall = false;
  s.getExpr()->accept(*this);
  auto& to = s.getType();
  auto& from = s.getExpr()->getType();
  auto isPass = [](ptr_Type& to, ptr_Type& from) {
    return (to->isInt() && (from->isInt() || from->isDouble() || from->isBool())) ||
           (to->isDouble() && (from->isInt() || from->isDouble())) ||
           (to->isPurePointer() && (from->isPointer() || from->isPurePointer())) ||
           (to->isTypePointer() && (from->isPurePointer() || from->equals(to.get())));
  };

  if (isPass(to, from)) {
    return;
  }

  throw SemanticException("Cannot cast to type \"" + to->getName() +
                          "\" expression with type \"" + from->getName() + "\"");
}

void TypeChecker::visit(AssignmentStmt& a) {
  if (isMustFunctionCall) {
    throw SemanticException(a.getLine(), a.getColumn(), "Expect function call but find assigment");
  }
  a.getLeft()->accept(*this);
  a.getRight()->accept(*this);
  if (!LvalueChecker::is(a.getLeft())) {
    throw SemanticException(a.getLeft()->getLine(), a.getLeft()->getColumn(), "Expect lvalue in assigment");
  }
  std::string mes = "Operation " + a.getOpr().getString() +
                    " to types \"" + a.getLeft()->getType()->getName() + "\" and \"" +
                    a.getRight()->getType()->getName() + "\" not valid";
  auto typeRight = a.getRight()->getType();
  auto typeLeft = a.getLeft()->getType();
  switch (a.getOpr().getTokenType()) {
    case TokenType::AssignmentWithMinus:
    case TokenType::AssignmentWithPlus: {
      if (!checkTypePlusMinus(a, true)) {
        throw SemanticException(a.getLine(), a.getColumn(), mes);
      }
      typeRight = a.getType();
      break;
    }
    case TokenType::AssignmentWithSlash:
    case TokenType::AssignmentWithAsterisk: {
      if (!checkTypeSlashAsterisk(a, true)) {
        throw SemanticException(a.getLine(), a.getColumn(), mes);
      }
      typeRight = a.getType();
      break;
    }
    case TokenType::Assignment: {
      break;
    }
    default: {
      throw std::logic_error("Not valid token type in assignment");
    }
  }
  if (isImplicitType(typeLeft, typeRight)) {
    a.setRight(std::make_unique<Cast>(typeLeft, std::move(a.getRight())));
    return;
  }
  if (!typeRight->equals(typeLeft.get())) {
    throw SemanticException(a.getLine(), a.getColumn(), mes);
  }
}

void TypeChecker::visit(FunctionCallStmt& f) {
  isMustFunctionCall = true;
  f.getFunctionCall()->accept(*this);
  isMustFunctionCall = false;
}

void TypeChecker::visit(BlockStmt& b) {
  for (auto& e: b.getBlock()) {
    e->accept(*this);
  }
}

void TypeChecker::visit(IfStmt& i) {
  i.getCondition()->accept(*this);
  i.getThen()->accept(*this);
  if (i.getElse() != nullptr) {
    i.getElse()->accept(*this);
  }
  if (i.getCondition()->getType()->isInt()) {
    i.setCondition(std::make_unique<Cast>(std::make_shared<Boolean>(), std::move(i.getCondition())));
    return;
  }
  if (!i.getCondition()->getType()->isBool()) {
    throw SemanticException(i.getCondition()->getLine(), i.getCondition()->getColumn(),
                            "Expect type bool, but find " + i.getCondition()->getType()->getName());
  }
}

void TypeChecker::visit(WhileStmt& w) {
  w.getCondition()->accept(*this);
  w.getBlock()->accept(*this);
  if (w.getCondition()->getType()->isInt()) {
    w.setCondition(std::make_unique<Cast>(std::make_shared<Boolean>(), std::move(w.getCondition())));
    return;
  }
  if (!w.getCondition()->getType()->isBool()) {
    throw SemanticException(w.getCondition()->getLine(), w.getCondition()->getColumn(),
                            "Expect type bool, but find " + w.getCondition()->getType()->getName());
  }
}

void TypeChecker::visit(ForStmt& f) {
  f.getVar()->accept(*this);
  f.getLow()->accept(*this);
  f.getHigh()->accept(*this);
  f.getBlock()->accept(*this);
  if (!(f.getVar()->getType()->isInt() && f.getLow()->getType()->isInt() &&
        f.getHigh()->getType()->isInt())) {
    throw SemanticException(f.getVar()->getLine(), f.getVar()->getColumn(), "Loop variable must be type int");
  }
}
