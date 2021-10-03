#include "codeanalysis.hpp"

#include "astquery.h"

#include <iostream>

std::shared_ptr<DeclarationNode>
CodeAnalysis::findDomainDeclaration(std::string domainName,
                                    std::string framework, ASTNode tree) {
  std::string domainFramework;
  auto separatorIndex = domainName.find("::");
  if (separatorIndex != std::string::npos) {
    domainFramework = domainName.substr(0, separatorIndex);
    domainName = domainName.substr(separatorIndex + 2);
  }

  if (domainFramework != framework && domainFramework != "") {
    std::cerr << __FILE__ << ":" << __LINE__ << "Unexpected domain mismatch"
              << std::endl;
  }
  for (const ASTNode &node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          std::static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "_domainDefinition") {
        if (domainName == decl->getName()) {
          auto domainDeclFramework = decl->getCompilerProperty("framework");
          if (domainDeclFramework &&
              domainDeclFramework->getNodeType() == AST::String) {
            if (framework ==
                std::static_pointer_cast<ValueNode>(domainDeclFramework)
                    ->getStringValue()) {
              return decl;
            }
          }
          if (framework.size() == 0 && !domainDeclFramework) {
            return decl;
          }
        }
      }
    }
  }
  return nullptr;
}

std::shared_ptr<DeclarationNode>
CodeAnalysis::findDomainDeclaration(std::string domainId, ASTNode tree) {
  std::string domainFramework;

  auto separatorIndex = domainId.find("::");
  if (separatorIndex != std::string::npos) {
    domainFramework = domainId.substr(0, separatorIndex);
    if (domainFramework !=
        CodeAnalysis::getFrameworkForDomain(domainId, tree)) {
      std::cerr << __FILE__ << ":" << __LINE__ << "ERROR framework mismatch"
                << std::endl;
    }
    domainId = domainId.substr(separatorIndex + 2);
  }
  auto trimmedDomain = domainId.substr(0, domainId.find(':'));
  return findDomainDeclaration(trimmedDomain, domainFramework, tree);
}

