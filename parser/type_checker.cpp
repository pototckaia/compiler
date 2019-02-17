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
  f.expr->accept(*this);
  lvalue = f.type->isPointer() ||
           (lvalue && f.type->equals(f.expr->type.get()));
}

void LvalueChecker::visit(UnaryOperation& u) {
  lvalue = u.getOpr().getTokenType() == TokenType::Caret;
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

void BaseTypeChecker::visit(Alias& a) { a.type->accept(*this); }

void BaseTypeChecker::visit(ForwardType& a) { a.type->accept(*this); }

void ArrayAccessChecker::make(ArrayAccess& a, ptr_Type& t) {
  ArrayAccessChecker c(a);
  t->accept(c);
}

void ArrayAccessChecker::visit(Pointer& a) {
  if (sizeBounds == 1) {
    arrayAccess.type = a.typeBase;
    return;
  } else if (sizeBounds > 1) {
    --sizeBounds;
    a.typeBase->accept(*this);
  } else if (sizeBounds < 1) {
    throw std::logic_error("Check Array Access size bounds < 1");
  }
}

void ArrayAccessChecker::visit(StaticArray& s) {
  uint64_t bounds = sizeBounds;
  uint64_t boundsType = s.bounds.size();
  if (bounds == boundsType) {
    arrayAccess.type = s.typeElem;
    return;
  } else if (boundsType > bounds) {
    auto copy = std::make_shared<StaticArray>(s);
    for (int i = 0; i < bounds; ++i) {
      copy->bounds.pop_front();
    }
//    auto iterEnd = copy->bounds.begin();
//    std::advance(iterEnd, boundsType - bounds);
//    copy->bounds.erase(copy->bounds.begin(), iterEnd);
    arrayAccess.type = std::move(copy);
    return;
  } else if (boundsType < bounds) {
    sizeBounds = bounds - boundsType;
    s.typeElem->accept(*this);
  }
}

void ArrayAccessChecker::visit(OpenArray& o) {
  if (sizeBounds == 1) {
    arrayAccess.type = o.typeElem;
    return;
  } else if (sizeBounds > 1) {
    --sizeBounds;
    o.typeElem->accept(*this);
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
  recordAccess.type = r.getTable().find(recordAccess.getField().getString())->type;
}

void FunctionCallChecker::make(FunctionCall& f, const ptr_Symbol& s) {
  FunctionCallChecker c(f);
  s->accept(c);
}


void FunctionCallChecker::visit(FunctionSignature& s) {
  if (s.paramsList.size() != f.getParam().size()) {
    throw SemanticException(f.getDeclPoint(),
                            "Expect number of arguments " + std::to_string(s.paramsList.size()) +
                            " but find " + std::to_string(f.getParam().size()));
  }
  f.type = s.returnType;
  auto iterParameter = s.paramsList.begin();
  ListExpr newParam;
  for (; iterParameter != s.paramsList.end(); ++iterParameter) {
    auto parameter = *iterParameter;
    auto argument = std::move(f.listParam.front());
    f.listParam.pop_front();
    if (parameter->spec == ParamSpec::Var ||
        parameter->spec == ParamSpec::Out) {
      if (!LvalueChecker::is(argument)) {
        throw SemanticException(argument->getDeclPoint(), "Expect lvalue in argument");
      }
    }
    if (parameter->type->equalsForCheckArgument(argument->type.get())) {
      newParam.push_back(std::move(argument));
      continue;
    }
    if ((parameter->type->isDouble() && argument->type->isInt()) ||
        (parameter->type->isPurePointer() && argument->type->isTypePointer())) {
      auto newArgument = std::make_unique<Cast>(parameter->type, std::move(argument));
      newParam.push_back(std::move(newArgument));
      continue;
    }
    throw SemanticException(argument->getDeclPoint(),
                            "Expect argument's type \"" + parameter->type->getSymbolName() +
                            "\" but find \"" + argument->type->getSymbolName() + "\"");
  }
  f.listParam = std::move(newParam);
}

void FunctionCallChecker::visit(Read&) {
  f.type = std::make_shared<Void>();
  for (auto& e : f.listParam) {
    auto& type = e->type;
    if (!LvalueChecker::is(e)) {
      throw SemanticException(e->getDeclPoint(), "Expect lvalue in argument");
    }
    if (type->isInt() || type->isDouble() || type->isChar()) {
      continue;
    }
    throw SemanticException(e->getDeclPoint(), "Expect readable type but find " + type->getSymbolName());
  }
}

void FunctionCallChecker::visit(Write&) {
  f.type = std::make_shared<Void>();
  for (auto& e : f.getParam()) {
    auto& type = e->type;
    if (type->isInt() || type->isDouble() || type->isChar() || type->isString() || type->isPointer()) {
      continue;
    }
    throw SemanticException(e->getDeclPoint(), "Expect writeable type but find " + type->getSymbolName());
  }
}

void FunctionCallChecker::visit(Chr& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Ord& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Prev& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Succ& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Trunc& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Round& c) { c.getSignature()->accept(*this); }

void FunctionCallChecker::visit(Exit& c) {
  f.type = std::make_shared<Void>();
  if (c.returnType->isVoid()) {
    if (!f.getParam().empty()) {
      throw SemanticException(f.getDeclPoint(),
                              "Expect 0 argument but find " + std::to_string(f.getParam().size()));
    }
    return;
  }
  if (f.getParam().size() > 1) {
    throw SemanticException(f.getDeclPoint(),
                            "Expect 1 argument but find " + std::to_string(f.getParam().size()));
  }
  if (!f.getParam().back()->type->equalsForCheckArgument(c.returnType.get())) {
    throw SemanticException(f.getDeclPoint(),
                            "Expect type " + c.returnType->getSymbolName() + "but find" +
                            f.getParam().back()->type->getSymbolName());
  }
}

void FunctionCallChecker::visit(High&) {
  if (f.getParam().empty() || f.getParam().size() > 1) {
    throw SemanticException(f.getDeclPoint(), "Expect argument but find " + std::to_string(f.getParam().size()));
  }
  auto& type = f.getParam().back()->type;
  if (type->isOpenArray() || type->isStaticArray()) {
    f.type = std::make_shared<Int>();
    return;
  }
  throw SemanticException(f.getDeclPoint(), "Expect array type but find " + type->getSymbolName());
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
    throw SemanticException(l.getDeclPoint(), "Expect function call ");
  }
  switch (l.getValue().getTokenType()) {
    case TokenType::Int: {
      l.type = std::make_shared<Int>();
      break;
    }
    case TokenType::Double: {
      l.type = std::make_shared<Double>();
      break;
    }
    case TokenType::Nil: {
      l.type = std::make_shared<TPointer>();
      break;
    }
    case TokenType::String: {
      if (l.getValue().getString().size() > 1) {
        l.type = std::make_shared<String>();
      } else {
        l.type = std::make_shared<Char>();
      }
      break;
    }
    case TokenType::False:
    case TokenType::True: {
      l.type = std::make_shared<Boolean>();
      break;
    }
    default:
      throw std::logic_error("Error TokenType in Literal");
  }
}

void TypeChecker::visit(Variable& v) {
  if (isMustFunctionCall) {
    throw SemanticException(v.getDeclPoint(), "Expect function call");
  }

  if (stackTable.isFunction(v.getName().getString())) {
    auto f = stackTable.findFunction(v.getName().getString());
    if (f->isEmbedded()) {
      v.embeddedFunction = stackTable.findFunction(v.getName().getString());
      v.type = nullptr;
      return;
    } else if (stackTable.top().tableVariable.checkContain(f->getSymbolName()) &&
               !wasFunctionCall) {
      // for variable result function - foo and foo()
      v.type = f->getSignature()->returnType;
      return;
    }
    f->getSignature()->setSymbolName(f->getSymbolName());
    v.type = f->getSignature();
    return;
  }

  // TODO const
  if (stackTable.isConst(v.getName().getString())) {
    v.type = stackTable.findConst(v.getName().getString())->type;
    return;
  }

  if (!stackTable.isVar(v.getName().getString())) {
    throw NotDefinedException(v.getName());
  }
  v.type = stackTable.findVar(v.getName().getString())->type;
}

bool TypeChecker::setCast(BinaryOperation& b, bool isAssigment) {
  auto& leftType = b.getLeft()->type;
  auto& rightType = b.getRight()->type;

  if (isImplicitType(leftType, rightType)) {
    b.right = std::make_unique<Cast>(leftType, std::move(b.right));
    b.type = leftType;
    return true;
  } else if (!isAssigment && isImplicitType(rightType, leftType)) {
    b.left = std::make_unique<Cast>(rightType, std::move(b.left));
    b.type = rightType;
    return true;
  }
  return false;
}

bool TypeChecker::checkTypePlusMinus(BinaryOperation& b, bool isAssigment) {
  auto notValidType = [](ptr_Type t) -> bool {
    return !(t->isInt() || t->isDouble() || t->isPurePointer() || t->isTypePointer());
  };

  if (notValidType(b.getLeft()->type) || notValidType(b.getRight()->type)) {
    return false;
  }
  setCast(b, isAssigment);
  auto& leftType = b.getLeft()->type;
  auto& rightType = b.getRight()->type;

  if ((b.getOpr().is(TokenType::Plus) ||
       b.getOpr().is(TokenType::AssignmentWithPlus)) &&
      leftType->isPointer() && rightType->isPointer()) {
    return false;
  } else if ((b.getOpr().is(TokenType::Minus) ||
              b.getOpr().is(TokenType::AssignmentWithMinus)) &&
             leftType->isPointer() && rightType->isPointer()) {
    b.type = std::make_shared<Int>();
    return true;
  }

  if (leftType->equals(rightType.get())) {
    b.type = leftType;
    return true;
  }

  auto m = [](ptr_Expr& r, uint64_t s) -> ptr_Expr {
    auto c = std::make_unique<BinaryOperation>(
      Token(-1, -1, TokenType::Asterisk),
      std::move(r),
      std::make_unique<Literal>(
          Token(-1, -1, s, ""),
          std::make_shared<Int>())
    );
    c->type = std::make_unique<Int>();
    return c;
  };

  if (leftType->isTypePointer() && rightType->isInt()) {
    // маштабирование указателя
    b.type = leftType;
    b.right = m(b.right, leftType->getPointerBase()->size());
    return true;
  } else if (leftType->isInt() && rightType->isTypePointer() && b.getOpr().is(TokenType::Plus)) {
    b.type = rightType;
    b.left = m(b.left, rightType->getPointerBase()->size());
    return true;
  }

  return false;
}

bool TypeChecker::checkTypeSlashAsterisk(BinaryOperation& b, bool isAssigment) {
  setCast(b, isAssigment);
  auto& leftType = b.getLeft()->type;
  auto& rightType = b.getRight()->type;

  bool isPass = (leftType->isDouble() || leftType->isInt()) && leftType->equals(rightType.get());
  b.type = leftType;
  if (b.getOpr().is(TokenType::Slash) ||
      b.getOpr().is(TokenType::AssignmentWithSlash)) {
    if (leftType->isInt() && rightType->isInt()) {
      isPass = !isAssigment;
      b.left = std::make_unique<Cast>(std::make_shared<Double>(), std::move(b.left));
      b.right = std::make_unique<Cast>(std::make_shared<Double>(), std::move(b.right));
    }
    b.type = std::make_shared<Double>();
  }
  return isPass;
}

void TypeChecker::visit(BinaryOperation& b) {
  if (isMustFunctionCall) {
    throw SemanticException(b.getDeclPoint(), "Expect function call but find " + b.getOpr().getString());
  }
  wasFunctionCall = false;

  b.getLeft()->accept(*this);
  b.getRight()->accept(*this);

  auto& leftType = b.getLeft()->type;
  auto& rightType = b.getRight()->type;
  if (leftType == nullptr || rightType == nullptr) {
    throw SemanticException(b.getDeclPoint(), "Cannot " + b.getOpr().getString() + " function");
  }

  std::string mes = "Operation " + b.getOpr().getString() +
                    " to types \"" + leftType->getSymbolName() + "\" and \"" + rightType->getSymbolName() + "\" not valid";
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
      b.type = leftType;
      break;
    }
    case TokenType::And:
    case TokenType::Or:
    case TokenType::Xor: {
      isPass = (leftType->isInt() || leftType->isBool()) && leftType->equals(rightType.get());
      b.type = leftType;
      break;
    }
    case TokenType::Equals:
    case TokenType::NotEquals: {
      if (leftType->isPointer() && rightType->isPointer()) {
        isPass = setCast(b, false) || leftType->equals(rightType.get());
        b.type = std::make_shared<Boolean>();
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
      b.type = std::make_shared<Boolean>();
      break;
    }
    default: {
      throw std::logic_error("Not valid BinaryOperation tokenType " + toString(b.getOpr().getTokenType()));
    }
  }

  if (!isPass) {
    throw SemanticException(b.getDeclPoint(), mes);
  }
}

