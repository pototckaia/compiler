#include "visitor_type.h"

#include "../exception.h"

bool LvalueChecker::is(ptr_Expr& e) {
  LvalueChecker checkLvalue;
  e->accept(checkLvalue);
  return checkLvalue.isLvalue();
}
void LvalueChecker::visit(Literal& l) { lvalue = false; }
void LvalueChecker::visit(Variable&) { lvalue = true; }
void LvalueChecker::visit(FunctionCall& f) { lvalue = false; }
void LvalueChecker::visit(BinaryOperation& f) { lvalue = false;}
void LvalueChecker::visit(ArrayAccess& a) { a.getName()->accept(*this); }
void LvalueChecker::visit(RecordAccess& r) { r.getRecord()->accept(*this); }
void LvalueChecker::visit(Cast& f) {
  lvalue = f.type->isPointer() || f.type->equals(f.expr->type.get());
}
void LvalueChecker::visit(UnaryOperation& u) {
  lvalue = u.getOpr()->getTokenType() == tok::TokenType::Caret;
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
  int bounds = sizeBounds;
  int boundsType = s.bounds.size();
  if (bounds == boundsType) {
    arrayAccess.type = s.typeElem;
    return;
  } else if (boundsType > bounds) {
    auto copy = std::make_shared<StaticArray>(s);
    auto iterEnd = copy->bounds.begin();
    std::advance(iterEnd, boundsType - bounds);
    copy->bounds.erase(copy->bounds.begin(), iterEnd);
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
  if (!r.fields.checkContain(recordAccess.getField()->getValueString())) {
    throw NotDefinedException(recordAccess.getField());
  }
  recordAccess.type = r.fields.find(recordAccess.getField()->getValueString())->type;
}

void FunctionCallChecker::make(FunctionCall& f, const ptr_Symbol& s) {
  FunctionCallChecker c(f);
  s->accept(c);
}


void FunctionCallChecker::visit(FunctionSignature& s) {
  if (s.paramsList.size() != f.getParam().size()) {
    throw SemanticException(f.line, f.column,
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
        throw SemanticException(argument->line, argument->column, "Expect lvalue in argument");
      }
    }
    if (parameter->type->equalsForCheckArgument(argument->type.get())) {
      newParam.push_back(std::move(argument));
      continue;
    }
    if ((parameter->type->isDouble() && argument->type->isInt()) ||
       (parameter->type->isPurePointer() && argument->type->isTypePointer()) ){
      auto newArgument = std::make_unique<Cast>(parameter->type, std::move(argument));
      newParam.push_back(std::move(newArgument));
      continue;
    }
    throw SemanticException(argument->line, argument->column,
      "Expect argument's type \"" + parameter->type->name +
      "\" but find \"" + argument->type->name + "\"");
  }
  f.listParam = std::move(newParam);
}

void FunctionCallChecker::visit(Read&) {
  f.type = std::make_shared<Void>();
  for (auto& e : f.listParam) {
    auto& type = e->type;
    if (!LvalueChecker::is(e)) {
      throw SemanticException(e->line, e->column, "Expect lvalue in argument");
    }
    if (type->isInt() || type->isDouble() || type->isChar()) {
      continue;
    }
    throw SemanticException(e->line, e->column, "Expect readable type but find " + type->name);
  }
}

void FunctionCallChecker::visit(Write&) {
  f.type = std::make_shared<Void>();
  for (auto& e : f.getParam()) {
    auto& type = e->type;
    if (type->isInt() || type->isDouble() || type->isChar() || type->isString()) {
      continue;
    }
    throw SemanticException(e->line, e->column, "Expect writeable type but find " + type->name);
  }
}

void FunctionCallChecker::visit(Chr& c) { c.signature->accept(*this); }
void FunctionCallChecker::visit(Ord& c) { c.signature->accept(*this); }
void FunctionCallChecker::visit(Prev& c) { c.signature->accept(*this); }
void FunctionCallChecker::visit(Succ& c) { c.signature->accept(*this); }
void FunctionCallChecker::visit(Trunc& c) { c.signature->accept(*this); }
void FunctionCallChecker::visit(Round& c) { c.signature->accept(*this); }

