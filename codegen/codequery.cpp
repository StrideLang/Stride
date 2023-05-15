#include "codequery.hpp"

#include "astquery.h"

#include <cassert>
#include <iostream>

std::string CodeQuery::resolveBundleDataType(BundleNode *bundle,
                                             ScopeStack scopeStack,
                                             ASTNode tree) {
  std::shared_ptr<DeclarationNode> declaration =
      ASTQuery::findDeclarationByName(bundle->getName(), scopeStack, tree);
  if (declaration) {
    if (declaration->getObjectType() == "constant") {
      std::shared_ptr<PropertyNode> property =
          ASTQuery::findPropertyByName(declaration->getProperties(), "value");
      if (property) {
        return resolveNodeOutDataType(property->getValue(), scopeStack, tree);
      }
    } else if (declaration->getObjectType() == "signal") {
      std::vector<std::shared_ptr<PropertyNode>> properties =
          declaration->getProperties();
      ASTNode typeNode = declaration->getPropertyValue("type");
      if (typeNode && typeNode->getNodeType() == AST::Block) {
        return std::static_pointer_cast<BlockNode>(typeNode)->getName();
      }
      return "";
    }
  }
  return "";
}

std::string CodeQuery::resolveBlockDataType(BlockNode *name,
                                            ScopeStack scopeStack,
                                            ASTNode tree) {
  std::shared_ptr<DeclarationNode> declaration =
      ASTQuery::findDeclarationByName(name->getName(), scopeStack, tree);
  if (declaration) {
    if (declaration->getObjectType() == "constant") {
      std::vector<std::shared_ptr<PropertyNode>> properties =
          declaration->getProperties();
      std::shared_ptr<PropertyNode> property =
          ASTQuery::findPropertyByName(properties, "value");
      if (property) {
        return resolveNodeOutDataType(property->getValue(), scopeStack, tree);
      }
    } else if (declaration->getObjectType() == "signal") {
      std::vector<std::shared_ptr<PropertyNode>> properties =
          declaration->getProperties();
      ASTNode typeNode = declaration->getPropertyValue("type");
      if (typeNode && typeNode->getNodeType() == AST::Block) {
        return std::static_pointer_cast<BlockNode>(typeNode)->getName();
      }
      return "";
    } else {
      return "";
    }
  }
  return "";
}

std::string CodeQuery::resolveNodeOutDataType(ASTNode node,
                                              ScopeStack scopeStack,
                                              ASTNode tree) {
  if (node->getNodeType() == AST::Int) {
    return "_IntType";
  } else if (node->getNodeType() == AST::Real) {
    return "_RealType";
  } else if (node->getNodeType() == AST::Switch) {
    return "_SwitchType";
  } else if (node->getNodeType() == AST::String) {
    return "_StringType";
  } else if (node->getNodeType() == AST::List) {
    return resolveListDataType(static_cast<ListNode *>(node.get()), scopeStack,
                               tree);
  } else if (node->getNodeType() == AST::Bundle) {
    return resolveBundleDataType(static_cast<BundleNode *>(node.get()),
                                 scopeStack, tree);
  } else if (node->getNodeType() == AST::Expression) {
    return resolveExpressionDataType(static_cast<ExpressionNode *>(node.get()),
                                     scopeStack, tree);
  } else if (node->getNodeType() == AST::Block) {
    return resolveBlockDataType(static_cast<BlockNode *>(node.get()),
                                scopeStack, tree);
  } else if (node->getNodeType() == AST::Range) {
    return resolveRangeDataType(static_cast<RangeNode *>(node.get()),
                                scopeStack, tree);
  } else if (node->getNodeType() == AST::PortProperty) {
    return resolvePortPropertyDataType(
        static_cast<PortPropertyNode *>(node.get()), scopeStack, tree);
  }
  return "";
}

std::string CodeQuery::resolveListDataType(ListNode *listnode,
                                           ScopeStack scopeStack,
                                           ASTNode tree) {
  std::vector<ASTNode> members = listnode->getChildren();
  if (members.size() == 0) {
    return "";
  }
  ASTNode firstMember = members.at(0);
  auto type = resolveNodeOutDataType(firstMember, scopeStack, tree);

  for (const ASTNode &member : members) {
    auto nextPortType = resolveNodeOutDataType(member, scopeStack, tree);
    if (type != nextPortType) {
      if (type == "_IntType" &&
          nextPortType == "_RealType") { // List becomes Real if Real found
        type = "_RealType";
      } else if (type == "_RealType" &&
                 nextPortType == "_IntType") { // Int in Real list
        // Nothing here for now
      } else { // Invalid combination
        return "";
      }
    }
  }
  return type;
}

std::string CodeQuery::resolveExpressionDataType(ExpressionNode *exprnode,
                                                 ScopeStack scopeStack,
                                                 ASTNode tree) {
  if (!exprnode->isUnary()) {
    ASTNode left = exprnode->getLeft();
    ASTNode right = exprnode->getRight();
    auto leftType = resolveNodeOutDataType(left, scopeStack, tree);
    auto rightType = resolveNodeOutDataType(right, scopeStack, tree);

    auto type = leftType;
    if (type != rightType) {
      if (type == "_IntType" &&
          rightType == "_RealType") { // Expr becomes Real if Real found
        type = "_RealType";
      } else if (type == "_RealType" && rightType == "_IntType") {
        // Int in Real list
        // Nothing here for now
      } else { // Invalid combination
        return "";
      }
    }
    return type;

  } else {

    return resolveNodeOutDataType(exprnode->getValue(), scopeStack, tree);
  }
}

std::string CodeQuery::resolveRangeDataType(RangeNode *rangenode,
                                            ScopeStack scopeStack,
                                            ASTNode tree) {
  auto leftType =
      resolveNodeOutDataType(rangenode->startIndex(), scopeStack, tree);
  auto rightType =
      resolveNodeOutDataType(rangenode->endIndex(), scopeStack, tree);
  if (leftType == rightType) {
    return leftType;
  }
  return "";
}

std::string
CodeQuery::resolvePortPropertyDataType(PortPropertyNode *portproperty,
                                       ScopeStack scopeStack, ASTNode tree) {
  // FIXME implement correctly. Should be read from framework?
  if (portproperty->getPortName() == "size") {
    return "_IntType";
  } else if (portproperty->getPortName() == "rate") {
    return "_RealType";
  }
  return "_RealType";
}