void TypeChecker::visit(UnaryOperation& u) {
  if (isMustFunctionCall) {
    throw SemanticException(u.getDeclPoint(), "Expect function call but find " + u.getOpr().getString());
  }
  wasFunctionCall = false;

  u.getExpr()->accept(*this);
  auto& childType = u.getExpr()->type;
  if (childType == nullptr) {
    throw SemanticException(u.getDeclPoint(),
                            "Cannot " + u.getOpr().getString() + " embedded function");
  }

  bool isPass = false;
  std::string mesLvalue = "Expect lvalue in operator " + toString(u.getOpr().getTokenType());
  std::string mes = "Operation " + u.getOpr().getString() +
                    " to types \"" + childType->getSymbolName() + "\" not valid";

  switch (u.getOpr().getTokenType()) {
    case TokenType::Plus:
    case TokenType::Minus: {
      isPass = childType->isInt() || childType->isDouble();
      u.type = childType;
      break;
    }
    case TokenType::Not: {
      isPass = childType->isInt() || childType->isBool();
      u.type = childType;
      break;
    }
    case TokenType::Caret: {
      if (!childType->isTypePointer()) {
        isPass = false;
        break;
      }
      isPass = true;
      u.type = childType->getPointerBase();
      break;
    }
    case TokenType::At: {
      if (!LvalueChecker::is(u.expr)) {
        throw SemanticException(u.getDeclPoint(), mesLvalue);
      }
      isPass = true;
      u.type = std::make_shared<Pointer>(childType);
      if (childType->isProcedureType() && stackTable.isFunction(childType->getSymbolName())) {
        u.type = childType;
      }
      break;
    }
    default:
      throw std::logic_error("Not valid UnaryOperation tokenType " +
                             toString(u.getOpr().getTokenType()));
  }

  if (!isPass) {
    throw SemanticException(u.getDeclPoint(), mes);
  }
}

