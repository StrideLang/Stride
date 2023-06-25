#include "codeanalysis.hpp"

#include "stride/parser/astfunctions.h"
#include "stride/parser/astquery.h"

#include <cassert>
#include <iostream>

std::vector<std::string> CodeAnalysis::getUsedDomains(ASTNode tree) {
  std::vector<std::string> domains;
  for (const ASTNode &node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      auto streamNode = std::static_pointer_cast<StreamNode>(node);
      auto childNode = streamNode->getLeft();
      auto nextNode = streamNode->getRight();
      while (childNode) {
        std::string domainName =
            CodeAnalysis::getNodeDomainName(childNode, {}, tree);
        if (domainName.size() > 0) {
          if (std::find(domains.begin(), domains.end(), domainName) ==
              domains.end()) {
            domains.push_back(domainName);
          }
        }
        if (!nextNode) {
          childNode = nullptr;
        } else if (nextNode->getNodeType() == AST::Stream) {
          streamNode = std::static_pointer_cast<StreamNode>(nextNode);
          childNode = streamNode->getLeft();
          nextNode = streamNode->getRight();
        } else {
          childNode = nextNode;
          nextNode = nullptr;
        }
      }
    } else if (node->getNodeType() == AST::Declaration ||
               node->getNodeType() == AST::BundleDeclaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      auto domainName =
          CodeAnalysis::getNodeDomainName(decl, ScopeStack(), tree);
      if (domainName.size() > 0) {
        if (std::find(domains.begin(), domains.end(), domainName) ==
            domains.end()) {
          domains.push_back(domainName);
        }
      }
    }
  }
  return domains;
}

std::vector<std::string> CodeAnalysis::getUsedFrameworks(ASTNode tree) {
  std::vector<std::string> domains = CodeAnalysis::getUsedDomains(tree);
  std::vector<std::string> usedFrameworks;
  for (const std::string &domain : domains) {
    usedFrameworks.push_back(CodeAnalysis::getFrameworkForDomain(domain, tree));
  }
  return usedFrameworks;
}

ASTNode CodeAnalysis::getInstance(ASTNode block, ScopeStack scopeStack,
                                  ASTNode tree) {
  ASTNode inst;
  if (block->getNodeType() == AST::List) {
    inst = block;
  } else if (block->getNodeType() == AST::Function) {
    inst = block;
  } else if (block->getNodeType() == AST::Declaration ||
             block->getNodeType() == AST::BundleDeclaration) {
    if (std::static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "signal" ||
        std::static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "switch" ||
        std::static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "trigger" ||
        std::static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "string" ||
        std::static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "constant") {
      inst = block;
    } else if (std::static_pointer_cast<DeclarationNode>(block)
                   ->getObjectType() == "reaction") {
      inst = block;
    } else {
      std::cout << __FILE__ << ":" << __LINE__
                << " Unexpected declaration in getInstance()";
    }
  } else if (block->getNodeType() == AST::Int ||
             block->getNodeType() == AST::Real ||
             block->getNodeType() == AST::String) {
    inst = block;
  } else {
    std::shared_ptr<DeclarationNode> decl =
        std::static_pointer_cast<DeclarationNode>(
            block->getCompilerProperty("declaration"));
    if (!decl) {
      decl = ASTQuery::findDeclarationByName(ASTQuery::getNodeName(block),
                                             scopeStack, tree);
    }
    if (decl) {
      auto typeDecl = ASTQuery::findTypeDeclaration(decl, scopeStack, tree);
      if (typeDecl && typeDecl->getObjectType() == "platformBlock") {
        inst = block;
      } else {
        inst = decl;
      }
    } else {
      inst = block;
    }
  }
  return inst;
}