void FunctionCallChecker::visit(Exit& c) {
  f.type = std::make_shared<Void>();
  if (c.returnType->isVoid()) {
    if (!f.getParam().empty()) {
      throw SemanticException(f.line, f.column,
        "Expect 0 argument but find " + std::to_string(f.getParam().size()));
    }
    return;
  }
  if (f.getParam().size() > 1) {
    throw SemanticException(f.line, f.column,
      "Expect 1 argument but find " + std::to_string(f.getParam().size()));
  }
  if (!f.getParam().back()->type->equalsForCheckArgument(c.returnType.get())) {
    throw SemanticException(f.line, f.column,
      "Expect type " + c.returnType->name + "but find" +
      f.getParam().back()->type->name);
  }
}

void FunctionCallChecker::visit(High&) {
 if (f.getParam().empty() || f.getParam().size() > 1) {
   throw SemanticException(f.line, f.column, "Expect argument but find " + std::to_string(f.getParam().size()));
 }
 auto& type = f.getParam().back()->type;
 if (type->isOpenArray() || type->isStaticArray()) {
   f.type = std::make_shared<Int>();
   return;
 }
  throw SemanticException(f.line, f.column, "Expect array type but find " + type->name);
}

void FunctionCallChecker::visit(Low&) { High h; visit(h); }


TypeChecker::TypeChecker(StackTable s) : stackTable(std::move(s)) {}

bool TypeChecker::isImplicitType(ptr_Type& typeLeft, ptr_Type& typeRight) {
  return ((typeRight->isInt() && typeLeft->isDouble()) ||
          (typeRight->isTypePointer() && typeLeft->isPurePointer()));
}

void TypeChecker::visit(Literal& l) {
  if (isMustFunctionCall) {
    throw SemanticException(l.line, l.column, "Expect function call ");
  }
  switch (l.getValue()->getTokenType()) {
    case tok::TokenType::Int: {
      l.type = std::make_shared<Int>();
      break;
    }
    case tok::TokenType::Double: {
      l.type = std::make_shared<Double>();
      break;
    }
    case tok::TokenType::Nil: {
      l.type = std::make_shared<TPointer>();
      break;
    }
    case tok::TokenType::String: {
      if (l.getValue()->getValueString().size() > 1) {
        l.type = std::make_shared<String>();
      } else {
        l.type = std::make_shared<Char>();
      }
      break;
    }
    case tok::TokenType::False:
    case tok::TokenType::True: {
      l.type = std::make_shared<Boolean>();
      break;
    }
    default:
      throw std::logic_error("Error TokenType in Literal");
  }
}

void TypeChecker::visit(Variable& v) {
  if (isMustFunctionCall) {
    throw SemanticException(v.line, v.column, "Expect function call");
  }

  if (stackTable.isFunction(v.getName()->getValueString())) {
    auto f = stackTable.findFunction(v.getName()->getValueString());
    if (f->isEmbedded()) {
      v.embeddedFunction = stackTable.findFunction(v.getName()->getValueString());
      v.type = f->signature;
      return;
    } else if (stackTable.top().tableVariable.checkContain(f->name) &&
               !wasFunctionCall) {
      // for variable result function - foo and foo()
      v.type = f->signature->returnType;
      return;
    }
    f->signature->name = f->name;
    v.type = f->signature;
    return;
  }

  // TODO const
  if (stackTable.isConst(v.getName()->getValueString())) {
    v.type = stackTable.findConst(v.getName()->getValueString())->type;
    return;
  }

  if (!stackTable.isVar(v.getName()->getValueString())) {
    throw NotDefinedException(v.getName());
  }
  v.type = stackTable.findVar(v.getName()->getValueString())->type;
}

bool TypeChecker::implicitCast(BinaryOperation& b, bool isAssigment) {
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
  implicitCast(b, isAssigment);

  auto& leftType = b.getLeft()->type;
  auto& rightType = b.getRight()->type;
  if (b.getOpr()->getTokenType() == tok::TokenType::Plus ||
      b.getOpr()->getTokenType() == tok::TokenType::AssignmentWithPlus) {
    if (leftType->isPointer() && rightType->isPointer()) {
      return false;
    }

    if (leftType->equals(rightType.get())) {
      b.type = leftType;
      return true;
    }

    if (leftType->isPointer() && rightType->isInt()) {
      b.type = leftType;
      return true;
    } else if (leftType->isInt() && rightType->isPointer()) {
      b.type = rightType;
      return true;
    }

  } else if (b.getOpr()->getTokenType() == tok::TokenType::Minus ||
             b.getOpr()->getTokenType() == tok::TokenType::AssignmentWithMinus) {
    if (leftType->isPointer() && rightType->isPointer()) {
      b.type = std::make_shared<Int>();
      return true;
    }

    if (leftType->equals(rightType.get())) {
      b.type = leftType;
      return true;
    }

    if (leftType->isPointer() && rightType->isInt()) {
      b.type = leftType;
      return true;
    }
  }
  return false;
}

