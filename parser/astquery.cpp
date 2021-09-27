#include "astquery.h"
#include "blocknode.h"
#include "valuenode.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>

std::shared_ptr<DeclarationNode>
ASTQuery::findDeclarationWithType(std::string name, std::string type,
                                  ASTNode tree) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      if (decl->getName() == name) {
        if (decl->getObjectType() == type) {
          return decl;
        }
      }
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
    for (std::string ns : namespaces) {
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
    for (ASTNode node : tree->getChildren()) {
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
    for (ASTNode node : tree->getChildren()) {
      if (node->getNodeType() == AST::Declaration ||
          node->getNodeType() == AST::BundleDeclaration) {
        auto decl = getDecl(node, typeName, namespaces, "");
        if (decl) {
          return decl;
        }
      }
    }
  }
  for (auto subScope : scope) {
    for (auto scopeMember : subScope.second) {
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
      for (ASTNode inheritsFromName : inherits->getChildren()) {
        if (inheritsFromName->getNodeType() == AST::String) {
          assert(0 == 1); // Disallowed
        } else if (inheritsFromName->getNodeType() == AST::Block) {
          auto inheritsBlock =
              std::static_pointer_cast<BlockNode>(inheritsFromName);
          std::shared_ptr<DeclarationNode> inheritedDeclaration =
              ASTQuery::findDeclaration(inheritsBlock->getName(), scope, tree);
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
          ASTQuery::findDeclaration(inheritsBlock->getName(), scope, tree);
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

std::shared_ptr<DeclarationNode>
ASTQuery::findDeclaration(std::string objectName, const ScopeStack &scopeStack,
                          ASTNode tree, std::vector<std::string> namespaces,
                          std::string platform) {
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
    return findDeclaration(objectName, scopeStack, tree, namespaces);
  } else {
    return nullptr;
  }
}