std::shared_ptr<DeclarationNode> CodeAnalysis::getDeclaration(ASTNode node) {
  if (node->getNodeType() == AST::Int || node->getNodeType() == AST::Real ||
      node->getNodeType() == AST::String) {
    return nullptr;
  }
  return std::static_pointer_cast<DeclarationNode>(
      node->getCompilerProperty("declaration"));
}

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
      if (funcDecl) {
        if (funcDecl->getObjectType() == "platformModule") {
          domainNode = funcDecl->getPropertyValue("domain");

          // TODO move this namespace resolution for domains to be more general
          auto domainId =
              CodeAnalysis::getDomainIdentifier(domainNode, {}, tree);
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
        } else if (funcDecl->getObjectType() == "reaction") {
          // reactions currently belong to a single domain.
          domainNode = funcDecl->getPropertyValue("domain");
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

double CodeAnalysis::getNodeRate(ASTNode node, ScopeStack scope, ASTNode tree) {
  if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(name->getName(), scope, tree,
                                        name->getNamespaceList());
    if (declaration) {
      ASTNode rateNode = declaration->getPropertyValue("rate");
      if (rateNode) {
        std::vector<LangError> tempErrors;
        auto rate = CodeAnalysis::resolveRateToFloat(rateNode, scope, tree,
                                                     &tempErrors);
        if (tempErrors.size() == 0) {
          return rate;
        } else {
          return -1;
        }
      }
    }
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(bundle->getName(), scope, tree,
                                        bundle->getNamespaceList());
    if (declaration) {
      ASTNode rateNode = declaration->getPropertyValue("rate");
      if (rateNode) {
        std::vector<LangError> tempErrors;
        auto rate = CodeAnalysis::resolveRateToFloat(rateNode, scope, tree,
                                                     &tempErrors);
        if (tempErrors.size() == 0) {
          return rate;
        } else {
          return -1;
        }
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    FunctionNode *func = static_cast<FunctionNode *>(node.get());
    return func->getRate();
  } else if (node->getNodeType() == AST::List ||
             node->getNodeType() == AST::Expression) {
    double rate = -1.0;
    for (const ASTNode &element : node->getChildren()) {
      double elementRate = CodeAnalysis::getNodeRate(element, scope, tree);
      if (elementRate != -1.0) {
        if (rate != elementRate) {
          //                    qDebug() << "Warning: List has different
          //                    rates!";
        }
        rate = elementRate;
      }
    }
    return rate;
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    auto decl = std::static_pointer_cast<DeclarationNode>(node);
    if (decl) {
      ASTNode rateNode = decl->getPropertyValue("rate");
      if (rateNode) {
        std::vector<LangError> tempErrors;
        auto rate = CodeAnalysis::resolveRateToFloat(rateNode, scope, tree,
                                                     &tempErrors);
        if (tempErrors.size() == 0) {
          return rate;
        } else {
          return -1;
        }
      }
    }
  }
  return -1;
}

double CodeAnalysis::resolveRateToFloat(ASTNode rateNode, ScopeStack scope,
                                        ASTNode tree,
                                        std::vector<LangError> *errors) {
  double rate = -1;

  if (rateNode->getNodeType() == AST::Int) {
    rate = ASTFunctions::evaluateConstInteger(rateNode, {}, tree, errors);
    return rate;
  } else if (rateNode->getNodeType() == AST::Real) {
    rate = ASTFunctions::evaluateConstReal(rateNode, {}, tree, errors);
    return rate;
  } else if (rateNode->getNodeType() == AST::Block) {
    BlockNode *block = static_cast<BlockNode *>(rateNode.get());
    std::shared_ptr<DeclarationNode> valueDeclaration =
        ASTQuery::findDeclarationByName(block->getName(), scope, tree,
                                        block->getNamespaceList());
    if (valueDeclaration && valueDeclaration->getObjectType() == "constant") {
      std::shared_ptr<PropertyNode> property = ASTQuery::findPropertyByName(
          valueDeclaration->getProperties(), "value");
      if (property) {
        std::shared_ptr<ValueNode> rateNode =
            std::static_pointer_cast<ValueNode>(property->getValue());
        if (rateNode->getNodeType() == AST::Int) {
          rate = ASTFunctions::evaluateConstInteger(rateNode, {}, tree, errors);
        } else if (rateNode->getNodeType() == AST::Real) {
          rate = ASTFunctions::evaluateConstReal(rateNode, {}, tree, errors);
        }
      }
    }
    if (errors->size() == 0) {
      return rate;
    }
  } else if (rateNode->getNodeType() == AST::PortProperty) {
  }
  return rate;
}

double CodeAnalysis::getDomainDefaultRate(
    std::shared_ptr<DeclarationNode> domainDecl) {
  assert(domainDecl->getObjectType() == "_domainDefinition");
  auto ratePort = domainDecl->getPropertyValue("rate");
  if (ratePort->getNodeType() == AST::Real ||
      ratePort->getNodeType() == AST::Int) {
    double rate = std::static_pointer_cast<ValueNode>(ratePort)->toReal();
    return rate;
  }
  return -1;
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

int CodeAnalysis::numParallelStreams(StreamNode *stream, StrideSystem &platform,
                                     const ScopeStack &scope, ASTNode tree,
                                     std::vector<LangError> *errors) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();
  int numParallel = 0;

  int leftSize = 0;
  int rightSize = 0;

  if (left->getNodeType() == AST::Block) {
    leftSize = ASTQuery::getNodeSize(left, scope, tree);
  } else if (left->getNodeType() == AST::List) {
    leftSize = ASTQuery::getNodeSize(left, scope, tree);
  } else {
    leftSize = CodeAnalysis::getNodeNumOutputs(left, scope, tree, errors);
  }
  if (right->getNodeType() == AST::Block || right->getNodeType() == AST::List) {
    rightSize = ASTQuery::getNodeSize(right, scope, tree);
  } else if (right->getNodeType() == AST::Function) {
    int functionNodeSize = ASTQuery::getNodeSize(right, scope, tree);
    if (functionNodeSize == 1) {
      rightSize = leftSize;
    }
  } else if (right->getNodeType() == AST::Stream) {
    StreamNode *rightStream = static_cast<StreamNode *>(right.get());
    numParallel =
        numParallelStreams(rightStream, platform, scope, tree, errors);

    ASTNode firstMember = rightStream->getLeft();
    if (firstMember->getNodeType() == AST::Block ||
        firstMember->getNodeType() == AST::List) {
      rightSize = ASTQuery::getNodeSize(firstMember, scope, tree);
    } else {
      rightSize =
          CodeAnalysis::getNodeNumInputs(firstMember, scope, tree, errors);
    }
    if (firstMember->getNodeType() == AST::Function) {
      int functionNodeSize = ASTQuery::getNodeSize(firstMember, scope, tree);
      if (functionNodeSize == 1) {
        rightSize =
            CodeAnalysis::getNodeNumInputs(firstMember, scope, tree, errors);
        ;
      } else {
      }
    }
  } else {
    rightSize = CodeAnalysis::getNodeNumInputs(right, scope, tree, errors);
  }
  int thisParallel = -1;

  if (leftSize == rightSize ||
      (rightSize / (float)leftSize) == (int)(rightSize / (float)leftSize)) {
    if (leftSize == 0) {
      thisParallel = -1;
    }
    if (leftSize == rightSize) {
      thisParallel = leftSize;
    } else {
      thisParallel = leftSize / rightSize;
    }
  }
  if (leftSize == 1) {
    thisParallel = rightSize;
  } else if (rightSize == 1) {
    thisParallel = leftSize;
  }
  if (thisParallel != numParallel && numParallel > 0) {
    if (rightSize == 1)
      numParallel = -1;
  } else {
    numParallel = thisParallel;
  }

  return numParallel;
}

std::shared_ptr<DeclarationNode>
CodeAnalysis::resolveConnectionBlock(ASTNode node, ScopeStack scopeStack,
                                     ASTNode tree, bool downStream) {
  if (!node) {
    return nullptr;
  }
  if (node->getNodeType() == AST::Declaration) { // Signal
    return std::static_pointer_cast<DeclarationNode>(node);
  } else if (node->getNodeType() == AST::Function) {
    auto funcDecl = std::static_pointer_cast<DeclarationNode>(
        node->getCompilerProperty("declaration"));
    if (funcDecl) {
      auto ports = funcDecl->getPropertyValue("ports");
      auto portDeclBlock = ASTQuery::getModuleMainOutputPortBlock(funcDecl);
      if (portDeclBlock) {
        auto portBlock = portDeclBlock->getPropertyValue("block");
        if (portBlock) {
          auto funcName = funcDecl->getName();
          auto portBlockDecl = ASTQuery::findDeclarationByName(
              ASTQuery::getNodeName(portBlock),
              {{node, std::static_pointer_cast<DeclarationNode>(funcDecl)
                          ->getPropertyValue("blocks")
                          ->getChildren()}},
              nullptr);
          if (portBlockDecl) {
            auto blockDomain =
                CodeAnalysis::getNodeDomain(portBlockDecl, {}, nullptr);
            if (blockDomain &&
                blockDomain->getNodeType() == AST::PortProperty) {
              auto portName =
                  std::static_pointer_cast<PortPropertyNode>(blockDomain)
                      ->getName();
              assert(std::static_pointer_cast<PortPropertyNode>(blockDomain)
                         ->getPortName() == "domain");
              // Now match the port name from the domain to the port
              // declaration
              for (const auto &port : ports->getChildren()) {
                if (port->getNodeType() == AST::Declaration) {
                  auto portDecl =
                      std::static_pointer_cast<DeclarationNode>(port);
                  if (portDecl->getName() ==
                      portName) { // Port match. Now get outer node
                    auto portType =
                        std::static_pointer_cast<DeclarationNode>(port)
                            ->getObjectType();
                    // TODO connect all port types
                    if (portType == "mainOutputPort" && downStream) {
                      auto outerBlock =
                          node->getCompilerProperty("outputBlock");
                      if (outerBlock->getNodeType() == AST::Block ||
                          outerBlock->getNodeType() == AST::Bundle) {
                        auto decl = ASTQuery::findDeclarationByName(
                            ASTQuery::getNodeName(outerBlock), scopeStack,
                            tree);
                        if (decl) {
                          return decl;
                        } else {
                          return std::shared_ptr<DeclarationNode>();
                        }
                      }
                      return resolveConnectionBlock(outerBlock, scopeStack,
                                                    tree, true);
                    } else if (portType == "mainInputPort" && !downStream) {
                      auto outerBlock = node->getCompilerProperty("inputBlock");
                      return resolveConnectionBlock(outerBlock, scopeStack,
                                                    tree, false);
                    }
                  }
                }
              }
            } else {
              std::cerr << "Unexpected port block domain" << std::endl;
            }
          }
        }
      }
    }
  }
  return std::shared_ptr<DeclarationNode>();
}

ASTNode CodeAnalysis::getMatchedOuterInstance(
    std::shared_ptr<FunctionNode> functionNode,
    std::shared_ptr<DeclarationNode> blockDecl,
    std::shared_ptr<DeclarationNode> funcDecl, ScopeStack scopeStack,
    ASTNode tree) {
  // Instance is declaration for block, but stream node for function
  ASTNode matchedInst = nullptr;
  auto internalBlocks = funcDecl->getPropertyValue("blocks")->getChildren();
  for (const auto &port : funcDecl->getPropertyValue("ports")->getChildren()) {
    if (port->getNodeType() == AST::Declaration) {
      auto portDecl = std::static_pointer_cast<DeclarationNode>(port);

      auto portBlock = std::static_pointer_cast<BlockNode>(
          portDecl->getPropertyValue("block"));

      // Search only within internal scope for port block
      auto portBlockDecl = ASTQuery::findDeclarationByName(
          ASTQuery::getNodeName(portBlock), {{functionNode, internalBlocks}},
          nullptr);

      if (portBlockDecl && portBlockDecl->getName() == blockDecl->getName()) {
        // If it's a main port, we can get the external block from the
        // compiler properties of the FunctionNode
        if (portDecl->getObjectType() == "mainOutputPort") {
          ASTNode extOutputBlockInst;
          auto outputBlock = functionNode->getCompilerProperty("outputBlock");
          if (outputBlock) {
            extOutputBlockInst =
                CodeAnalysis::getInstance(outputBlock, scopeStack, tree);
          }
          matchedInst = extOutputBlockInst;
          break;
        } else if (portDecl->getObjectType() == "mainInputPort") {
          ASTNode extInputBlockInst;
          auto inputBlock = functionNode->getCompilerProperty("inputBlock");
          if (inputBlock) {
            extInputBlockInst =
                CodeAnalysis::getInstance(inputBlock, scopeStack, tree);
          }
          matchedInst = extInputBlockInst;
          break;
        } else {
          // If it's a secondary port, we get the block from the FunctionNode
          // properties
          auto propertyName = std::static_pointer_cast<ValueNode>(
              portDecl->getPropertyValue("name"));
          if (propertyName->getNodeType() == AST::String) {
            auto outerBlock =
                functionNode->getPropertyValue(propertyName->toString());
            if (outerBlock) {
              if (outerBlock->getNodeType() == AST::Block ||
                  outerBlock->getNodeType() == AST::Bundle) {
                matchedInst = ASTQuery::findDeclarationByName(
                    ASTQuery::getNodeName(outerBlock), scopeStack, tree);
                break;
              } else if (outerBlock->getNodeType() == AST::Function ||
                         outerBlock->getNodeType() == AST::Expression ||
                         outerBlock->getNodeType() == AST::List) {
                matchedInst = outerBlock;
                break;
              } else if (outerBlock->getNodeType() == AST::Int ||
                         outerBlock->getNodeType() == AST::Real ||
                         outerBlock->getNodeType() == AST::Switch) {
                matchedInst = outerBlock;
                break;
              }
            }
          }
        }
      }
    }
  }
  return matchedInst;
}

std::vector<ASTNode> CodeAnalysis::getBlocksInScope(ASTNode root,
                                                    ScopeStack scopeStack,
                                                    ASTNode tree) {
  std::vector<ASTNode> blocks;
  if (root->getNodeType() == AST::Declaration ||
      root->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> decl =
        std::static_pointer_cast<DeclarationNode>(root);

    if (decl->getPropertyValue("ports")) {
      for (const ASTNode &block :
           decl->getPropertyValue("ports")->getChildren()) {
        blocks.push_back(block);
      }
    }
  } else if (root->getNodeType() == AST::List) {
    std::vector<ASTNode> elements =
        std::static_pointer_cast<ListNode>(root)->getChildren();
    for (const ASTNode &element : elements) {
      auto newBlocks = getBlocksInScope(element, scopeStack, tree);
      blocks.insert(blocks.end(), newBlocks.begin(), newBlocks.end());
    }
  } else if (root->getNodeType() == AST::Block) {
    std::shared_ptr<BlockNode> name = std::static_pointer_cast<BlockNode>(root);
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(name->getName(), scopeStack, tree);
    if (declaration) {
      blocks = getBlocksInScope(declaration, scopeStack, tree);
    }
  } else if (root->getNodeType() == AST::Bundle) {
    std::shared_ptr<BundleNode> name =
        std::static_pointer_cast<BundleNode>(root);
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(name->getName(), scopeStack, tree);
    if (declaration) {
      blocks = getBlocksInScope(declaration, scopeStack, tree);
    }
  } else {
    for (const ASTNode &child : root->getChildren()) {
      auto newBlocks = getBlocksInScope(child, scopeStack, tree);
      blocks.insert(blocks.end(), newBlocks.begin(), newBlocks.end());
    }
  }
  return blocks;
}

int CodeAnalysis::evaluateSizePortProperty(
    std::string targetPortName, ScopeStack scopeStack,
    std::shared_ptr<DeclarationNode> decl, std::shared_ptr<FunctionNode> func,
    ASTNode tree) {
  auto portsNode = decl->getPropertyValue("ports");
  auto blocksNode = decl->getPropertyValue("blocks");
  int portSize = 0;
  if (portsNode && blocksNode) {
    std::vector<ASTNode> ports = portsNode->getChildren();
    std::vector<ASTNode> blocks = blocksNode->getChildren();
    for (const auto &port : ports) {
      if (port->getNodeType() == AST::Declaration) {
        auto portDecl = std::static_pointer_cast<DeclarationNode>(port);
        auto portName = portDecl->getName();
        //                std::string portDomainName = portName + "_domain";
        //        scopeStack.push_back({func, blocks});
        if (portName == targetPortName) {
          auto portBlock = portDecl->getPropertyValue("block");
          auto portNameNode = portDecl->getPropertyValue("name");
          std::string portName;

          if (portNameNode && portNameNode->getNodeType() == AST::String) {
            portName = std::static_pointer_cast<ValueNode>(portNameNode)
                           ->getStringValue();
          }
          std::string portBlockName;
          if (portBlock && (portBlock->getNodeType() == AST::Block ||
                            portBlock->getNodeType() == AST::Bundle)) {
            if (portDecl->getObjectType() == "mainInputPort") {
              portSize =
                  CodeAnalysis::getNodeNumInputs(portBlock, scopeStack, tree);
              if (portSize == -2) {
                std::pair<ASTNode, std::vector<ASTNode>> innerScope =
                    scopeStack.back();
                scopeStack.pop_back();
                // We need to look one scope up.
                portSize = CodeAnalysis::getNodeNumOutputs(
                    func->getCompilerProperty("inputBlock"), scopeStack, tree);
                scopeStack.push_back(innerScope);
              }

            } else if (portDecl->getObjectType() == "mainOutputPort") {
              portSize =
                  CodeAnalysis::getNodeNumOutputs(portBlock, scopeStack, tree);
              if (portSize == CodeAnalysis::SIZE_PORT_PROPERTY) {
                std::pair<ASTNode, std::vector<ASTNode>> innerScope =
                    scopeStack.back();
                scopeStack.pop_back();
                // We need to look one scope up.
                portSize = CodeAnalysis::getNodeNumOutputs(
                    func->getPropertyValue(portName), scopeStack, tree);
                scopeStack.push_back(innerScope);
              }
            } else if (portDecl->getObjectType() == "propertyInputPort") {
              portSize =
                  CodeAnalysis::getNodeNumInputs(portBlock, scopeStack, tree);
              if (portSize == CodeAnalysis::SIZE_PORT_PROPERTY) {
                std::pair<ASTNode, std::vector<ASTNode>> innerScope =
                    scopeStack.back();
                scopeStack.pop_back();
                // We need to look one scope up.
                portSize = CodeAnalysis::getNodeNumOutputs(
                    func->getPropertyValue(portName), scopeStack, tree);
                scopeStack.push_back(innerScope);
              }

            } else if (portDecl->getObjectType() == "propertyOutputPort") {
              portSize =
                  CodeAnalysis::getNodeNumOutputs(portBlock, scopeStack, tree);

              if (portSize == CodeAnalysis::SIZE_PORT_PROPERTY) {
                std::pair<ASTNode, std::vector<ASTNode>> innerScope =
                    scopeStack.back();
                scopeStack.pop_back();
                // We need to look one scope up.
                portSize = CodeAnalysis::getNodeNumOutputs(
                    func->getPropertyValue(portName), scopeStack, tree);
                scopeStack.push_back(innerScope);
              }
            }
          } else {
            std::cerr << "Unexpected port block entry" << std::endl;
          }
        }
      }
    }
  }
  return portSize;
}

double CodeAnalysis::evaluateRatePortProperty(
    std::string targetPortName, ScopeStack scopeStack,
    std::shared_ptr<DeclarationNode> decl, std::shared_ptr<FunctionNode> func,
    ASTNode tree) {
  auto portsNode = decl->getPropertyValue("ports");
  auto blocksNode = decl->getPropertyValue("blocks");

  std::vector<ASTNode> ports = portsNode->getChildren();
  std::vector<ASTNode> blocks = blocksNode->getChildren();
  for (const auto &port : ports) {
    if (port->getNodeType() == AST::Declaration) {
      auto portDecl = std::static_pointer_cast<DeclarationNode>(port);
      auto portName = portDecl->getName();
      //                std::string portDomainName = portName + "_domain";
      //      scopeStack.push_back({decl->getName(), blocks});
      if (portName == targetPortName) {
        auto portBlock = portDecl->getPropertyValue("block");
        auto portNameNode = portDecl->getPropertyValue("name");
        std::string portName;

        if (portNameNode && portNameNode->getNodeType() == AST::String) {
          // For property input ports that need a name. Name will be empty for
          // main ports
          portName = std::static_pointer_cast<ValueNode>(portNameNode)
                         ->getStringValue();
        }
        std::string portBlockName;
        if (portBlock && (portBlock->getNodeType() == AST::Block ||
                          portBlock->getNodeType() == AST::Bundle)) {
          if (portDecl->getObjectType() == "mainInputPort") {
            double rate =
                CodeAnalysis::resolveRate(portBlock, scopeStack, tree, false);
            if (rate == -1) {
              auto resolvedBlock = CodeAnalysis::getInstance(
                  func->getCompilerProperty("inputBlock"), scopeStack, tree);
              auto scopeStackOuter = scopeStack;
              for (const auto &subScope : scopeStack) {
                rate = CodeAnalysis::resolveRate(resolvedBlock, scopeStack,
                                                 tree, true);
                if (rate != -1) {
                  return rate;
                }
                if (subScope.first->getNodeType() != AST::Function) {
                  // If no longer a module instance (i.e. AST::Function), the
                  // rate cannot be resolved
                  break;
                }
                auto functionNode =
                    std::static_pointer_cast<FunctionNode>(subScope.first);
                // TODO check framework too
                std::string framework;
                auto funcDecl = ASTQuery::findDeclarationByName(
                    functionNode->getName(), scopeStackOuter, tree,
                    functionNode->getNamespaceList(), framework);
                if (resolvedBlock->getNodeType() == AST::Declaration ||
                    resolvedBlock->getNodeType() == AST::BundleDeclaration) {
                  resolvedBlock = getMatchedOuterInstance(
                      functionNode,
                      std::static_pointer_cast<DeclarationNode>(resolvedBlock),
                      funcDecl, scopeStackOuter, tree);
                  scopeStackOuter.pop_back();
                }
              }
            }
            return rate;

          } else if (portDecl->getObjectType() == "mainOutputPort") {
            double rate =
                CodeAnalysis::resolveRate(portBlock, scopeStack, tree, false);
            if (rate == -1) {
              auto resolvedBlock = CodeAnalysis::getInstance(
                  func->getCompilerProperty("outputBlock"), scopeStack, tree);
              auto scopeStackOuter = scopeStack;
              for (const auto &subScope : scopeStack) {
                rate = CodeAnalysis::resolveRate(resolvedBlock, scopeStack,
                                                 tree, true);
                if (rate != -1) {
                  return rate;
                }
                if (!subScope.first ||
                    subScope.first->getNodeType() != AST::Function) {
                  // If no longer a module instance (i.e. AST::Function), the
                  // rate cannot be resolved
                  break;
                }
                auto functionNode =
                    std::static_pointer_cast<FunctionNode>(subScope.first);
                // TODO check framework too
                std::string framework;
                auto funcDecl = ASTQuery::findDeclarationByName(
                    functionNode->getName(), scopeStackOuter, tree,
                    functionNode->getNamespaceList(), framework);
                if (resolvedBlock->getNodeType() == AST::Declaration ||
                    resolvedBlock->getNodeType() == AST::BundleDeclaration) {
                  resolvedBlock = getMatchedOuterInstance(
                      functionNode,
                      std::static_pointer_cast<DeclarationNode>(resolvedBlock),
                      funcDecl, scopeStackOuter, tree);
                  scopeStackOuter.pop_back();
                }
              }
            }
            return rate;

          } else if (portDecl->getObjectType() == "propertyInputPort" ||
                     portDecl->getObjectType() == "propertyOutputPort") {
            double rate =
                CodeAnalysis::resolveRate(portBlock, scopeStack, tree, false);
            if (rate == -1) {
              auto resolvedBlock =
                  CodeAnalysis::getInstance(portBlock, scopeStack, tree);
              auto scopeStackOuter = scopeStack;
              for (const auto &subScope : scopeStack) {
                rate = CodeAnalysis::resolveRate(resolvedBlock, scopeStack,
                                                 tree, true);
                if (rate != -1) {
                  return rate;
                }
                if (subScope.first->getNodeType() != AST::Function) {
                  // If no longer a module instance (i.e. AST::Function), the
                  // rate cannot be resolved
                  break;
                }
                auto functionNode =
                    std::static_pointer_cast<FunctionNode>(subScope.first);
                // TODO check framework too
                std::string framework;
                auto funcDecl = ASTQuery::findDeclarationByName(
                    functionNode->getName(), scopeStackOuter, tree,
                    functionNode->getNamespaceList(), framework);
                if (resolvedBlock->getNodeType() == AST::Declaration ||
                    resolvedBlock->getNodeType() == AST::BundleDeclaration) {
                  resolvedBlock = getMatchedOuterInstance(
                      functionNode,
                      std::static_pointer_cast<DeclarationNode>(resolvedBlock),
                      funcDecl, scopeStackOuter, tree);
                  scopeStackOuter.pop_back();
                }
              }
            }
            return rate;
          }
        } else {
          std::cerr << "Unexpected port block entry" << std::endl;
        }
      }
    }
  }

  //  for (auto port : portsNode->getChildren()) {
  //    if (port->getNodeType() == AST::Declaration) {
  //      auto portDecl = static_pointer_cast<DeclarationNode>(port);
  //      auto portName = portDecl->getName();
  //      std::string portDomainName = portName + "_domain";
  //      if (portName == targetPortName) {
  //        auto portBlock = portDecl->getPropertyValue("block");
  //        std::string portBlockName;
  //        if (portBlock && (portBlock->getNodeType() == AST::Block ||
  //                          portBlock->getNodeType() == AST::Bundle)) {
  //          portBlockName = CodeValidator::streamMemberName(portBlock);
  //        }
  //        if (portDecl->getObjectType() == "mainInputPort" ||
  //            portDecl->getObjectType() == "propertyInputPort") {
  //          if (blockInMap.find(portDomainName) != blockInMap.end()) {
  //            for (auto mapping : blockInMap[portDomainName]) {
  //              if (mapping.externalConnection &&
  //                  mapping.internalDecl->getName() == portBlockName) {
  //                if (std::find(usedPortPropertyName.begin(),
  //                              usedPortPropertyName.end(),
  //                              std::pair<std::string, std::string>{
  //                                  portProperty->getName(),
  //                                  portProperty->getPortName()}) ==
  //                    usedPortPropertyName.end()) {
  //                  auto rate = CodeAnalysis::resolveRate(
  //                      mapping.externalConnection, scopeStack, tree,
  //                      false);

  //                  auto contextStack = scopeStack;
  //                  contextStack.pop_back();
  //                  auto inMapStackIt = m_inMapStack.rbegin();
  //                  auto outMapStackIt = m_outMapStack.rbegin();
  //                  assert(m_inMapStack.size() == m_outMapStack.size());
  //                  while (outMapStackIt != m_outMapStack.rend()) {
  //                    auto &outMap = *outMapStackIt;
  //                    auto &inMap = *inMapStackIt;
  //                    for (auto entry : outMap) {
  //                      for (auto outerMapping : entry.second) {
  //                        if (outerMapping.internalDecl ==
  //                            mapping.externalConnection) {
  //                          // Pointers should match. Do we need to
  //                          // make a further check?
  //                          rate = CodeAnalysis::resolveRate(
  //                              outerMapping.externalConnection, scopeStack,
  //                              tree, true);
  //                          break;
  //                        }
  //                      }
  //                    }
  //                    for (auto entry : inMap) {
  //                      for (auto outerMapping : entry.second) {
  //                        if (outerMapping.internalDecl ==
  //                            mapping.externalConnection) {
  //                          // Pointers should match. Do we need to
  //                          // make a further check?
  //                          rate = CodeAnalysis::resolveRate(
  //                              outerMapping.externalConnection, scopeStack,
  //                              tree, true);
  //                          break;
  //                        }
  //                      }
  //                    }
  //                    contextStack.pop_back();
  //                    inMapStackIt++;
  //                    outMapStackIt++;
  //                  }
  //                  return rate;
  //                }
  //              }
  //            }
  //          }

  //        } else if (portDecl->getObjectType() == "mainOutputPort" ||
  //                   portDecl->getObjectType() ==
  //                       "propertyOutputPort") {  // Output port so
  //          // look in blockOutMap
  //          if (blockOutMap.find(portDomainName) != blockOutMap.end()) {
  //            for (auto mapping : blockOutMap[portDomainName]) {
  //              if (mapping.externalConnection &&
  //                  mapping.internalDecl->getName() == portBlockName) {
  //                auto rate = CodeAnalysis::resolveRate(
  //                    mapping.externalConnection, scopeStack, m_tree, true);
  //                auto inMapStackIt = m_inMapStack.rbegin();
  //                auto outMapStackIt = m_outMapStack.rbegin();
  //                assert(m_inMapStack.size() == m_outMapStack.size());
  //                while (outMapStackIt != m_outMapStack.rend()) {
  //                  auto &outMap = *outMapStackIt;
  //                  auto &inMap = *inMapStackIt;
  //                  for (auto entry : outMap) {
  //                    for (auto outerMapping : entry.second) {
  //                      if (outerMapping.internalDecl ==
  //                          mapping.externalConnection) {
  //                        // Pointers should match. Do we need to
  //                        // make a further check?
  //                        if (outerMapping.externalConnection) {
  //                          rate = CodeAnalysis::resolveRate(
  //                              outerMapping.externalConnection, scopeStack,
  //                              tree, true);
  //                        }
  //                        break;
  //                      }
  //                    }
  //                  }
  //                  for (auto entry : inMap) {
  //                    for (auto outerMapping : entry.second) {
  //                      if (outerMapping.internalDecl ==
  //                          mapping.externalConnection) {
  //                        // Pointers should match. Do we need to
  //                        // make a further check?
  //                        if (outerMapping.externalConnection) {
  //                          rate = CodeAnalysis::resolveRate(
  //                              outerMapping.externalConnection, scopeStack,
  //                              tree, true);
  //                        }
  //                        break;
  //                      }
  //                    }
  //                  }
  //                  inMapStackIt++;
  //                  outMapStackIt++;
  //                }
  //                return rate;
  //              }
  //            }
  //          }
  //        }
  //      }
  //    }
  //  }
  return -1.0;
}

std::vector<std::shared_ptr<PortPropertyNode>>
CodeAnalysis::getUsedPortProperties(std::shared_ptr<DeclarationNode> funcDecl) {
  std::vector<std::shared_ptr<PortPropertyNode>> used;

  auto blocksNode = funcDecl->getPropertyValue("blocks");
  if (blocksNode) {
    for (const auto &node : blocksNode->getChildren()) {
      if (node->getNodeType() == AST::Declaration) {
        auto decl = std::static_pointer_cast<DeclarationNode>(node);
        auto newUsed = CodeAnalysis::getUsedPortProperties(decl);
        used.insert(used.end(), newUsed.begin(), newUsed.end());
      }
    }
  }

  auto streams = funcDecl->getPropertyValue("streams");
  if (streams) {
    for (const auto &streamNode : streams->getChildren()) {
      if (streamNode->getNodeType() == AST::Stream) {
        auto stream = std::static_pointer_cast<StreamNode>(streamNode);
        ASTNode node = stream->getLeft();
        ASTNode next = stream->getRight();
        do {
          auto newUsed = getUsedPortPropertiesInNode(node);
          used.insert(used.end(), newUsed.begin(), newUsed.end());

          if (next && next->getNodeType() == AST::Stream) {
            node = std::static_pointer_cast<StreamNode>(next)->getLeft();
            next = std::static_pointer_cast<StreamNode>(next)->getRight();
          } else if (next) {
            node = next;
            next = nullptr;
          } else {
            node = nullptr;
          }
        } while (node);
      }
    }
  }

  return used;
}

std::vector<std::shared_ptr<PortPropertyNode>>
CodeAnalysis::getUsedPortPropertiesInNode(ASTNode node) {

  std::vector<std::shared_ptr<PortPropertyNode>> used;
  if (node->getNodeType() == AST::PortProperty) {
    used.push_back(std::static_pointer_cast<PortPropertyNode>(node));
  } else if (node->getNodeType() == AST::List ||
             node->getNodeType() == AST::Expression) {
    for (const auto &elem : node->getChildren()) {
      auto newUsed = getUsedPortPropertiesInNode(elem);
      used.insert(used.end(), newUsed.begin(), newUsed.end());
    }
  }
  return used;
}

ASTNode CodeAnalysis::resolveDomain(ASTNode node, ScopeStack scopeStack,
                                    ASTNode tree, bool downStream) {
  auto blockDecl =
      CodeAnalysis::resolveConnectionBlock(node, scopeStack, tree, downStream);
  if (blockDecl) {
    return CodeAnalysis::getNodeDomain(blockDecl, scopeStack, tree);
  } else {
    return CodeAnalysis::getNodeDomain(node, scopeStack, tree);
  }
}

double CodeAnalysis::resolveRate(ASTNode node, ScopeStack scopeStack,
                                 ASTNode tree, bool downStream) {
  auto blockDecl =
      CodeAnalysis::resolveConnectionBlock(node, scopeStack, tree, downStream);
  if (blockDecl) {
    return CodeAnalysis::getNodeRate(blockDecl, scopeStack, tree);
  } else if (node) {
    return CodeAnalysis::getNodeRate(node, scopeStack, tree);
  } else {
    return -1;
  }
}

int CodeAnalysis::getNodeNumOutputs(ASTNode node, const ScopeStack &scope,
                                    ASTNode tree,
                                    std::vector<LangError> *errors) {
  if (node->getNodeType() == AST::List) {
    int size = 0;
    for (const ASTNode &member : node->getChildren()) {
      auto newSize =
          CodeAnalysis::getNodeNumOutputs(member, scope, tree, errors);
      if (newSize >= 0) {
        size += newSize;
      } else {
        assert(0 == 1); // FIXME implement
      }
    }
    return size;
  } else if (node->getNodeType() == AST::Bundle) {
    return ASTQuery::getBundleSize(static_cast<BundleNode *>(node.get()), scope,
                                   tree, errors);
  } else if (node->getNodeType() == AST::Int ||
             node->getNodeType() == AST::Real ||
             node->getNodeType() == AST::String ||
             node->getNodeType() == AST::Switch) {
    return 1;
  } else if (node->getNodeType() == AST::Expression) {
    auto expr = std::static_pointer_cast<ExpressionNode>(node);
    if (expr->isUnary()) {
      return getNodeNumOutputs(expr->getValue(), scope, tree, errors);
    } else {
      auto leftSize = getNodeNumOutputs(expr->getLeft(), scope, tree, errors);
      auto rightSize = getNodeNumOutputs(expr->getRight(), scope, tree, errors);
      auto size = std::max(leftSize, rightSize);
      return size;
    }
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> block =
        ASTQuery::findDeclarationByName(name->getName(), scope, tree);
    if (block) {
      return getTypeNumOutputs(block, scope, tree, errors);
    } else {
      return CodeAnalysis::SIZE_UNKNOWN;
    }
  } else if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        std::static_pointer_cast<FunctionNode>(node);
    std::shared_ptr<DeclarationNode> platformFunc =
        ASTQuery::findDeclarationByName(func->getName(), scope, tree);
    int dataSize = CodeAnalysis::getFunctionDataSize(func, scope, tree, errors);
    if (platformFunc) {
      return getTypeNumOutputs(platformFunc, scope, tree, errors) * dataSize;
    } else {
      return CodeAnalysis::SIZE_UNKNOWN;
    }
  } else if (node->getNodeType() == AST::PortProperty) {
    std::shared_ptr<PortPropertyNode> portProp =
        std::static_pointer_cast<PortPropertyNode>(node);
    if (portProp->getPortName() == "size" ||
        portProp->getPortName() == "rate" ||
        portProp->getPortName() == "domain") {
      return CodeAnalysis::SIZE_PORT_PROPERTY;
    } else {
      std::cerr
          << "Unknown port property in getNodeNumOutputs() setting size to 1"
          << std::endl;
      return 1;
    }
  } else if (node->getNodeType() == AST::BundleDeclaration) {
    return ASTQuery::getBlockDeclaredSize(
        std::static_pointer_cast<DeclarationNode>(node), scope, tree, errors);
  } else if (node->getNodeType() == AST::BundleDeclaration) {
    return ASTQuery::getBlockDeclaredSize(
        std::static_pointer_cast<DeclarationNode>(node), scope, tree, errors);
  }
  return CodeAnalysis::SIZE_UNKNOWN;
}

int CodeAnalysis::getNodeNumInputs(ASTNode node, ScopeStack scope, ASTNode tree,
                                   std::vector<LangError> *errors) {
  if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        std::static_pointer_cast<FunctionNode>(node);
    std::shared_ptr<DeclarationNode> platformFunc =
        ASTQuery::findDeclarationByName(func->getName(), scope, tree);
    int dataSize = CodeAnalysis::getFunctionDataSize(func, scope, tree, errors);
    if (platformFunc) {
      if (platformFunc->getObjectType() == "reaction") {
        return 1; // Reactions always have one input as main port for trigger
      } else {
        auto subScope = ASTQuery::getModuleBlocks(platformFunc);
        scope.push_back({func, subScope});

        int numInputs = getTypeNumInputs(platformFunc, scope, tree, errors);
        if (numInputs < 0) {
          return numInputs;
        } else {
          return numInputs * dataSize;
        }
      }
    } else {
      return -1;
    }
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *stream = static_cast<StreamNode *>(node.get());
    ASTNode left = stream->getLeft();
    int leftSize = CodeAnalysis::getNodeNumInputs(left, scope, tree, errors);
    return leftSize;
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> block =
        ASTQuery::findDeclarationByName(name->getName(), scope, tree);
    if (block) {
      return getTypeNumInputs(block, scope, tree, errors);
    } else {
      return CodeAnalysis::SIZE_UNKNOWN;
    }
  } else if (node->getNodeType() == AST::Bundle) {
    return ASTQuery::getBundleSize(static_cast<BundleNode *>(node.get()), scope,
                                   tree, errors);
  } else if (node->getNodeType() == AST::List) {
    int size = 0;
    for (const ASTNode &member : node->getChildren()) {
      size += CodeAnalysis::getNodeNumInputs(member, scope, tree, errors);
    }
    return size;
  } else if (node->getNodeType() == AST::PortProperty) {
    std::cerr << "Unexpected write to port portperty" << std::endl;
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    return getTypeNumInputs(std::static_pointer_cast<DeclarationNode>(node),
                            scope, tree, errors);
  } else {
    return 0;
  }
  return CodeAnalysis::SIZE_UNKNOWN;
}

int CodeAnalysis::getTypeNumOutputs(
    std::shared_ptr<DeclarationNode> blockDeclaration, const ScopeStack &scope,
    ASTNode tree, std::vector<LangError> *errors) {
  if (blockDeclaration->getNodeType() == AST::BundleDeclaration) {
    return ASTQuery::getBlockDeclaredSize(blockDeclaration, scope, tree,
                                          errors);
  } else if (blockDeclaration->getNodeType() == AST::Declaration) {
    if (blockDeclaration->getObjectType() == "module") {
      std::shared_ptr<DeclarationNode> portBlock =
          ASTQuery::getModuleMainOutputPortBlock(blockDeclaration);

      BlockNode *outputName = nullptr;
      if (portBlock) {
        if (portBlock->getPropertyValue("block")->getNodeType() == AST::Block) {
          outputName = static_cast<BlockNode *>(
              portBlock->getPropertyValue("block").get());
        } else {
          std::cerr << "WARNING: Expecting name node for output block"
                    << std::endl;
        }
      }
      if (!outputName || outputName->getNodeType() == AST::None) {
        return 0;
      }
      auto subScope = ASTQuery::getModuleBlocks(
          std::static_pointer_cast<DeclarationNode>(blockDeclaration));
      if (subScope.size() > 0) {
        assert(outputName->getNodeType() == AST::Block);
        std::string outputBlockName = outputName->getName();
        for (const ASTNode &internalDeclarationNode : subScope) {
          if (internalDeclarationNode->getNodeType() ==
                  AST::BundleDeclaration ||
              internalDeclarationNode->getNodeType() == AST::Declaration) {
            std::string blockName =
                static_cast<DeclarationNode *>(internalDeclarationNode.get())
                    ->getName();
            if (blockName == outputBlockName) {
              std::shared_ptr<DeclarationNode> intBlock =
                  std::static_pointer_cast<DeclarationNode>(
                      internalDeclarationNode);
              if (intBlock->getName() == outputBlockName) {
                if (internalDeclarationNode->getNodeType() ==
                    AST::BundleDeclaration) {
                  return ASTQuery::getBlockDeclaredSize(intBlock, scope, tree,
                                                        errors);
                } else if (internalDeclarationNode->getNodeType() ==
                           AST::Declaration) {
                  return 1;
                }
              }
            }
          }
        }
      }
    } else if (blockDeclaration->getObjectType() == "platformModule") {
      auto outputList = blockDeclaration->getPropertyValue("outputs");
      if (outputList && outputList->getNodeType() == AST::List) {
        return outputList->getChildren().size();
      }
    } else if (blockDeclaration->getObjectType() == "buffer") {
      auto sizeNode = blockDeclaration->getPropertyValue("size");
      if (sizeNode->getNodeType() == AST::Int) {
        return std::static_pointer_cast<ValueNode>(sizeNode)->getIntValue();
      }
    }
    return 1;
  }
  return 0;
}

int CodeAnalysis::getTypeNumInputs(
    std::shared_ptr<DeclarationNode> blockDeclaration, const ScopeStack &scope,
    ASTNode tree, std::vector<LangError> *errors) {
  if (blockDeclaration->getNodeType() == AST::BundleDeclaration) {
    return ASTQuery::getBlockDeclaredSize(blockDeclaration, scope, tree,
                                          errors);
  } else if (blockDeclaration->getNodeType() == AST::Declaration) {
    if (blockDeclaration->getObjectType() == "module" ||
        blockDeclaration->getObjectType() == "loop") {
      auto subScope = ASTQuery::getModuleBlocks(
          std::static_pointer_cast<DeclarationNode>(blockDeclaration));
      if (subScope.size() > 0) {
        BlockNode *inputName = nullptr;

        std::shared_ptr<DeclarationNode> portBlock =
            ASTQuery::getModuleMainInputPortBlock(blockDeclaration);
        if (portBlock) {
          if (portBlock->getPropertyValue("block")->getNodeType() ==
              AST::Block) {
            inputName = static_cast<BlockNode *>(
                portBlock->getPropertyValue("block").get());
          } else {
            std::cerr << "WARNING: Expecting name node for input block"
                      << std::endl;
          }
        }
        if (!inputName || inputName->getNodeType() == AST::None) {
          return 0;
        }
        assert(inputName->getNodeType() == AST::Block);
        std::string inputBlockName = inputName->getName();
        for (const ASTNode &internalDeclarationNode : subScope) {
          //                assert(internalDeclarationNode->getNodeType() ==
          //                AST::BundleDeclaration ||
          //                internalDeclarationNode->getNodeType() ==
          //                AST::Declaration);
          if (!internalDeclarationNode) {
            return CodeAnalysis::SIZE_UNKNOWN;
          }
          if (internalDeclarationNode->getNodeType() == AST::Declaration ||
              internalDeclarationNode->getNodeType() ==
                  AST::BundleDeclaration) {
            std::string blockName =
                static_cast<DeclarationNode *>(internalDeclarationNode.get())
                    ->getName();
            if (blockName == inputBlockName) {
              if (internalDeclarationNode->getNodeType() ==
                  AST::BundleDeclaration) {
                std::shared_ptr<DeclarationNode> intBlock =
                    std::static_pointer_cast<DeclarationNode>(
                        internalDeclarationNode);
                assert(intBlock->getNodeType() == AST::BundleDeclaration);
                return ASTQuery::getBlockDeclaredSize(intBlock, scope, tree,
                                                      errors);
              } else if (internalDeclarationNode->getNodeType() ==
                         AST::Declaration) {
                return 1;
              }
            }
          }
          //                return -1; // Should never get here!
        }
      }
    } else if (blockDeclaration->getObjectType() == "platformModule") {
      auto outputList = blockDeclaration->getPropertyValue("inputs");
      if (outputList && outputList->getNodeType() == AST::List) {
        return outputList->getChildren().size();
      }
    } else {
      int size = ASTQuery::getNodeSize(blockDeclaration, scope, tree);
      int internalSize = 1;
      auto typeDeclaration = ASTQuery::findTypeDeclarationByName(
          blockDeclaration->getObjectType(), scope, tree);
      if (typeDeclaration &&
          (typeDeclaration->getObjectType() == "platformModule" ||
           typeDeclaration->getObjectType() == "platformBlock")) {
        auto inputs = typeDeclaration->getPropertyValue("inputs");
        if (inputs && inputs->getNodeType() == AST::List) {
          internalSize = inputs->getChildren().size();
        }
      }
      return size * internalSize;
    }
    return 1;
  }
  return 0;
}

int CodeAnalysis::getFunctionDataSize(std::shared_ptr<FunctionNode> func,
                                      ScopeStack scope, ASTNode tree,
                                      std::vector<LangError> *errors) {
  auto ports = func->getProperties();
  if (ports.size() == 0) {
    return 1;
  }
  int size = 1;
  for (std::shared_ptr<PropertyNode> port : ports) {
    ASTNode value = port->getValue();
    // FIXME need to decide the size by also looking at the port block size
    int newSize = CodeAnalysis::getNodeNumOutputs(value, scope, tree, errors);
    auto decl = ASTQuery::findDeclarationByName(func->getName(), scope, tree,
                                                func->getNamespaceList());
    if (decl && decl->getPropertyValue("ports")) {
      std::shared_ptr<DeclarationNode> declaredPort;
      for (auto portDecl : decl->getPropertyValue("ports")->getChildren()) {
        if (portDecl->getNodeType() == AST::Declaration) {
          auto nameNode = std::static_pointer_cast<DeclarationNode>(portDecl)
                              ->getPropertyValue("name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            if (port->getName() == std::static_pointer_cast<ValueNode>(nameNode)
                                       ->getStringValue()) {
              declaredPort =
                  std::static_pointer_cast<DeclarationNode>(portDecl);
              break;
            }
          }
        }
      }
      if (declaredPort) {
        auto portBlockNode = declaredPort->getPropertyValue("block");
        auto blocks = decl->getPropertyValue("blocks");
        if (portBlockNode && blocks) {
          std::string portBlockName;
          if (portBlockNode->getNodeType() == AST::Block) {
            portBlockName =
                std::static_pointer_cast<BlockNode>(portBlockNode)->getName();
          } else if (portBlockNode->getNodeType() == AST::Bundle) {
            portBlockName =
                std::static_pointer_cast<BundleNode>(portBlockNode)->getName();
          }
          auto portBlockDecl = ASTQuery::findDeclarationByName(
              portBlockName, {{nullptr, blocks->getChildren()}}, nullptr);
          if (declaredPort->getObjectType() == "mainInputPort" ||
              declaredPort->getObjectType() == "propertyInputPort") {
            size = CodeAnalysis::getNodeNumInputs(
                portBlockNode, {{nullptr, blocks->getChildren()}}, tree,
                errors);
          } else if (declaredPort->getObjectType() == "mainOutputPort" ||
                     declaredPort->getObjectType() == "propertyOutputPort") {
            size = CodeAnalysis::getNodeNumOutputs(
                portBlockNode, {{nullptr, blocks->getChildren()}}, tree,
                errors);
          }
        }
      }
    }
    if (size != newSize) {
      if (size == 1 && newSize > 0) {
        size = newSize;
      } else if (newSize == 1) {
        // just ignore
      } else {
        //        size = -1; // TODO should this be a reported error?
      }
    }
  }
  return size;
}

