#include "astquery.h"
#include "strideparser.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>

std::shared_ptr<DeclarationNode> ASTQuery::findDeclarationWithType(
    std::string objectName, std::string type, const ScopeStack &scopeStack,
    ASTNode tree, std::vector<std::string> namespaces, std::string platform) {
  auto decls = ASTQuery::findAllDeclarations(objectName, scopeStack, tree,
                                             namespaces, platform);
  for (const auto &decl : decls) {
    if (decl && decl->getObjectType() == type) {
      return decl;
    }
  }
  return nullptr;
}

std::vector<std::shared_ptr<DeclarationNode>> ASTQuery::findAllDeclarations(
    std::string objectName, const ScopeStack &scopeStack, ASTNode tree,
    std::vector<std::string> namespaces, std::string currentFramework) {
  std::vector<std::shared_ptr<DeclarationNode>> decls;
  std::vector<std::string> scopesList;
  std::istringstream iss(objectName);
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(), back_inserter(scopesList));

  if (objectName.size() > 0) {
    objectName = scopesList.back();
    scopesList.pop_back();
    for (const std::string &ns : namespaces) {
      scopesList.push_back(ns);
    }
  }
  for (auto subScopeIt = scopeStack.rbegin(); subScopeIt != scopeStack.rend();
       subScopeIt++) {
    auto subScope = *subScopeIt;
    for (ASTNode scopeNode : subScope.second) {
      if (scopeNode->getNodeType() == AST::BundleDeclaration ||
          scopeNode->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> decl =
            std::static_pointer_cast<DeclarationNode>(scopeNode);
        std::string name = decl->getName();
        if (name == objectName) {
          if (ASTQuery::namespaceMatch(scopesList, decl, currentFramework)) {
            if (std::find(decls.begin(), decls.end(), decl) == decls.end()) {
              decls.push_back(decl);
            }
          } else {
            auto frameworkNode = decl->getCompilerProperty("framework");
            if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
              std::string frameworkName =
                  std::static_pointer_cast<ValueNode>(frameworkNode)
                      ->getStringValue();
              if (ASTQuery::namespaceMatch(scopesList, decl, frameworkName)) {
                if (std::find(decls.begin(), decls.end(), decl) ==
                    decls.end()) {
                  decls.push_back(decl);
                }
              }
            }
          }
        }
      }
    }
  }
  if (tree) {
    for (const ASTNode &node : tree->getChildren()) {
      if (node->getNodeType() == AST::BundleDeclaration ||
          node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> decl =
            std::static_pointer_cast<DeclarationNode>(node);
        std::string name = decl->getName();
        if (name == objectName) {
          //          auto frameworkNode =
          //          decl->getCompilerProperty("framework"); std::string
          //          frameworkName; if (frameworkNode &&
          //          frameworkNode->getNodeType() == AST::String) {
          //            frameworkName =
          //                static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
          //          }
          if (ASTQuery::namespaceMatch(scopesList, decl, currentFramework)) {
            if (std::find(decls.begin(), decls.end(), decl) == decls.end()) {
              decls.push_back(decl);
            }
          }
        }
      }
    }
  }
  return decls;
}

std::shared_ptr<DeclarationNode>
ASTQuery::findTypeDeclaration(std::shared_ptr<DeclarationNode> block,
                              ScopeStack scope, ASTNode tree,
                              std::string currentFramework) {
  std::string typeName = block->getObjectType();
  return ASTQuery::findTypeDeclarationByName(
      typeName, scope, tree, block->getNamespaceList(), currentFramework);
}

