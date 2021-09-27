#include "astruntime.h"

#include <cassert>

ASTRuntime::ASTRuntime() {}

std::shared_ptr<ValueNode>
ASTRuntime::multiply(std::shared_ptr<ValueNode> left,
                     std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() * right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else { // Automatic casting from int to real
    assert((left->getNodeType() == AST::Real &&
            right->getNodeType() == AST::Int) ||
           (left->getNodeType() == AST::Int &&
            right->getNodeType() == AST::Real) ||
           (left->getNodeType() == AST::Real &&
            right->getNodeType() == AST::Real));
    return std::make_shared<ValueNode>(left->toReal() * right->toReal(),
                                       left->getFilename().data(),
                                       left->getLine());
  }
}

std::shared_ptr<ValueNode>
ASTRuntime::divide(std::shared_ptr<ValueNode> left,
                   std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() / right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else { // Automatic casting from int to real
    assert((left->getNodeType() == AST::Real &&
            right->getNodeType() == AST::Int) ||
           (left->getNodeType() == AST::Int &&
            right->getNodeType() == AST::Real) ||
           (left->getNodeType() == AST::Real &&
            right->getNodeType() == AST::Real));
    return std::make_shared<ValueNode>(left->toReal() / right->toReal(),
                                       left->getFilename().data(),
                                       left->getLine());
  }
}

std::shared_ptr<ValueNode> ASTRuntime::add(std::shared_ptr<ValueNode> left,
                                           std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() + right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else { // Automatic casting from int to real
    assert((left->getNodeType() == AST::Real &&
            right->getNodeType() == AST::Int) ||
           (left->getNodeType() == AST::Int &&
            right->getNodeType() == AST::Real) ||
           (left->getNodeType() == AST::Real &&
            right->getNodeType() == AST::Real));
    return std::make_shared<ValueNode>(left->toReal() + right->toReal(),
                                       left->getFilename().data(),
                                       left->getLine());
  }
}

std::shared_ptr<ValueNode>
ASTRuntime::subtract(std::shared_ptr<ValueNode> left,
                     std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() - right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else { // Automatic casting from int to real
    assert((left->getNodeType() == AST::Real &&
            right->getNodeType() == AST::Int) ||
           (left->getNodeType() == AST::Int &&
            right->getNodeType() == AST::Real) ||
           (left->getNodeType() == AST::Real &&
            right->getNodeType() == AST::Real));
    return std::make_shared<ValueNode>(left->toReal() - right->toReal(),
                                       left->getFilename().data(),
                                       left->getLine());
  }
}

std::shared_ptr<ValueNode>
ASTRuntime::unaryMinus(std::shared_ptr<ValueNode> value) {
  if (value->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        -value->getIntValue(), value->getFilename().data(), value->getLine());
  } else if (value->getNodeType() == AST::Real) {
    return std::make_shared<ValueNode>(
        -value->getRealValue(), value->getFilename().data(), value->getLine());
  }
  return nullptr;
}

std::shared_ptr<ValueNode>
ASTRuntime::logicalAnd(std::shared_ptr<ValueNode> left,
                       std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() & right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else if (left->getNodeType() == AST::Switch &&
             right->getNodeType() == AST::Switch) {
    return std::make_shared<ValueNode>(
        left->getSwitchValue() == right->getSwitchValue(),
        left->getFilename().data(), left->getLine());
  }
  return nullptr;
}

std::shared_ptr<ValueNode>
ASTRuntime::logicalOr(std::shared_ptr<ValueNode> left,
                      std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() | right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else if (left->getNodeType() == AST::Switch &&
             right->getNodeType() == AST::Switch) {
    return std::make_shared<ValueNode>(
        left->getSwitchValue() == right->getSwitchValue(),
        left->getFilename().data(), left->getLine());
  }
  return nullptr;
}

std::shared_ptr<ValueNode>
ASTRuntime::logicalNot(std::shared_ptr<ValueNode> value) {
  if (value->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        ~(value->getIntValue()), value->getFilename().data(), value->getLine());
  } else if (value->getNodeType() == AST::Switch) {
    return std::make_shared<ValueNode>(!value->getSwitchValue(),
                                       value->getFilename().data(),
                                       value->getLine());
  }
  return nullptr;
}
