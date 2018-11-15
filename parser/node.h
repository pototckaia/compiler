#pragma once

#include <string>
#include <memory>

#include "token.h"

namespace pr {

class Visitor;

class ASTNode {
 public:
  explicit ASTNode() = default;
  virtual ~ASTNode() = default;

  virtual void accept(Visitor&) = 0;
};


class Variable : public ASTNode {
 public:
  explicit Variable(std::unique_ptr<tok::TokenBase>&& n);
  ~Variable() override = default;

  const auto& getName() const { return name; }

  void accept(Visitor&) override;

 private:
  std::unique_ptr<tok::TokenBase> name;
};


class Literal : public ASTNode {
 public:
  explicit Literal(std::unique_ptr<tok::TokenBase>&&);
  ~Literal() override = default;

  const auto& getValue() const { return value; }

  void accept(Visitor&) override;

 private:
  std::unique_ptr<tok::TokenBase> value;
};


class BinaryOperation : public ASTNode {
 public:
  BinaryOperation(std::unique_ptr<tok::TokenBase>&&,
                  std::unique_ptr<ASTNode>&&,
                  std::unique_ptr<ASTNode>&&);
  ~BinaryOperation() override = default;

  const auto& getLeft() const { return left; }
  const auto& getRight() const { return right; }
  const auto& getOpr() const { return opr; }

  void accept(Visitor&) override ;

 private:
  std::unique_ptr<tok::TokenBase> opr;
  std::unique_ptr<ASTNode> left;
  std::unique_ptr<ASTNode> right;
};

} // namespace pr