void TypeChecker::visit(ArrayAccess& a) {
  if (isMustFunctionCall) {
    throw SemanticException(a.getDeclPoint(), "Expect function call but find []");
  }
  wasFunctionCall = false;

  if (!LvalueChecker::is(a.nameArray)) {
    throw SemanticException(a.nameArray->getDeclPoint(), "Expect lvalue in []");
  }
  a.getName()->accept(*this);
  if (a.getName()->type == nullptr) {
    throw SemanticException(a.getName()->getDeclPoint(), "Cannot [] on function");
  }

  for (auto& e : a.getListIndex()) {
    e->accept(*this);
    if (e->type == nullptr) {
      throw SemanticException(e->getDeclPoint(), "Function not valid index");
    }
    if (!e->type->isInt()) {
      throw SemanticException(e->getDeclPoint(), "Expect \"Integer\", but find " + e->type->getSymbolName());
    }
  }
  auto& childType = a.getName()->type;
  ArrayAccessChecker::make(a, childType);
}

void TypeChecker::visit(RecordAccess& r) {
  if (isMustFunctionCall) {
    throw SemanticException(r.getDeclPoint(), "Expect function call but find .");
  }
  wasFunctionCall = false;

  if (!LvalueChecker::is(r.record)) {
    throw SemanticException(r.record->getDeclPoint(), "Expect lvalue in .");
  }
  r.getRecord()->accept(*this);
  if (r.getRecord()->type == nullptr) {
    throw SemanticException(r.getRecord()->getDeclPoint(), "Cannot . on function");
  }
  RecordAccessChecker::make(r, r.getRecord()->type);
}

