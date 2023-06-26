#include "stride/codegen/astruntime.hpp"

#include <cassert>
#include <iostream>

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

void ASTRuntime::nodesAreEqual(ASTNode &node1, ASTNode &node2, ASTNode &output,
                               StrideRuntimeStatus &status) {
  assert(output->getNodeType() == AST::Switch);
  auto outSwitch = std::static_pointer_cast<ValueNode>(output);
  if (node1->getNodeType() == AST::Int && node2->getNodeType() == AST::Int) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() ==
        std::static_pointer_cast<ValueNode>(node2)->getIntValue();
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Real) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() ==
        std::static_pointer_cast<ValueNode>(node2)->getRealValue();
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Int) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() ==
        std::static_pointer_cast<ValueNode>(node2)->getIntValue();
  } else if (node1->getNodeType() == AST::Int &&
             node2->getNodeType() == AST::Real) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() ==
        std::static_pointer_cast<ValueNode>(node2)->getRealValue();
  } else if (node1->getNodeType() == AST::Switch &&
             node2->getNodeType() == AST::Switch) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getSwitchValue() ==
        std::static_pointer_cast<ValueNode>(node2)->getSwitchValue();
  } else {
    outSwitch->m_switch = false;
    status.ok = false;
    return;
  }
  status.ok = true;
}

void ASTRuntime::nodesAreNotEqual(ASTNode &node1, ASTNode &node2,
                                  ASTNode &output,
                                  StrideRuntimeStatus &status) {
  nodesAreEqual(node1, node2, output, status);
  if (status.ok) {
    assert(output->getNodeType() == AST::Switch);
    auto outputSwitch = std::static_pointer_cast<ValueNode>(output);
    outputSwitch->m_switch = !outputSwitch->m_switch;
  }
}

void ASTRuntime::nodesIsGreater(ASTNode &node1, ASTNode &node2, ASTNode &output,
                                StrideRuntimeStatus &status) {
  assert(output->getNodeType() == AST::Switch);
  auto outSwitch = std::static_pointer_cast<ValueNode>(output);
  if (node1->getNodeType() == AST::Int && node2->getNodeType() == AST::Int) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() >
        std::static_pointer_cast<ValueNode>(node2)->getIntValue();
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Real) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() >
        std::static_pointer_cast<ValueNode>(node2)->getRealValue();
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Int) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() >
        std::static_pointer_cast<ValueNode>(node2)->getIntValue();
  } else if (node1->getNodeType() == AST::Int &&
             node2->getNodeType() == AST::Real) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() >
        std::static_pointer_cast<ValueNode>(node2)->getRealValue();
  } else {
    status.ok = false;
    return;
  }
  status.ok = true;
}

void ASTRuntime::nodesIsNotGreater(ASTNode &node1, ASTNode &node2,
                                   ASTNode &output,
                                   StrideRuntimeStatus &status) {
  nodesIsGreater(node1, node2, output, status);
  if (status.ok) {
    assert(output->getNodeType() == AST::Switch);
    auto outputSwitch = std::static_pointer_cast<ValueNode>(output);
    outputSwitch->m_switch = !outputSwitch->m_switch;
  }
}

void ASTRuntime::nodesIsLesser(ASTNode &node1, ASTNode &node2, ASTNode &output,
                               StrideRuntimeStatus &status) {
  assert(output->getNodeType() == AST::Switch);
  auto outSwitch = std::static_pointer_cast<ValueNode>(output);
  if (node1->getNodeType() == AST::Int && node2->getNodeType() == AST::Int) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() <
        std::static_pointer_cast<ValueNode>(node2)->getIntValue();
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Real) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() <
        std::static_pointer_cast<ValueNode>(node2)->getRealValue();
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Int) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() <
        std::static_pointer_cast<ValueNode>(node2)->getIntValue();
  } else if (node1->getNodeType() == AST::Int &&
             node2->getNodeType() == AST::Real) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() <
        std::static_pointer_cast<ValueNode>(node2)->getRealValue();
  } else {
    status.ok = false;
    return;
  }
  status.ok = true;
}

void ASTRuntime::nodesIsNotLesser(ASTNode &node1, ASTNode &node2,
                                  ASTNode &output,
                                  StrideRuntimeStatus &status) {
  nodesIsLesser(node1, node2, output, status);
  if (status.ok) {
    assert(output->getNodeType() == AST::Switch);
    auto outputSwitch = std::static_pointer_cast<ValueNode>(output);
    outputSwitch->m_switch = !outputSwitch->m_switch;
  }
}

void ASTRuntime::nodesOr(ASTNode &node1, ASTNode &node2, ASTNode &output,
                         StrideRuntimeStatus &status) {
  assert(output->getNodeType() == AST::Switch);
  auto outSwitch = std::static_pointer_cast<ValueNode>(output);
  if (node1->getNodeType() == AST::Switch &&
      node2->getNodeType() == AST::Switch) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getSwitchValue() ||
        std::static_pointer_cast<ValueNode>(node2)->getSwitchValue();
    status.ok = true;
  }
  status.ok = false;
}

void ASTRuntime::nodesAnd(ASTNode &node1, ASTNode &node2, ASTNode &output,
                          StrideRuntimeStatus &status) {
  assert(output->getNodeType() == AST::Switch);
  auto outSwitch = std::static_pointer_cast<ValueNode>(output);
  if (node1->getNodeType() == AST::Switch &&
      node2->getNodeType() == AST::Switch) {
    outSwitch->m_switch =
        std::static_pointer_cast<ValueNode>(node1)->getSwitchValue() &&
        std::static_pointer_cast<ValueNode>(node2)->getSwitchValue();
    status.ok = true;
  }
  status.ok = false;
}