std::shared_ptr<DeclarationNode> ASTQuery::findTypeDeclarationByName(
    std::string typeName, ScopeStack scope, ASTNode tree,
    std::vector<std::string> namespaces, std::string currentFramework) {
  auto getDecl =
      [&](ASTNode scopeMember, std::string typeName,
          std::vector<std::string> namespaces,
          std::string framework) -> std::shared_ptr<DeclarationNode> {
    std::shared_ptr<DeclarationNode> declarationNode =
        std::static_pointer_cast<DeclarationNode>(scopeMember);
    if (declarationNode->getObjectType() == "type" ||
        declarationNode->getObjectType() == "platformBlock") {
      ASTNode valueNode = declarationNode->getPropertyValue("typeName");
      if (valueNode && valueNode->getNodeType() == AST::String) {
        auto value = std::static_pointer_cast<ValueNode>(valueNode);
        if (typeName == value->getStringValue()) {
          if (ASTQuery::namespaceMatch(namespaces, declarationNode,
                                       currentFramework)) {
            return declarationNode;
          }
        }
      }
    } else if (declarationNode->getObjectType() == "platformModule") {
      if (typeName == declarationNode->getName()) {
        if (ASTQuery::namespaceMatch(namespaces, declarationNode,
                                     currentFramework)) {
          return declarationNode;
        }
      }
    }
    return nullptr;
  };
  if (tree) {
    for (const ASTNode &node : tree->getChildren()) {
      if (node->getNodeType() == AST::Declaration ||
          node->getNodeType() == AST::BundleDeclaration) {
        auto decl = getDecl(node, typeName, namespaces, "");
        if (decl) {
          return decl;
        }
      }
    }
  }
  for (const auto &subScope : scope) {
    for (const auto &scopeMember : subScope.second) {
      if (scopeMember->getNodeType() == AST::Declaration ||
          scopeMember->getNodeType() == AST::BundleDeclaration) {
        auto decl =
            getDecl(scopeMember, typeName, namespaces, currentFramework);
        if (decl) {
          return decl;
        }
      }
    }
  }
  return nullptr;
}

std::string ASTQuery::getNodeName(ASTNode node) {
  if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    return name->getName();
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    return bundle->getName();
  } else if (node->getNodeType() == AST::Function) {
    FunctionNode *func = static_cast<FunctionNode *>(node.get());
    return func->getName();
  } else if (node->getNodeType() == AST::List) {
    std::string listCommonName;
    for (const auto &elem : node->getChildren()) {
      auto elemName = getNodeName(elem);
      if (listCommonName.size() == 0) {
        listCommonName = elemName;
      } else if (elemName != listCommonName) {
        return std::string();
      }
    }
    return listCommonName;
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    auto decl = std::static_pointer_cast<DeclarationNode>(node);
    return decl->getName();
  } else {
  }
  return std::string();
}

std::vector<std::shared_ptr<DeclarationNode>>
ASTQuery::getInheritedTypes(std::shared_ptr<DeclarationNode> block,
                            ScopeStack scope, ASTNode tree) {
  std::vector<std::shared_ptr<DeclarationNode>> inheritedTypes;
  if (!block) {
    return {};
  }
  ASTNode inherits = block->getPropertyValue("inherits");
  if (inherits) {
    if (inherits->getNodeType() == AST::List) {
      for (const ASTNode &inheritsFromName : inherits->getChildren()) {
        if (inheritsFromName->getNodeType() == AST::String) {
          assert(0 == 1); // Disallowed
        } else if (inheritsFromName->getNodeType() == AST::Block) {
          auto inheritsBlock =
              std::static_pointer_cast<BlockNode>(inheritsFromName);
          std::shared_ptr<DeclarationNode> inheritedDeclaration =
              ASTQuery::findDeclarationByName(inheritsBlock->getName(), scope,
                                              tree);
          if (inheritedDeclaration) {
            inheritedTypes.push_back(inheritedDeclaration);
            auto parentTypes =
                ASTQuery::getInheritedTypes(inheritedDeclaration, scope, tree);
            inheritedTypes.insert(inheritedTypes.end(), parentTypes.begin(),
                                  parentTypes.end());
          }
        }
      }
    } else if (inherits->getNodeType() == AST::String) {
      assert(0 == 1); // Disallowed
    } else if (inherits->getNodeType() == AST::Block) {
      auto inheritsBlock = std::static_pointer_cast<BlockNode>(inherits);
      std::shared_ptr<DeclarationNode> inheritedDeclaration =
          ASTQuery::findDeclarationByName(inheritsBlock->getName(), scope,
                                          tree);
      if (inheritedDeclaration) {
        auto parentTypes =
            ASTQuery::getInheritedTypes(inheritedDeclaration, scope, tree);
        inheritedTypes.insert(inheritedTypes.end(), parentTypes.begin(),
                              parentTypes.end());
      }
    } else {
      std::cout << "Unexpected type for inherits property" << std::endl;
    }
  }
  return inheritedTypes;
}