void TypeChecker::visit(FunctionCall& f) {
  if (isMustFunctionCall) {
    isMustFunctionCall = false;
  }
  if (!LvalueChecker::is(f.nameFunction)) {
    throw SemanticException(f.nameFunction->getDeclPoint(), "Expect lvalue in ()");
  }
  wasFunctionCall = true;
  f.getName()->accept(*this);
  wasFunctionCall = false;
  for (auto& e: f.getParam()) {
    e->accept(*this);
  }
  if (f.getName()->embeddedFunction != nullptr) {
    FunctionCallChecker::make(f, f.nameFunction->embeddedFunction);
  } else {
    FunctionCallChecker::make(f, f.nameFunction->type);
  }
}

void TypeChecker::visit(Cast& s) {
  if (isMustFunctionCall) {
    throw SemanticException("Expect function call but find cast");
  }
  wasFunctionCall = false;
  s.expr->accept(*this);
  auto& to = s.type;
  auto& from = s.expr->type;
  auto isPass = [](ptr_Type& to, ptr_Type& from) {
    return (to->isInt() && (from->isInt() || from->isDouble() || from->isBool())) ||
           (to->isDouble() && (from->isInt() || from->isDouble())) ||
           (to->isPurePointer() && (from->isPointer() || from->isPurePointer())) ||
           (to->isTypePointer() && (from->isPurePointer() || from->equals(to.get())));
  };

  if (isPass(to, from)) {
    return;
  }

  throw SemanticException("Cannot cast to type \"" + to->getSymbolName() +
                          "\" expression with type \"" + from->getSymbolName() + "\"");
}