bool TypeChecker::checkTypeSlashAsterisk(BinaryOperation& b, bool isAssigment) {
  if (implicitCast(b, isAssigment)) {
    return true;
  }
  auto& leftType = b.getLeft()->type;
  auto& rightType = b.getRight()->type;

  bool isPass = (leftType->isDouble() || leftType->isInt()) && leftType->equals(rightType.get());
  b.type = leftType;
  if (b.getOpr()->getTokenType() == tok::TokenType::Slash) {
    b.type = std::make_shared<Double>();
  }
  return isPass;
}

void TypeChecker::visit(BinaryOperation& b) {
  if (isMustFunctionCall) {
    throw SemanticException(b.line, b.column, "Expect function call but find " + b.getOpr()->getValueString());
  }
  wasFunctionCall = false;

  b.getLeft()->accept(*this);
  b.getRight()->accept(*this);

  auto& leftType = b.getLeft()->type;
  auto& rightType = b.getRight()->type;
  if (leftType == nullptr || rightType == nullptr) {
    throw SemanticException(b.line, b.column, "Cannot " + b.getOpr()->getValueString() + " function");
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
      isPass = checkTypeSlashAsterisk(b);
      break;
    }
    case tok::TokenType::Div:
    case tok::TokenType::Mod:
    case tok::TokenType::ShiftLeft:
    case tok::TokenType::Shl:
    case tok::TokenType::ShiftRight:
    case tok::TokenType::Shr: {
      isPass = leftType->isInt() && rightType->isInt();
      b.type = leftType;
      break;
    }
    case tok::TokenType::And:
    case tok::TokenType::Or:
    case tok::TokenType::Xor: {
      isPass = (leftType->isInt() || leftType->isBool()) && leftType->equals(rightType.get());
      b.type = leftType;
      break;
    }
    case tok::TokenType::Equals:
    case tok::TokenType::NotEquals: {
      if (leftType->isString() || rightType->isString()) {
        isPass = false;
        b.type = std::make_shared<Boolean>();
        break;
      }
      if (leftType->equals(rightType.get())) {
        isPass = true;
        b.type = std::make_shared<Boolean>();
        break;
      }
      isPass = implicitCast(b, false);
      b.type = std::make_shared<Boolean>();
      break;
    }
    case tok::TokenType::StrictLess:
    case tok::TokenType::StrictGreater:
    case tok::TokenType::LessOrEquals:
    case tok::TokenType::GreaterOrEquals: {
      isPass = ((leftType->isInt() || leftType->isDouble() || leftType->isChar()) &&
                 leftType->equals(rightType.get())) ||
                implicitCast(b, false);
      b.type = std::make_shared<Boolean>();
      break;
    }
    default: {
      throw std::logic_error("Not valid BinaryOperation tokenType " + tok::toString(b.getOpr()->getTokenType()));
    }
  }

  if(!isPass) {
    throw SemanticException(b.line, b.column, mes);
  }
}

void TypeChecker::visit(UnaryOperation& u) {
  if (isMustFunctionCall) {
    throw SemanticException(u.line, u.column, "Expect function call but find " + u.getOpr()->getValueString());
  }
  wasFunctionCall = false;

  u.getExpr()->accept(*this);
  auto& childType = u.getExpr()->type;
  if (childType== nullptr) {
    throw SemanticException(u.line, u.column,
      "Cannot " + u.getOpr()->getValueString() + " function");
  }

  bool isPass = false;
  std::string mesLvalue = "Expect lvalue in operator " + tok::toString(u.getOpr()->getTokenType());
  std::string mes = "Operation " + u.getOpr()->getValueString() +
    " to types \"" + childType->name + "\" not valid";

  switch (u.getOpr()->getTokenType()) {
    case tok::TokenType::Plus:
    case tok::TokenType::Minus: {
      isPass = childType->isInt() || childType->isDouble();
      u.type = childType;
      break;
    }
    case tok::TokenType::Not: {
      isPass = childType->isInt() || childType->isBool();
      u.type = childType;
      break;
    }
    case tok::TokenType::Caret: {
      if (!childType->isTypePointer()) {
        isPass = false;
        break;
      }
      isPass = true;
      u.type = childType->getPointerBase();
      break;
    }
    case tok::TokenType::At: {
      if (!LvalueChecker::is(u.expr)) {
        throw SemanticException(u.line, u.column, mesLvalue);
      }
      isPass = true;
      u.type = std::make_shared<Pointer>(childType);
      if (childType->isProcedureType() && stackTable.isFunction(childType->name)) {
        u.type = childType;
      }
      break;
    }
    default:
      throw std::logic_error("Not valid UnaryOperation tokenType " +
                             tok::toString(u.getOpr()->getTokenType()));
  }

  if (!isPass) {
    throw SemanticException(u.line, u.column, mes);
  }
}