std::vector<ASTNode>
ASTQuery::getInheritedPorts(std::shared_ptr<DeclarationNode> block,
                            ScopeStack scope, ASTNode tree) {
  std::vector<ASTNode> inheritedProperties;
  auto inheritedTypes = ASTQuery::getInheritedTypes(block, scope, tree);
  //  std::vector<std::string> inheritedName;
  for (auto typeDeclaration : inheritedTypes) {
    auto inheritedFromType = getPortsForTypeBlock(typeDeclaration, scope, tree);
    for (ASTNode property : inheritedFromType) {
      if (std::find(inheritedProperties.begin(), inheritedProperties.end(),
                    property) == inheritedProperties.end()) {
        inheritedProperties.push_back(property);
      }
      //      if (property->getNodeType() == AST::Declaration) {
      //        inheritedName.push_back(
      //            std::static_pointer_cast<DeclarationNode>(property)->getName());
      //      }
    }
  }
  return inheritedProperties;
}

std::vector<ASTNode>
ASTQuery::getPortsForType(std::string typeName, ScopeStack scope, ASTNode tree,
                          std::vector<std::string> namespaces,
                          std::string framework) {
  std::vector<ASTNode> portList;

  std::shared_ptr<DeclarationNode> typeBlock =
      ASTQuery::findTypeDeclarationByName(typeName, scope, tree, namespaces,
                                          framework);

  if (typeBlock) {
    if (typeBlock->getObjectType() == "type" ||
        typeBlock->getObjectType() == "platformModule" ||
        typeBlock->getObjectType() == "platformBlock") {
      ValueNode *name = static_cast<ValueNode *>(
          typeBlock->getPropertyValue("typeName").get());
      if (name) {
        assert(name->getNodeType() == AST::String);
        assert(name->getStringValue() == typeName);
        auto newPortList =
            ASTQuery::getPortsForTypeBlock(typeBlock, scope, tree);
        portList.insert(portList.end(), newPortList.begin(), newPortList.end());
      } else {
        std::cerr
            << "ASTQuery::getModulePortsForType type missing typeName port."
            << std::endl;
      }
    }

    //        QVector<ASTNode > inheritedProperties =
    //        getInheritedPorts(typeBlock, scope, tree); portList <<
    //        inheritedProperties;
  }

  return portList;
}

std::vector<ASTNode>
ASTQuery::getPortsForTypeBlock(std::shared_ptr<DeclarationNode> block,
                               ScopeStack scope, ASTNode tree) {
  ASTNode portsValue = block->getPropertyValue("properties");
  std::vector<ASTNode> outList;
  if (portsValue && portsValue->getNodeType() != AST::None) {
    assert(portsValue->getNodeType() == AST::List);
    ListNode *portList = static_cast<ListNode *>(portsValue.get());
    for (ASTNode port : portList->getChildren()) {
      outList.push_back(port);
    }
  }

  auto inherited = ASTQuery::getInheritedPorts(block, scope, tree);
  outList.insert(outList.end(), inherited.begin(), inherited.end());
  return outList;
}

std::vector<ASTNode>
ASTQuery::getValidTypesForPort(std::shared_ptr<DeclarationNode> typeDeclaration,
                               std::string portName, ScopeStack scope,
                               ASTNode tree) {
  std::vector<ASTNode> validTypes;
  auto portList = ASTQuery::getPortsForTypeBlock(typeDeclaration, scope, tree);
  for (ASTNode node : portList) {
    DeclarationNode *portNode = static_cast<DeclarationNode *>(node.get());
    ValueNode *name =
        static_cast<ValueNode *>(portNode->getPropertyValue("name").get());
    assert(name->getNodeType() == AST::String);
    if (name->getStringValue() == portName) {
      ListNode *typesPort =
          static_cast<ListNode *>(portNode->getPropertyValue("types").get());
      assert(typesPort->getNodeType() == AST::List);
      for (ASTNode type : typesPort->getChildren()) {
        validTypes.push_back(type);
      }
    }
  }
  return validTypes;
}

std::vector<ASTNode>
ASTQuery::getModuleBlocks(std::shared_ptr<DeclarationNode> moduleDecl) {
  std::vector<ASTNode> blocks;

  ASTNode blocksNode = moduleDecl->getPropertyValue("blocks");
  if (blocksNode) {
    if (blocksNode->getNodeType() == AST::Declaration ||
        blocksNode->getNodeType() == AST::BundleDeclaration) {
      blocks.push_back(blocksNode);
    } else if (blocksNode->getNodeType() == AST::List) {
      for (ASTNode node : blocksNode->getChildren()) {
        if (node->getNodeType() == AST::Declaration ||
            node->getNodeType() == AST::BundleDeclaration) {
          blocks.push_back(node);
        }
      }
    }
  }
  return blocks;
}

