#ifndef ASTRUNTIME_H
#define ASTRUNTIME_H

#include "valuenode.h"

class ASTRuntime
{
public:
  ASTRuntime();
  
  // Operators
  static std::shared_ptr<ValueNode> multiply(std::shared_ptr<ValueNode> left,
                                             std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> divide(std::shared_ptr<ValueNode> left,
                                           std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> add(std::shared_ptr<ValueNode> left,
                                        std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> subtract(std::shared_ptr<ValueNode> left,
                                             std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode>
  unaryMinus(std::shared_ptr<ValueNode> value);
  static std::shared_ptr<ValueNode>
  logicalAnd(std::shared_ptr<ValueNode> left, std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> logicalOr(std::shared_ptr<ValueNode> left,
                                              std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> logicalNot(std::shared_ptr<ValueNode> left);
};

#endif // ASTRUNTIME_H