int CodeAnalysis::getFunctionNumInstances(std::shared_ptr<FunctionNode> func,
                                          ScopeStack scope, ASTNode tree,
                                          std::vector<LangError> *errors) {
  auto portProperties = func->getProperties();
  int numInstances = 1;
  for (const auto &propertyNode : portProperties) {
    auto portBlock = propertyNode->getValue();
    auto portName = propertyNode->getName();
    auto funcDecl = ASTQuery::findDeclarationByName(ASTQuery::getNodeName(func),
                                                    scope, tree);
    if (funcDecl) {
      if (funcDecl->getPropertyValue("blocks")) {
        scope.push_back(
            {func, funcDecl->getPropertyValue("blocks")->getChildren()});
      }
      auto ports = funcDecl->getPropertyValue("ports");
      if (ports) {
        // Match port in declaration to current property
        // This will tell us how many instances we should make
        for (const auto &port : ports->getChildren()) {
          if (port->getNodeType() == AST::Declaration) {
            auto portDecl = std::static_pointer_cast<DeclarationNode>(port);
            auto nameNode = portDecl->getPropertyValue("name");
            if (!(nameNode->getNodeType() == AST::String)) {
              continue;
            }
            if (std::static_pointer_cast<ValueNode>(nameNode)
                    ->getStringValue() == portName) {
              int propertyBlockSize = 0;
              if (portDecl->getObjectType() == "mainInputPort" ||
                  portDecl->getObjectType() == "propertyInputPort") {
                propertyBlockSize = CodeAnalysis::getNodeNumOutputs(
                    portBlock, scope, tree, errors);
              } else {
                propertyBlockSize = CodeAnalysis::getNodeNumInputs(
                    portBlock, scope, tree, errors);
              }
              auto internalPortBlock = portDecl->getPropertyValue("block");
              if (internalPortBlock) {
                int internalBlockSize;

                if (portDecl->getObjectType() == "mainInputPort" ||
                    portDecl->getObjectType() == "propertyInputPort") {
                  internalBlockSize = CodeAnalysis::getNodeNumOutputs(
                      internalPortBlock, scope, tree, errors);
                } else {
                  internalBlockSize = CodeAnalysis::getNodeNumInputs(
                      internalPortBlock, scope, tree, errors);
                }
                //                numInstances = 1;
                if (internalBlockSize == -2) {
                  // FIXME this assumes that the block is given the size of its
                  // own port, but it is possible it could be give the size of
                  // another port, causing an error or a different number of
                  // instances
                  return 1;
                }
                int portNumInstances = propertyBlockSize / internalBlockSize;
                assert(propertyBlockSize / internalBlockSize ==
                       float(propertyBlockSize) / internalBlockSize);
                if (numInstances == 0) {
                  numInstances = portNumInstances;
                } else if (numInstances == 1 && portNumInstances > 1) {
                  numInstances = portNumInstances;
                } else if (numInstances != portNumInstances &&
                           portNumInstances != 1) {
                  std::cerr << "ERROR size mismatch in function ports"
                            << std::endl;
                }
              }
            }
          }
        }
      }
    }
  }
  return numInstances;
}