ASTNode CodeAnalysis::getNodeDomain(ASTNode node, ScopeStack scopeStack,
                                    ASTNode tree) {
  ASTNode domainNode = nullptr;
  if (!node) {
    return nullptr;
  }

  if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(name->getName(), scopeStack, tree,
                                        name->getNamespaceList());

    if (declaration) {
      auto typeDeclaration =
          ASTQuery::findTypeDeclaration(declaration, scopeStack, tree);
      if (typeDeclaration &&
          typeDeclaration->getObjectType() == "platformModule") {
        domainNode = name->getCompilerProperty("domain")->deepCopy();
        domainNode->setNamespaceList(node->getNamespaceList());
      } else {
        if (declaration->getDomain()) {
          domainNode = declaration->getDomain()->deepCopy();
          if (node->getNamespaceList().size() > 0) {
            domainNode->setNamespaceList(node->getNamespaceList());
          }
        }
      }
    }
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *name = static_cast<BundleNode *>(node.get());

    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(name->getName(), scopeStack, tree,
                                        name->getNamespaceList());
    if (declaration && declaration->getDomain()) {
      domainNode = declaration->getDomain()->deepCopy();
      domainNode->setNamespaceList(node->getNamespaceList());
    }
  } else if (node->getNodeType() == AST::List) {
    std::vector<std::string> domainList;
    std::string tempDomainName;
    for (const ASTNode &member : node->getChildren()) {
      if (member->getNodeType() == AST::Block) {
        BlockNode *name = static_cast<BlockNode *>(member.get());
        std::shared_ptr<DeclarationNode> declaration =
            ASTQuery::findDeclarationByName(name->getName(), scopeStack, tree,
                                            name->getNamespaceList());
        if (declaration) {
          tempDomainName =
              CodeAnalysis::getNodeDomainName(declaration, scopeStack, tree);
        }
      } else if (member->getNodeType() == AST::Bundle) {
        BundleNode *name = static_cast<BundleNode *>(member.get());
        std::shared_ptr<DeclarationNode> declaration =
            ASTQuery::findDeclarationByName(name->getName(), scopeStack, tree);
        if (declaration) {
          tempDomainName =
              CodeAnalysis::getNodeDomainName(declaration, scopeStack, tree);
        }
      } else if (member->getNodeType() == AST::Int ||
                 member->getNodeType() == AST::Real ||
                 member->getNodeType() == AST::String ||
                 member->getNodeType() == AST::Switch ||
                 member->getNodeType() == AST::PortProperty) {
        continue; // Don't append empty domain to domainList, as a value
                  // should take any domain.
      } else {
        tempDomainName =
            CodeAnalysis::getNodeDomainName(member, scopeStack, tree);
      }
      domainList.push_back(tempDomainName);
    }
    // There is no real resolution for domain from a list. The caller
    // should query each member for the domain. For now, assuming the first
    // element of the list determines domain.
    return CodeAnalysis::getNodeDomain(node->getChildren().at(0), scopeStack,
                                       tree);
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    auto decl = static_cast<DeclarationNode *>(node.get());
    if (decl->getObjectType() == "reaction") {
      decl->getCompilerProperty("triggerDomain");
      // FIXME implement domain support for reactions
    } else {
      if (decl->getDomain()) {
        domainNode = decl->getDomain()->deepCopy();
        auto frameworkNode = node->getCompilerProperty("framework");
        if (frameworkNode) {
          domainNode->addScope(
              std::static_pointer_cast<ValueNode>(frameworkNode)
                  ->getStringValue());
        }
        //        domainNode->setNamespaceList(node->getNamespaceList());
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    domainNode = node->getCompilerProperty(
        "domain"); // static_cast<FunctionNode *>(node.get())->getDomain();
    if (!domainNode) {
      auto funcDecl = ASTQuery::findDeclarationByName(
          std::static_pointer_cast<FunctionNode>(node)->getName(), scopeStack,
          tree, node->getNamespaceList());
      if (funcDecl && funcDecl->getObjectType() == "platformModule") {
        domainNode = funcDecl->getPropertyValue("domain");

        auto domainId = CodeAnalysis::getDomainIdentifier(domainNode, {}, tree);
        std::string domainFramework;
        auto separatorIndex = domainId.find("::");
        if (separatorIndex != std::string::npos) {
          domainFramework = domainId.substr(0, separatorIndex);
          domainId = domainId.substr(separatorIndex + 2);
        }

        auto domainDecl = CodeAnalysis::findDomainDeclaration(
            domainId, domainFramework, tree);
        if (domainDecl) {
          auto parentDomain = domainDecl->getPropertyValue("parentDomain");
          if (parentDomain && parentDomain->getNodeType() == AST::Block) {
            auto domainInstanceCountNode =
                parentDomain->getCompilerProperty("instances");
            if (!domainInstanceCountNode) {
              domainInstanceCountNode =
                  std::make_shared<ValueNode>((int64_t)0, __FILE__, __LINE__);
              parentDomain->setCompilerProperty("instances",
                                                domainInstanceCountNode);
            }
          }
        }
      }
    }
  } else if (node->getNodeType() == AST::Expression) {
    ExpressionNode *expr = static_cast<ExpressionNode *>(node.get());
    if (expr->isUnary()) {
      domainNode =
          CodeAnalysis::getNodeDomain(expr->getValue(), scopeStack, tree);
    } else {
      if (expr->getCompilerProperty("samplingDomain")) {
        return expr->getCompilerProperty("samplingDomain");
      }
    }
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *stream = static_cast<StreamNode *>(node.get());
    ASTNode leftDomain = getNodeDomain(stream->getLeft(), scopeStack, tree);
    ASTNode rightDomain = getNodeDomain(stream->getRight(), scopeStack, tree);
    if (!leftDomain) {
      leftDomain = rightDomain;
    } else if (!rightDomain) {
      rightDomain = leftDomain;
    }
    if ((leftDomain && rightDomain) &&
        (getNodeDomainName(leftDomain, scopeStack, tree) ==
         getNodeDomainName(rightDomain, scopeStack, tree))) {
      return leftDomain;
    }
  } else if (node->getNodeType() == AST::Real ||
             node->getNodeType() == AST::Int ||
             node->getNodeType() == AST::Switch ||
             node->getNodeType() == AST::String) {
    domainNode = std::static_pointer_cast<ValueNode>(node)->getDomain();
  } else if (node->getNodeType() == AST::PortProperty) {

    domainNode = node->getCompilerProperty("domain");
  }

  return domainNode;
}

std::string CodeAnalysis::getNodeDomainName(ASTNode node, ScopeStack scopeStack,
                                            ASTNode tree) {
  std::string domainName;
  ASTNode domainNode = CodeAnalysis::getNodeDomain(node, scopeStack, tree);
  if (domainNode) {
    domainName =
        CodeAnalysis::getDomainIdentifier(domainNode, scopeStack, tree);
  }
  return domainName;
}

std::string CodeAnalysis::getDomainIdentifier(ASTNode domain,
                                              ScopeStack scopeStack,
                                              ASTNode tree) {
  std::string name;
  // To avoid inconsistencies, we rely on the block name rather than the
  // domainName property. This guarantees uniqueness within a scope.
  // TODO we should add scope markers to this identifier to avoid clashes
  if (domain) {
    if (domain->getNodeType() == AST::Block) {
      auto domainBlock = std::static_pointer_cast<BlockNode>(domain);
      std::string framework;
      if (domainBlock->getScopeLevels() > 0) {
        framework = domainBlock->getNamespaceList()[0];
      }
      std::shared_ptr<DeclarationNode> domainDeclaration =
          CodeAnalysis::findDomainDeclaration(domainBlock->getName(), framework,
                                              tree);
      if (domainDeclaration) {
        if (domainDeclaration->getObjectType() == "_domainDefinition") {
          name = domainDeclaration->getName();
        } else if (domainDeclaration->getObjectType() == "PlatformDomain") {
          auto domainNameNode = domainDeclaration->getPropertyValue("value");
          name = getDomainIdentifier(domainNameNode, scopeStack, tree);
        }
        auto frameworkNode =
            domainDeclaration->getCompilerProperty("framework");
        std::string frameworkName;
        if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
          frameworkName = std::static_pointer_cast<ValueNode>(frameworkNode)
                              ->getStringValue();
          if (frameworkName.size() > 0) {
            name = frameworkName + "::" + name;
          }
        }
      }
      auto domainInstanceIndex = domain->getCompilerProperty("domainInstance");
      if (domainInstanceIndex) {
        int index = std::static_pointer_cast<ValueNode>(domainInstanceIndex)
                        ->getIntValue();
        name += ":" + std::to_string(index);
      }
    } else if (domain->getNodeType() == AST::Bundle) {
      auto domainBlock = std::static_pointer_cast<BundleNode>(domain);
      auto domainDeclaration = ASTQuery::findDeclarationByName(
          domainBlock->getName(), scopeStack, tree);
      if (domainDeclaration) {
        if (domainDeclaration->getObjectType() == "_domainDefinition") {
          name = domainDeclaration->getName();
        } else if (domainDeclaration->getObjectType() == "PlatformDomain") {
          auto domainNameNode = domainDeclaration->getPropertyValue("value");
          name = getDomainIdentifier(domainNameNode, scopeStack, tree);
        }
      }
    } else if (domain->getNodeType() == AST::String) {
      // Should anything be added to the id? Scope?
      name = std::static_pointer_cast<ValueNode>(domain)->getStringValue();
      std::cerr << "DEPRECATED: String domain no longer allowed." << std::endl;
      //      assert(0 == 1); // Should not be allowed
    } else if (domain->getNodeType() == AST::PortProperty) {
      // Should anything be added to the id? Scope?
      auto portProperty = std::static_pointer_cast<PortPropertyNode>(domain);
      name = portProperty->getName() + "_" + portProperty->getPortName();
    }
  }
  return name;
}