std::shared_ptr<DeclarationNode> ASTQuery::getModuleMainOutputPortBlock(
    std::shared_ptr<DeclarationNode> moduleDecl) {
  ListNode *ports =
      static_cast<ListNode *>(moduleDecl->getPropertyValue("ports").get());
  if (ports && ports->getNodeType() == AST::List) {
    for (ASTNode port : ports->getChildren()) {
      std::shared_ptr<DeclarationNode> portBlock =
          std::static_pointer_cast<DeclarationNode>(port);
      if (portBlock->getObjectType() == "mainOutputPort") {
        return portBlock;
      }
    }
  } else if (ports && ports->getNodeType() == AST::None) {
    // If port list is None, then ignore
  } else {
    std::cerr << "ERROR! ports property must be a list or None!" << std::endl;
  }
  return nullptr;
}

std::shared_ptr<DeclarationNode> ASTQuery::getModuleMainInputPortBlock(
    std::shared_ptr<DeclarationNode> moduleDecl) {
  ListNode *ports =
      static_cast<ListNode *>(moduleDecl->getPropertyValue("ports").get());
  if (ports) {
    if (ports->getNodeType() == AST::List) {
      for (ASTNode port : ports->getChildren()) {
        std::shared_ptr<DeclarationNode> portBlock =
            std::static_pointer_cast<DeclarationNode>(port);
        if (portBlock->getObjectType() == "mainInputPort") {
          return portBlock;
        }
      }
    } else if (ports->getNodeType() == AST::None) {
      // If port list is None, then ignore
    } else {
      std::cerr << "ERROR! ports property must be a list or None!" << std::endl;
    }
  }
  return nullptr;
}

std::shared_ptr<DeclarationNode>
ASTQuery::getModulePort(std::shared_ptr<DeclarationNode> moduleDecl,
                        std::string name) {
  ListNode *ports =
      static_cast<ListNode *>(moduleDecl->getPropertyValue("ports").get());
  if (ports->getNodeType() == AST::List) {
    for (ASTNode port : ports->getChildren()) {
      std::shared_ptr<DeclarationNode> portBlock =
          std::static_pointer_cast<DeclarationNode>(port);
      if (portBlock->getName() == name) {
        if (portBlock->getObjectType() == "mainInputPort" ||
            portBlock->getObjectType() == "mainOutputPort" ||
            portBlock->getObjectType() == "propertyInputPort" ||
            portBlock->getObjectType() == "propertyOutputPort") {
          return portBlock;
        } else {
          std::cerr << "WARNING name found in getPort() but unexpected type"
                    << std::endl;
        }
      }
    }
  } else if (ports->getNodeType() == AST::None) {
    // If port list is None, then ignore
  } else {
    std::cerr << "ERROR! ports property must be a list or None!" << std::endl;
  }
  return nullptr;
}

std::vector<std::string> ASTQuery::getModulePortNames(
    std::shared_ptr<DeclarationNode> blockDeclaration) {
  std::vector<std::string> portNames;
  if (blockDeclaration->getObjectType() == "module") {
    auto portsList = std::static_pointer_cast<ListNode>(
        blockDeclaration->getPropertyValue("ports"));
    if (portsList->getNodeType() == AST::List) {
      for (ASTNode portDeclaration : portsList->getChildren()) {
        if (portDeclaration->getNodeType() == AST::Declaration) {
          auto port =
              std::static_pointer_cast<DeclarationNode>(portDeclaration);
          ASTNode nameProperty = port->getPropertyValue("name");
          if (nameProperty) {
            assert(nameProperty->getNodeType() == AST::String);
            if (nameProperty->getNodeType() == AST::String) {
              portNames.push_back(
                  static_cast<ValueNode *>(nameProperty.get())->toString());
            }
          }
        }
      }
    }
  } else if (blockDeclaration->getObjectType() == "platformModule") {
    auto portsList = std::static_pointer_cast<ListNode>(
        blockDeclaration->getPropertyValue("ports"));
    if (portsList->getNodeType() == AST::List) {
      for (ASTNode portDeclaration : portsList->getChildren()) {
        if (portDeclaration->getNodeType() == AST::Declaration) {
          auto port =
              std::static_pointer_cast<DeclarationNode>(portDeclaration);
          ASTNode nameProperty = port->getPropertyValue("name");
          if (nameProperty) {
            assert(nameProperty->getNodeType() == AST::String);
            if (nameProperty->getNodeType() == AST::String) {
              portNames.push_back(
                  static_cast<ValueNode *>(nameProperty.get())->toString());
            }
          }
        }
      }
    }
  }
  return portNames;
}