std::shared_ptr<DeclarationNode>
CodeAnalysis::findDataTypeDeclaration(std::string dataTypeName, ASTNode tree) {
  for (const auto &node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      auto nodeDecl = std::static_pointer_cast<DeclarationNode>(node);
      if (nodeDecl->getObjectType() == "platformDataType") {
        auto typeProp = nodeDecl->getPropertyValue("type");
        if (typeProp && typeProp->getNodeType() == AST::String) {
          if (std::static_pointer_cast<ValueNode>(typeProp)->getStringValue() ==
              dataTypeName) {
            return nodeDecl;
          }
        }
      }
    }
  }
  return nullptr;
}

std::string
CodeAnalysis::getDataTypeForDeclaration(std::shared_ptr<DeclarationNode> decl,
                                        ASTNode tree) {
  if (decl->getObjectType() == "signal") {
    auto typeNode = decl->getPropertyValue("type");
    if (typeNode) {
      if (typeNode->getNodeType() == AST::Block) {
        return std::static_pointer_cast<BlockNode>(typeNode)->getName();
      } else if (typeNode->getNodeType() == AST::None) {
        // FIXME we should resolve the types in CodeResolver.
        return "_RealType";
      }
    }
    std::cerr << __FILE__ << ":" << __LINE__ << " ERROR unsupported object type"
              << std::endl;
    assert(0 == 1);

  } else if (decl->getObjectType() == "constant") {
    // TODO should constants take value from their given value or an
    // additional port?
    //    return getDataType(decl->getPropertyValue("value"), tree);
  } else if (decl->getObjectType() == "string") {
    return "_StringType";
  } else {
    //    qDebug() << __FILE__ << ":" << __LINE__ << " ERROR unsupported
    //    object type";
  }
  return std::string();
}