std::string CodeAnalysis::getFrameworkForDomain(std::string domainName,
                                                ASTNode tree) {
  std::string domainFramework;
  auto separatorIndex = domainName.find("::");
  if (separatorIndex != std::string::npos) {
    domainFramework = domainName.substr(0, separatorIndex);
    domainName = domainName.substr(separatorIndex + 2);
  }
  for (const ASTNode &node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      DeclarationNode *decl = static_cast<DeclarationNode *>(node.get());
      if (decl->getObjectType() == "_domainDefinition") {
        std::string declDomainName = decl->getName();
        if (declDomainName == domainName) {
          ASTNode frameworkNameValue = decl->getPropertyValue("framework");
          if (frameworkNameValue->getNodeType() == AST::String) {
            return static_cast<ValueNode *>(frameworkNameValue.get())
                ->getStringValue();
          } else if (frameworkNameValue->getNodeType() == AST::Block) {
            auto fwBlock =
                std::static_pointer_cast<BlockNode>(frameworkNameValue);

            auto declImportFramework = decl->getCompilerProperty("framework");
            std::string frameworkName;
            if (declImportFramework) {
              frameworkName =
                  std::static_pointer_cast<ValueNode>(declImportFramework)
                      ->getStringValue();
            }
            std::shared_ptr<DeclarationNode> fwDeclaration =
                ASTQuery::findDeclarationByName(fwBlock->getName(), {}, tree,
                                                fwBlock->getNamespaceList(),
                                                frameworkName);
            if (fwDeclaration &&
                (fwDeclaration->getObjectType() == "_frameworkDescription")) {
              ASTNode frameworkName =
                  fwDeclaration->getPropertyValue("frameworkName");
              if (frameworkName &&
                  frameworkName->getNodeType() == AST::String) {
                if (domainFramework !=
                    static_cast<ValueNode *>(frameworkName.get())
                        ->getStringValue()) {
                  //                    qDebug() << "Inconsistent name";
                  continue;
                }
                return static_cast<ValueNode *>(frameworkName.get())
                    ->getStringValue();
              }
            }
          }
        }
      }
    }
  }
  return std::string();
}