bool ASTQuery::namespaceMatch(std::vector<std::string> scopeList,
                              std::shared_ptr<DeclarationNode> decl,
                              std::string currentFramework) {
  if (scopeList.size() == 1 && scopeList[0].size() == 0) {
    scopeList.clear();
  }
  auto declScopesNode = decl->getCompilerProperty("namespaceTree");

  auto frameworkNode = decl->getCompilerProperty("framework");
  std::string framework;
  if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
    framework =
        std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
  }
  std::vector<std::string> namespaceTree;
  // Add import namespaces
  if (declScopesNode) {
    for (auto node : declScopesNode->getChildren()) {
      assert(node->getNodeType() == AST::String);

      namespaceTree.push_back(
          std::static_pointer_cast<ValueNode>(node)->getStringValue());
    }
  }
  // Remove namespace if calling from within the same framework
  if (framework == currentFramework && namespaceTree.size() > 0 &&
      namespaceTree.at(0) == currentFramework) {
    //    if (scopeList.size() > 0 && scopeList.at(0) != currentFramework) {
    namespaceTree.erase(namespaceTree.begin());
    //    }
  }
  if (framework.size() > 0 && framework != currentFramework) {
    if (namespaceTree.size() == 0 || framework != namespaceTree.at(0)) {
      namespaceTree.insert(namespaceTree.begin(), framework);
    }
  } else {
    //      if (scopeList.size() > 0 && scopeList[0] == framework) {
    //        scopeList.erase(scopeList.begin());
    //      }
  }
  if (namespaceTree.size() != scopeList.size()) {
    return false;
  }

  for (size_t i = 0; i < namespaceTree.size(); i++) {
    if (namespaceTree[i] != scopeList[i]) {
      return false;
    }
  }

  return true;
}

std::shared_ptr<DeclarationNode> ASTQuery::findDeclarationByName(
    std::string objectName, const ScopeStack &scopeStack, ASTNode tree,
    std::vector<std::string> namespaces, std::string platform) {
  std::vector<std::string> scopesList;
  std::istringstream iss(objectName);
  std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(), back_inserter(scopesList));

  if (objectName.size() > 0) {
    objectName = scopesList.back();
    scopesList.pop_back();
    for (std::string &ns : namespaces) {
      scopesList.push_back(ns);
    }
  }
  for (auto subScopeIt = scopeStack.rbegin(); subScopeIt != scopeStack.rend();
       subScopeIt++) {
    auto subScope = *subScopeIt;
    for (ASTNode scopeNode : subScope.second) {
      if (scopeNode->getNodeType() == AST::BundleDeclaration ||
          scopeNode->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> decl =
            std::static_pointer_cast<DeclarationNode>(scopeNode);
        std::string name = decl->getName();
        if (name == objectName) {
          if (ASTQuery::namespaceMatch(scopesList, decl, platform)) {
            return decl;
          }
        }
      }
    }
  }
  if (tree) {
    for (ASTNode node : tree->getChildren()) {
      if (node->getNodeType() == AST::BundleDeclaration ||
          node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> decl =
            std::static_pointer_cast<DeclarationNode>(node);
        std::string name = decl->getName();
        if (name == objectName) {
          auto frameworkNode = decl->getCompilerProperty("framework");
          std::string frameworkName;
          if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
            frameworkName = std::static_pointer_cast<ValueNode>(frameworkNode)
                                ->getStringValue();
          }
          if (ASTQuery::namespaceMatch(scopesList, decl, platform)) {
            if (platform.size() > 0) {
              auto platformNode = decl->getCompilerProperty("framework");
              if (platformNode && platformNode->getNodeType() == AST::String) {
                auto platformString =
                    std::static_pointer_cast<ValueNode>(platformNode)
                        ->getStringValue();
                if (platformString == platform) {
                  return decl;
                }
              }
            } else {
              return decl;
            }
          }
        }
      }
    }
  }
  if (platform.size() > 0) {
    // If not found try to find in root namespace
    return findDeclarationByName(objectName, scopeStack, tree, namespaces);
  } else {
    return nullptr;
  }
}