void TypeChecker::visit(AssignmentStmt& a) {
  if (isMustFunctionCall) {
    throw SemanticException(a.getDeclPoint(), "Expect function call but find assigment");
  }
  a.getLeft()->accept(*this);
  a.getRight()->accept(*this);
  if (!LvalueChecker::is(a.left)) {
    throw SemanticException(a.left->getDeclPoint(), "Expect lvalue in assigment");
  }
  std::string mes = "Operation " + a.getOpr().getString() +
                    " to types \"" + a.getLeft()->type->getSymbolName() + "\" and \"" +
                    a.getRight()->type->getSymbolName() + "\" not valid";
  auto typeRight = a.getRight()->type;
  auto typeLeft = a.getLeft()->type;
  switch (a.getOpr().getTokenType()) {
    case TokenType::AssignmentWithMinus:
    case TokenType::AssignmentWithPlus: {
      if (!checkTypePlusMinus(a, true)) {
        throw SemanticException(a.getDeclPoint(), mes);
      }
      typeRight = a.type;
      break;
    }
    case TokenType::AssignmentWithSlash:
    case TokenType::AssignmentWithAsterisk: {
      if (!checkTypeSlashAsterisk(a, true)) {
        throw SemanticException(a.getDeclPoint(), mes);
      }
      typeRight = a.type;
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
    a.right = std::make_unique<Cast>(typeLeft, std::move(a.right));
    return;
  }
  if (!typeRight->equals(typeLeft.get())) {
    throw SemanticException(a.getDeclPoint(), mes);
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
  if (i.getCondition()->type->isInt()) {
    i.condition = std::make_unique<Cast>(std::make_shared<Boolean>(), std::move(i.condition));
    return;
  }
  if (!i.getCondition()->type->isBool()) {
    throw SemanticException(i.getCondition()->getDeclPoint(),
                            "Expect type bool, but find " + i.getCondition()->type->getSymbolName());
  }
}

void TypeChecker::visit(WhileStmt& w) {
  w.getCondition()->accept(*this);
  w.getBlock()->accept(*this);
  if (w.getCondition()->type->isInt()) {
    w.condition = std::make_unique<Cast>(std::make_shared<Boolean>(), std::move(w.condition));
    return;
  }
  if (!w.getCondition()->type->isBool()) {
    throw SemanticException(w.getCondition()->getDeclPoint(),
                            "Expect type bool, but find " + w.getCondition()->type->getSymbolName());
  }
}

void TypeChecker::visit(ForStmt& f) {
  f.getVar()->accept(*this);
  f.getLow()->accept(*this);
  f.getHigh()->accept(*this);
  f.getBlock()->accept(*this);
  if (!(f.getVar()->type->isInt() && f.getLow()->type->isInt() &&
        f.getHigh()->type->isInt())) {
    throw SemanticException(f.getVar()->getDeclPoint(), "Loop variable must be type int");
  }
}