void ASTRuntime::nodeNot(ASTNode &node1, ASTNode &output,
                         StrideRuntimeStatus &status) {
  assert(output->getNodeType() == AST::Switch);
  auto outSwitch = std::static_pointer_cast<ValueNode>(output);
  if (node1->getNodeType() == AST::Switch) {
    outSwitch->m_switch =
        !std::static_pointer_cast<ValueNode>(node1)->getSwitchValue();
    status.ok = true;
  }
  status.ok = false;
}

void ASTRuntime::nodeIsNone(ASTNode &node1, ASTNode &output,
                            StrideRuntimeStatus &status) {
  assert(output->getNodeType() == AST::Switch);
  auto outSwitch = std::static_pointer_cast<ValueNode>(output);
  outSwitch->m_switch = node1->getNodeType() == AST::None;
  status.ok = true;
}

void ASTRuntime::resolveFunction(
    std::shared_ptr<FunctionNode> constraintFunction,
    std::vector<ASTNode> input, std::vector<ASTNode> output,
    StrideRuntimeStatus &status) {
  if (constraintFunction->getName() == "NotEqual") {
    // TODO we should not need to check as it has been checked before.
    if (input.size() == 2) {
      nodesAreNotEqual(input[0], input[1], output[0], status);
    } else {
      std::cerr << "ERROR: constraint function NotEqual fail." << std::endl;
    }
  } else if (constraintFunction->getName() == "Equal") {
    if (input.size() == 2) {
      nodesAreEqual(input[0], input[1], output[0], status);
    } else {
      std::cerr << "ERROR: constraint function Equal fail." << std::endl;
    }
  } else if (constraintFunction->getName() == "Greater") {
    if (input.size() == 2) {
      nodesAreNotEqual(input[0], input[1], output[0], status);
    } else {
      std::cerr << "ERROR: constraint function Greater fail." << std::endl;
    }
  } else if (constraintFunction->getName() == "GreaterOrEqual") {
    if (input.size() == 2) {
      ASTNode outNode, outNode2;
      nodesIsGreater(input[0], input[1], output[0], status);
      bool greater =
          std::static_pointer_cast<ValueNode>(output[0])->getSwitchValue();
      if (status.ok) {
        nodesAreEqual(input[0], input[1], output[0], status);
      }
      std::static_pointer_cast<ValueNode>(output[0])->m_switch =
          std::static_pointer_cast<ValueNode>(output[0])->getSwitchValue() ||
          greater;
    } else {
      std::cerr << "ERROR: constraint function GreaterOrEqual fail."
                << std::endl;
    }
  } else if (constraintFunction->getName() == "Less") {
    if (input.size() == 2) {
      nodesIsLesser(input[0], input[1], output[0], status);
    } else {
      std::cerr << "ERROR: constraint function Less fail." << std::endl;
    }
  } else if (constraintFunction->getName() == "LessOrEqual") {
    if (input.size() == 2) {
      ASTNode outNode, outNode2;
      nodesIsLesser(input[0], input[1], output[0], status);
      bool lesser =
          std::static_pointer_cast<ValueNode>(output[0])->getSwitchValue();
      if (status.ok) {
        nodesAreEqual(input[0], input[1], output[0], status);
      }
      std::static_pointer_cast<ValueNode>(output[0])->m_switch =
          std::static_pointer_cast<ValueNode>(output[0])->getSwitchValue() ||
          lesser;
    } else {
      std::cerr << "ERROR: constraint function LessOrEqual fail." << std::endl;
    }
  } else if (constraintFunction->getName() == "IsNone") {
    if (input.size() == 1) {
      std::static_pointer_cast<ValueNode>(output[0])->m_switch =
          input[0]->getNodeType() == AST::None;
      status.ok = true;
    } else {
      std::cerr << "ERROR: constraint function IsNone fail." << std::endl;
    }
  } else if (constraintFunction->getName() == "IsNotNone") {
    if (input.size() == 1) {
      std::static_pointer_cast<ValueNode>(output[0])->m_switch =
          input[0]->getNodeType() != AST::None;
      status.ok = true;
    } else {
      std::cerr << "ERROR: constraint function IsNotNone fail." << std::endl;
    }
  } else if (constraintFunction->getName() == "Error") {
    if (input.size() == 1 && input[0]->getNodeType() == AST::Switch &&
        std::static_pointer_cast<ValueNode>(input[0])->getSwitchValue()) {
      //      std::cerr << "Constraint: ERROR";
      status.ok = false;
      LangError &err = status.err;
      err.type = LangError::ConstraintFail;
      err.errorTokens.push_back("");
      err.errorTokens.push_back(constraintFunction->getFilename());
      err.errorTokens.push_back(std::to_string(constraintFunction->getLine()));

      auto errorMsg = constraintFunction->getPropertyValue("message");
      if (errorMsg && errorMsg->getNodeType() == AST::String) {
        err.errorTokens.push_back(
            std::static_pointer_cast<ValueNode>(errorMsg)->getStringValue());
      } else {
        err.errorTokens.push_back("Unspecified error");
      }
    } else {
      status.ok = true;
    }
  } else if (constraintFunction->getName() == "Or") {
    assert(0 == 1);
    status.ok = false;
    // TODO implement bitwise operators in constraints
  } else if (constraintFunction->getName() == "And") {
    assert(0 == 1);
    status.ok = false;
  } else if (constraintFunction->getName() == "Xor") {
    assert(0 == 1);
    status.ok = false;
  } else if (constraintFunction->getName() == "Not") {
    assert(0 == 1);
    status.ok = false;
  } else {
    status.ok = false;
  }
}
