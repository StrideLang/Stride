#ifndef ASTRUNTIME_H
#define ASTRUNTIME_H

#include <functional>

#include "stride/parser/functionnode.h"
#include "stride/parser/valuenode.h"

struct StrideRuntimeStatus {
  bool ok;
  LangError err;
};

class ASTRuntime {
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

  static void resolveFunction(std::shared_ptr<FunctionNode> constraintFunction,
                              std::vector<ASTNode> input,
                              std::vector<ASTNode> output,
                              StrideRuntimeStatus &status);

private:
  // Node functions
  // TODO we should have type specific versions of these for speed, so we do the
  // check on parsing but the runtime does not need to check types
  // This will also do away with the need for status
  static void nodesAreEqual(ASTNode &node1, ASTNode &node2, ASTNode &output,
                            StrideRuntimeStatus &status);

  static void nodesAreNotEqual(ASTNode &node1, ASTNode &node2, ASTNode &output,
                               StrideRuntimeStatus &status);

  static void nodesIsGreater(ASTNode &node1, ASTNode &node2, ASTNode &output,
                             StrideRuntimeStatus &status);

  static void nodesIsNotGreater(ASTNode &node1, ASTNode &node2, ASTNode &output,
                                StrideRuntimeStatus &status);

  static void nodesIsLesser(ASTNode &node1, ASTNode &node2, ASTNode &output,
                            StrideRuntimeStatus &status);

  static void nodesIsNotLesser(ASTNode &node1, ASTNode &node2, ASTNode &output,
                               StrideRuntimeStatus &status);

  static void nodesOr(ASTNode &node1, ASTNode &node2, ASTNode &output,
                      StrideRuntimeStatus &status);

  static void nodesAnd(ASTNode &node1, ASTNode &node2, ASTNode &output,
                       StrideRuntimeStatus &status);

  static void nodeNot(ASTNode &node1, ASTNode &output,
                      StrideRuntimeStatus &status);

  static void nodeIsNone(ASTNode &node1, ASTNode &output,
                         StrideRuntimeStatus &status);
};

#endif // ASTRUNTIME_H