void TypeChecker::visit(ArrayAccess& a) {
  if (isMustFunctionCall) {
    throw SemanticException(a.line, a.column, "Expect function call but find []");
  }
  wasFunctionCall = false;

  a.getName()->accept(*this);
  if (a.getName()->type == nullptr) {
    throw SemanticException(a.getName()->line, a.getName()->column, "Cannot [] on function");
  }

  for (auto& e : a.getListIndex()) {
    e->accept(*this);
    if (e->type == nullptr) {
      throw SemanticException(e->line, e->column, "Function not valid index");
    }
    if (!e->type->isInt()) {
      throw SemanticException(e->line, e->column, "Expect \"Integer\", but find " + e->type->name);
    }
  }
  auto& childType = a.getName()->type;
  ArrayAccessChecker::make(a, childType);
}

void TypeChecker::visit(RecordAccess& r) {
  if (isMustFunctionCall) {
    throw SemanticException(r.line, r.column, "Expect function call but find .");
  }
  wasFunctionCall = false;

  r.getRecord()->accept(*this);
  if (r.getRecord()->type == nullptr) {
    throw SemanticException(r.getRecord()->line, r.getRecord()->column, "Cannot . on function");
  }
  RecordAccessChecker::make(r, r.getRecord()->type);
}

void TypeChecker::visit(FunctionCall& f) {
  if (isMustFunctionCall) {
    isMustFunctionCall = false;
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
      (to->isTypePointer() && (from->isPurePointer() || from->equals(to.get()))) ;
  };

  if (isPass(to, from)) {
    return;
  }

  throw SemanticException("Cannot cast to type \"" + to->name +
    "\" expression with type \"" + from->name + "\"");
}

void TypeChecker::visit(AssignmentStmt& a) {
  if (isMustFunctionCall) {
    throw SemanticException(a.line, a.column, "Expect function call but find assigment");
  }
  a.getLeft()->accept(*this);
  a.getRight()->accept(*this);
  if (!LvalueChecker::is(a.left)) {
    throw SemanticException(a.left->line, a.left->column, "Expect lvalue in assigment");
  }
  std::string mes = "Operation " + a.getOpr()->getValueString() +
                    " to types \"" + a.getLeft()->type->name + "\" and \"" +
                    a.getRight()->type->name + "\" not valid";
  auto typeRight = a.getRight()->type;
  auto typeLeft = a.getLeft()->type;
  switch(a.getOpr()->getTokenType()) {
    case tok::TokenType::AssignmentWithMinus:
    case tok::TokenType::AssignmentWithPlus: {
      if (!checkTypePlusMinus(a, true)) {
        throw SemanticException(a.line, a.column, mes);
      }
      typeRight = a.type;
      break;
    }
    case tok::TokenType::AssignmentWithSlash:
    case tok::TokenType::AssignmentWithAsterisk: {
      if (!checkTypeSlashAsterisk(a, true)) {
        throw SemanticException(a.line, a.column, mes);
      }
      typeRight = a.type;
      break;
    }
    case tok::TokenType::Assignment: {
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
  if (!typeRight->equalsForCheckArgument(typeLeft.get())) {
    throw SemanticException(a.line, a.column, mes);
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
    throw SemanticException(i.getCondition()->line, i.getCondition()->column,
      "Expect type bool, but find " + i.getCondition()->type->name);
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
    throw SemanticException(w.getCondition()->line, w.getCondition()->column,
      "Expect type bool, but find " + w.getCondition()->type->name);
  }
}

void TypeChecker::visit(ForStmt& f) {
  f.getVar()->accept(*this);
  f.getLow()->accept(*this);
  f.getHigh()->accept(*this);
  f.getBlock()->accept(*this);
  if (!(f.getVar()->type->isInt() && f.getLow()->type->isInt() &&
        f.getHigh()->type->isInt())) {
    throw SemanticException(f.getVar()->line, f.getVar()->column, "Loop variable must be type int");
  }
}
