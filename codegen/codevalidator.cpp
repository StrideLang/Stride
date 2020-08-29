/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <memory>
#include <sstream>

#include <QDebug>
#include <QVector>

#include "coderesolver.h"
#include "codevalidator.h"

CodeValidator::CodeValidator(ASTNode tree, Options options)
    : m_system(nullptr), m_tree(tree), m_options(options) {
  validateTree(tree);
}

CodeValidator::~CodeValidator() {}

void CodeValidator::validateTree(ASTNode tree) {
  m_tree = tree;
  if (tree) {
    validate();
  }
}

bool CodeValidator::isValid() { return m_errors.size() == 0; }

QVector<std::shared_ptr<SystemNode>>
CodeValidator::getSystemNodes(ASTNode tree) {
  //  Q_ASSERT(m_tree);
  QVector<std::shared_ptr<SystemNode>> platformNodes;
  vector<ASTNode> nodes = tree->getChildren();
  for (ASTNode node : nodes) {
    if (node->getNodeType() == AST::Platform) {
      platformNodes.push_back(static_pointer_cast<SystemNode>(node));
    }
  }
  return platformNodes;
}

void CodeValidator::validatePlatform(ASTNode tree, ScopeStack scopeStack) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Platform) {
      std::shared_ptr<SystemNode> platformNode =
          static_pointer_cast<SystemNode>(node);
    }
  }
}

QVector<ASTNode> CodeValidator::getBlocksInScope(ASTNode root,
                                                 ScopeStack scopeStack,
                                                 ASTNode tree) {
  QVector<ASTNode> blocks;
  if (root->getNodeType() == AST::Declaration ||
      root->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> decl =
        static_pointer_cast<DeclarationNode>(root);

    if (decl->getPropertyValue("ports")) {
      for (ASTNode block : decl->getPropertyValue("ports")->getChildren()) {
        blocks << block;
      }
    }
  } else if (root->getNodeType() == AST::List) {
    vector<ASTNode> elements =
        static_pointer_cast<ListNode>(root)->getChildren();
    foreach (ASTNode element, elements) {
      blocks << getBlocksInScope(element, scopeStack, tree);
    }
  } else if (root->getNodeType() == AST::Block) {
    std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(root);
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                       scopeStack, tree);
    if (declaration) {
      blocks = getBlocksInScope(declaration, scopeStack, tree);
    }
  } else if (root->getNodeType() == AST::Bundle) {
    std::shared_ptr<BundleNode> name = static_pointer_cast<BundleNode>(root);
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                       scopeStack, tree);
    if (declaration) {
      blocks = getBlocksInScope(declaration, scopeStack, tree);
    }
  } else {
    foreach (ASTNode child, root->getChildren()) {
      blocks << getBlocksInScope(child, scopeStack, tree);
    }
  }

  return blocks;
}

std::vector<string> CodeValidator::getUsedDomains(ASTNode tree) {
  std::vector<string> domains;
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      auto streamNode = static_pointer_cast<StreamNode>(node);
      auto childNode = streamNode->getLeft();
      auto nextNode = streamNode->getRight();
      while (childNode) {
        string domainName =
            CodeValidator::getNodeDomainName(childNode, {}, tree);
        if (domainName.size() > 0) {
          if (std::find(domains.begin(), domains.end(), domainName) ==
              domains.end()) {
            domains.push_back(domainName);
          }
        }
        if (!nextNode) {
          childNode = nullptr;
        } else if (nextNode->getNodeType() == AST::Stream) {
          streamNode = static_pointer_cast<StreamNode>(nextNode);
          childNode = streamNode->getLeft();
          nextNode = streamNode->getRight();
        } else {
          childNode = nextNode;
          nextNode = nullptr;
        }
      }
    } else if (node->getNodeType() == AST::Declaration ||
               node->getNodeType() == AST::BundleDeclaration) {
      auto decl = static_pointer_cast<DeclarationNode>(node);
      auto domainName =
          CodeValidator::getNodeDomainName(decl, ScopeStack(), tree);
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

string CodeValidator::getFrameworkForDomain(string domainName, ASTNode tree) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      DeclarationNode *decl = static_cast<DeclarationNode *>(node.get());
      if (decl->getObjectType() == "_domainDefinition") {
        ASTNode domainNameValue = decl->getPropertyValue("domainName");
        if (domainNameValue->getNodeType() == AST::String) {
          string declDomainName =
              static_cast<ValueNode *>(domainNameValue.get())->getStringValue();
          if (declDomainName == domainName) {
            ASTNode frameworkNameValue = decl->getPropertyValue("framework");
            if (frameworkNameValue->getNodeType() == AST::String) {
              return static_cast<ValueNode *>(frameworkNameValue.get())
                  ->getStringValue();
            } else if (frameworkNameValue->getNodeType() == AST::Block) {
              auto fwBlock = static_pointer_cast<BlockNode>(frameworkNameValue);

              auto declImportFramework = decl->getCompilerProperty("framework");
              std::string frameworkName;
              if (declImportFramework) {
                frameworkName =
                    static_pointer_cast<ValueNode>(declImportFramework)
                        ->getStringValue();
              }
              std::shared_ptr<DeclarationNode> fwDeclaration =
                  CodeValidator::findDeclaration(
                      QString::fromStdString(fwBlock->getName()), {}, tree,
                      fwBlock->getNamespaceList(), frameworkName);
              if (fwDeclaration &&
                  (fwDeclaration->getObjectType() == "_frameworkDescription")) {
                ASTNode frameworkName =
                    fwDeclaration->getPropertyValue("frameworkName");
                if (frameworkName &&
                    frameworkName->getNodeType() == AST::String) {
                  return static_cast<ValueNode *>(frameworkName.get())
                      ->getStringValue();
                }
              }
            }
          }
        }
      }
    }
  }
  return string();
}

std::vector<string> CodeValidator::getUsedFrameworks(ASTNode tree) {
  std::vector<std::string> domains = CodeValidator::getUsedDomains(tree);
  std::vector<std::string> usedFrameworks;
  for (string domain : domains) {
    usedFrameworks.push_back(
        CodeValidator::getFrameworkForDomain(domain, tree));
  }
  return usedFrameworks;
}

double CodeValidator::resolveRateToFloat(ASTNode rateNode, ScopeStack scope,
                                         ASTNode tree) {
  double rate = -1;
  QList<LangError> errors;
  if (rateNode->getNodeType() == AST::Int) {
    rate = CodeValidator::evaluateConstInteger(rateNode, {}, tree, errors);
    return rate;
  } else if (rateNode->getNodeType() == AST::Real) {
    rate = CodeValidator::evaluateConstReal(rateNode, {}, tree, errors);
    return rate;
  } else if (rateNode->getNodeType() == AST::Block) {
    BlockNode *block = static_cast<BlockNode *>(rateNode.get());
    std::shared_ptr<DeclarationNode> valueDeclaration =
        CodeValidator::findDeclaration(QString::fromStdString(block->getName()),
                                       scope, tree, block->getNamespaceList());
    if (valueDeclaration && valueDeclaration->getObjectType() == "constant") {
      std::shared_ptr<PropertyNode> property =
          CodeValidator::findPropertyByName(valueDeclaration->getProperties(),
                                            "value");
      if (property) {
        std::shared_ptr<ValueNode> rateNode =
            static_pointer_cast<ValueNode>(property->getValue());
        if (rateNode->getNodeType() == AST::Int) {
          rate =
              CodeValidator::evaluateConstInteger(rateNode, {}, tree, errors);
        } else if (rateNode->getNodeType() == AST::Real) {
          rate = CodeValidator::evaluateConstReal(rateNode, {}, tree, errors);
        }
      }
    }
    if (errors.size() == 0) {
      return rate;
    }
  } else if (rateNode->getNodeType() == AST::PortProperty) {
  }
  return rate;
}

double CodeValidator::getNodeRate(ASTNode node, ScopeStack scope,
                                  ASTNode tree) {
  if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                       scope, tree, name->getNamespaceList());
    if (declaration) {
      ASTNode rateNode = declaration->getPropertyValue("rate");
      if (rateNode) {
        return CodeValidator::resolveRateToFloat(rateNode, scope, tree);
      }
    }
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(
            QString::fromStdString(bundle->getName()), scope, tree,
            bundle->getNamespaceList());
    if (declaration) {
      ASTNode rateNode = declaration->getPropertyValue("rate");
      if (rateNode) {
        return CodeValidator::resolveRateToFloat(rateNode, scope, tree);
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    FunctionNode *func = static_cast<FunctionNode *>(node.get());
    return func->getRate();
  } else if (node->getNodeType() == AST::List ||
             node->getNodeType() == AST::Expression) {
    double rate = -1.0;
    for (ASTNode element : node->getChildren()) {
      double elementRate = CodeValidator::getNodeRate(element, scope, tree);
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
    auto decl = static_pointer_cast<DeclarationNode>(node);
    if (decl) {
      ASTNode rateNode = decl->getPropertyValue("rate");
      if (rateNode) {
        return CodeValidator::resolveRateToFloat(rateNode, scope, tree);
      }
    }
  }
  return -1;
}

void CodeValidator::setNodeRate(ASTNode node, double rate, ScopeStack scope,
                                ASTNode tree) {
  if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                       scope, tree, name->getNamespaceList());
    if (declaration) {
      std::shared_ptr<ValueNode> value =
          std::make_shared<ValueNode>(rate, __FILE__, __LINE__);
      declaration->replacePropertyValue("rate", value);
    }
    return;
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(
            QString::fromStdString(bundle->getName()), scope, tree,
            bundle->getNamespaceList());
    if (declaration) {
      std::shared_ptr<ValueNode> value =
          std::make_shared<ValueNode>(rate, __FILE__, __LINE__);
      if (!declaration->replacePropertyValue("rate", value)) {
        qDebug() << "Couldn't set rate. Rate property does not exist.";
      }
    }
    return;
  } else if (node->getNodeType() == AST::Function) {
    FunctionNode *func = static_cast<FunctionNode *>(node.get());
    func->setRate(rate);
  } else if (node->getNodeType() == AST::List ||
             node->getNodeType() == AST::Expression) {
    for (ASTNode element : node->getChildren()) {
      double elementRate = CodeValidator::getNodeRate(element, scope, tree);
      if (elementRate < 0.0) {
        CodeValidator::setNodeRate(element, rate, scope, tree);
      }
    }
  }
}

double
CodeValidator::getDefaultForTypeAsDouble(string type, string port,
                                         ScopeStack scope, ASTNode tree,
                                         std::vector<string> namespaces) {
  double outValue = 0.0;
  ASTNode value = CodeValidator::getDefaultPortValueForType(type, port, scope,
                                                            tree, namespaces);
  QList<LangError> errors;
  if (value) {
    outValue = CodeValidator::evaluateConstReal(value, scope, tree, errors);
  }
  return outValue;
}

ASTNode
CodeValidator::getDefaultPortValueForType(string type, string portName,
                                          ScopeStack scope, ASTNode tree,
                                          std::vector<string> namespaces) {
  QVector<ASTNode> ports =
      CodeValidator::getPortsForType(type, scope, tree, namespaces);
  if (!ports.isEmpty()) {
    for (ASTNode port : ports) {
      DeclarationNode *block = static_cast<DeclarationNode *>(port.get());
      Q_ASSERT(block->getNodeType() == AST::Declaration);
      Q_ASSERT(block->getObjectType() == "typeProperty");
      ASTNode platPortNameNode = block->getPropertyValue("name");
      ValueNode *platPortName =
          static_cast<ValueNode *>(platPortNameNode.get());
      Q_ASSERT(platPortName->getNodeType() == AST::String);
      if (platPortName->getStringValue() == portName) {
        ASTNode platPortDefault = block->getPropertyValue("default");
        if (platPortDefault) {
          return platPortDefault;
        }
      }
    }
  }
  return nullptr;
}

ASTNode CodeValidator::getInstance(ASTNode block, ScopeStack scopeStack,
                                   ASTNode tree) {
  ASTNode inst;
  if (block->getNodeType() == AST::List) {
    inst = block;
  } else if (block->getNodeType() == AST::Function) {
    inst = block;
  } else if (block->getNodeType() == AST::Declaration ||
             block->getNodeType() == AST::BundleDeclaration) {
    if (static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "signal" ||
        static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "switch" ||
        static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "trigger") {
      inst = block;
    } else if (static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
               "reaction") {
      inst = block;
    } else {
      qDebug() << "Unexpected declaration in getInstance()";
    }
  } else {
    std::shared_ptr<DeclarationNode> decl =
        static_pointer_cast<DeclarationNode>(
            block->getCompilerProperty("declaration"));
    if (!decl) {
      decl = CodeValidator::findDeclaration(
          CodeValidator::streamMemberName(block), scopeStack, tree);
    }
    if (decl) {
      auto typeDecl =
          CodeValidator::findTypeDeclaration(decl, scopeStack, tree);
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

bool CodeValidator::namespaceMatch(std::vector<string> scopeList,
                                   std::shared_ptr<DeclarationNode> decl,
                                   std::string currentFramework) {
  if (scopeList.size() == 1 && scopeList[0].size() == 0) {
    scopeList.clear();
  }
  auto declScopesNode = decl->getCompilerProperty("namespaceTree");
  std::vector<std::string> namespaceTree;
  if (declScopesNode) {
    for (auto node : declScopesNode->getChildren()) {
      assert(node->getNodeType() == AST::String);
      namespaceTree.push_back(
          static_pointer_cast<ValueNode>(node)->getStringValue());
    }
  }
  auto frameworkNode = decl->getCompilerProperty("framework");
  std::string framework;
  if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
    framework = static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    if (framework.size() > 0 && framework != currentFramework) {
      namespaceTree.insert(namespaceTree.begin(), framework);
    } else {
      if (scopeList.size() > 0 && scopeList[0] == framework) {
        scopeList.erase(scopeList.begin());
      }
    }
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

vector<StreamNode *> CodeValidator::getStreamsAtLine(ASTNode tree, int line) {
  vector<StreamNode *> streams;
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      if (node->getLine() == line) {
        streams.push_back(static_cast<StreamNode *>(node.get()));
      }
    }
  }
  return streams;
}

double CodeValidator::getDomainDefaultRate(
    std::shared_ptr<DeclarationNode> domainDecl) {
  Q_ASSERT(domainDecl->getObjectType() == "_domainDefinition");
  auto ratePort = domainDecl->getPropertyValue("rate");
  Q_ASSERT(ratePort->getNodeType() == AST::Real ||
           ratePort->getNodeType() == AST::Int);
  double rate = static_pointer_cast<ValueNode>(ratePort)->toReal();
  return rate;
}

QList<LangError> CodeValidator::getErrors() { return m_errors; }

QStringList CodeValidator::getPlatformErrors() { return m_system->getErrors(); }

void CodeValidator::validate() {
  m_errors.clear();
  if (m_tree) {
    validatePlatform(m_tree, {});
    validateTypes(m_tree, {});
    validateBundleIndeces(m_tree, {});
    validateBundleSizes(m_tree, {});
    validateSymbolUniqueness({{"", m_tree->getChildren()}});
    validateListTypeConsistency(m_tree, {});
    validateStreamSizes(m_tree, {});
    validateRates(m_tree);

    //         TODO: validate expression type consistency
    //         TODO: validate expression list operations
  }
  sortErrors();
}

void CodeValidator::validateTypes(ASTNode node, ScopeStack scopeStack,
                                  vector<string> parentNamespace,
                                  std::string currentFramework) {
  if (node->getNodeType() == AST::BundleDeclaration ||
      node->getNodeType() == AST::Declaration) {
    std::shared_ptr<DeclarationNode> decl =
        static_pointer_cast<DeclarationNode>(node);
    auto blockType = decl->getObjectType();

    auto frameworkNode = node->getCompilerProperty("framework");
    std::string frameworkName;
    if (frameworkNode) {
      frameworkName =
          static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findTypeDeclaration(decl, scopeStack, m_tree,
                                           frameworkName);
    if (!declaration) { // Check if node type exists
      LangError error;  // Not a valid type, then error
      error.type = LangError::UnknownType;
      error.lineNumber = decl->getLine();
      error.errorTokens.push_back(decl->getObjectType());
      error.filename = decl->getFilename();
      m_errors << error;
    } else {
      // Validate port names and types
      vector<std::shared_ptr<PropertyNode>> ports = decl->getProperties();
      ASTNode blocksNode = decl->getPropertyValue("blocks");
      vector<ASTNode> blocks;
      if (blocksNode) {
        blocks = blocksNode->getChildren();
      }
      scopeStack.push_back({decl->getName(), blocks});
      for (auto port : ports) {
        QString portName = QString::fromStdString(port->getName());
        // Check if portname is valid
        // TODO: Move '_' ports to the compiler properties functions in the AST
        // nodes
        if (!portName.startsWith("_")) { // Ports starting with '_' have been
                                         // put in by the code validator
          QVector<ASTNode> portTypesList =
              validTypesForPort(declaration, portName, scopeStack, m_tree);
          if (portTypesList.isEmpty()) {
            LangError error;
            error.type = LangError::InvalidPort;
            error.lineNumber = port->getLine();
            error.errorTokens.push_back(blockType);
            error.errorTokens.push_back(portName.toStdString());
            error.filename = port->getFilename();
            m_errors << error;
          } else {
            // Then check type passed to port is valid
            bool typeIsValid = false;
            ASTNode portValue = port->getValue();

            auto getTypeName = [&](ASTNode child) {
              if (child->getNodeType() == AST::Declaration ||
                  child->getNodeType() == AST::BundleDeclaration) {
                auto typeDeclaration = CodeValidator::findTypeDeclaration(
                    static_pointer_cast<DeclarationNode>(child), scopeStack,
                    m_tree, currentFramework);
                return typeDeclaration->getName();
              } else if (child->getNodeType() == AST::Block ||
                         child->getNodeType() == AST::Bundle) {
                auto decl = CodeValidator::findDeclaration(
                    CodeValidator::streamMemberName(child), scopeStack, m_tree,
                    child->getNamespaceList(), currentFramework);
                if (decl) {
                  auto typeDeclaration = CodeValidator::findTypeDeclaration(
                      decl, scopeStack, m_tree, currentFramework);
                  return typeDeclaration->getName();
                }
                return std::string();
              } else if (child->getNodeType() == AST::String) {
                return std::string("_StringLiteral");
              } else if (child->getNodeType() == AST::Int) {
                return std::string("_IntLiteral");
              } else if (child->getNodeType() == AST::Real) {
                return std::string("_RealLiteral");
              } else if (child->getNodeType() == AST::Switch) {
                return std::string("_SwitchLiteral");
              } else if (child->getNodeType() == AST::PortProperty) {
                return std::string("_PortProperty");
              } else if (child->getNodeType() == AST::None) {
                return std::string("_NoneLiteral");
              }
              return std::string();
            };

            if (portValue) {
              std::vector<std::string> validTypeNames;

              for (ASTNode validType : portTypesList) {
                if (validType->getNodeType() == AST::String) {
                  std::string typeCode =
                      static_cast<ValueNode *>(validType.get())
                          ->getStringValue();
                  validTypeNames.push_back(typeCode);
                } else if (validType->getNodeType() == AST::Block) {
                  auto blockNode = static_pointer_cast<BlockNode>(validType);
                  std::shared_ptr<DeclarationNode> declaration =
                      findDeclaration(
                          QString::fromStdString(blockNode->getName()),
                          scopeStack, m_tree);
                  //                                    ASTNode typeNameValue =
                  //                                    declaration->getPropertyValue("typeName");
                  //                                    Q_ASSERT(typeNameValue->getNodeType()
                  //                                    == AST::String);
                  //                                    Q_ASSERT(declaration);
                  if (declaration) {
                    string validTypeName = declaration->getName();
                    validTypeNames.push_back(validTypeName);
                  }
                }
              }
              std::string failedType;
              if (std::find(validTypeNames.begin(), validTypeNames.end(), "") ==
                  validTypeNames.end()) {
                if (portValue->getNodeType() == AST::List) {
                  typeIsValid = true;
                  for (auto child : portValue->getChildren()) {
                    auto typeName = getTypeName(child);
                    bool thisTypeIsValid = false;
                    for (auto validTypeName : validTypeNames) {
                      if (validTypeName == typeName) {
                        thisTypeIsValid = true;
                        break;
                      }
                    }
                    if (!thisTypeIsValid) {
                      failedType += typeName + " ";
                    }
                    typeIsValid &= thisTypeIsValid;
                  }
                } else {
                  auto typeName = getTypeName(portValue);
                  for (auto validTypeName : validTypeNames) {
                    if (validTypeName == typeName || validTypeName == "") {
                      typeIsValid = true;
                      break;
                    }
                    failedType = typeName;
                  }
                }

              } else { // Catch all for old behavior
                typeIsValid = true;
              }
              if (!typeIsValid) {
                LangError error;
                error.type = LangError::InvalidPortType;
                error.lineNumber = port->getLine();
                error.errorTokens.push_back(blockType);
                error.errorTokens.push_back(portName.toStdString());
                error.errorTokens.push_back(failedType);
                //                                error.errorTokens.push_back(typeNames[0]);
                //                                //FIXME mark which entry fails
                std::string validTypesList;
                for (auto type : validTypeNames) {
                  validTypesList += type + ",";
                }
                if (validTypesList.size() > 0) {
                  validTypesList =
                      validTypesList.substr(0, validTypesList.size() - 1);
                }
                error.errorTokens.push_back(validTypesList);
                error.filename = port->getFilename();
                m_errors << error;
              }
            }
          }
        }
      }
    }
    std::vector<ASTNode> subScope;
    ASTNode subScopeNode = CodeValidator::getBlockSubScope(decl);
    if (subScopeNode) {
      subScope = subScopeNode->getChildren();
    }
    if (decl->getPropertyValue("ports")) {
      for (ASTNode port : decl->getPropertyValue("ports")->getChildren()) {
        subScope.push_back(port);
      }
    }
    scopeStack.push_back({streamMemberName(node), subScope});

    frameworkNode = decl->getCompilerProperty("framework");
    if (frameworkNode) {
      frameworkName =
          std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    // For BundleDeclarations in particular, we need to ignore the bundle when
    // declaring types. The inner bundle has no scope set, and trying to find it
    // will fail if the declaration is scoped....
    foreach (auto property, decl->getProperties()) {
      validateTypes(property->getValue(), scopeStack, decl->getNamespaceList(),
                    frameworkName);
    }
    return;
  } else if (node->getNodeType() == AST::Stream) {
    validateStreamMembers(static_cast<StreamNode *>(node.get()), scopeStack);
    return; // Children are validated when validating stream
  } else if (node->getNodeType() == AST::List) {
    // Children are checked automatically below
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *block = static_cast<BlockNode *>(node.get());
    vector<string> namespaces = block->getNamespaceList();
    namespaces.insert(namespaces.begin(), parentNamespace.begin(),
                      parentNamespace.end());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(block->getName(), scopeStack, m_tree,
                                       namespaces, currentFramework);
    if (!declaration) {
      LangError error;
      error.type = LangError::UndeclaredSymbol;
      error.lineNumber = node->getLine();
      error.filename = node->getFilename();
      //            error.errorTokens.push_back(block->getName());
      //            error.errorTokens.push_back(block->getNamespace());
      string blockName = "";
      if (block->getScopeLevels()) {
        for (unsigned int i = 0; i < block->getScopeLevels(); i++) {
          blockName += block->getScopeAt(i);
          blockName += "::";
        }
      }
      blockName += block->getName();
      error.errorTokens.push_back(blockName);
      m_errors << error;
    }

  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    vector<string> namespaces = bundle->getNamespaceList();
    namespaces.insert(namespaces.begin(), parentNamespace.begin(),
                      parentNamespace.end());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(bundle->getName(), scopeStack, m_tree,
                                       namespaces);
    if (!declaration) {
      LangError error;
      error.type = LangError::UndeclaredSymbol;
      error.lineNumber = node->getLine();
      error.filename = node->getFilename();
      //            error.errorTokens.push_back(bundle->getName());
      //            error.errorTokens.push_back(bundle->getNamespace());
      string bundleName = "";
      if (bundle->getScopeLevels()) {
        for (unsigned int i = 0; i < bundle->getScopeLevels(); i++) {
          bundleName += bundle->getScopeAt(i);
          bundleName += "::";
        }
      }
      bundleName += bundle->getName();
      error.errorTokens.push_back(bundleName);
      m_errors << error;
    }
  } else if (node->getNodeType() == AST::Function) {
    FunctionNode *func = static_cast<FunctionNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(QString::fromStdString(func->getName()),
                                       scopeStack, m_tree,
                                       func->getNamespaceList());
    if (!declaration) {
      LangError error;
      error.type = LangError::UndeclaredSymbol;
      error.lineNumber = node->getLine();
      error.filename = node->getFilename();
      //            error.errorTokens.push_back(func->getName());
      //            error.errorTokens.push_back(func->getNamespace());
      string funcName = "";
      if (func->getScopeLevels()) {
        for (unsigned int i = 0; i < func->getScopeLevels(); i++) {
          funcName += func->getScopeAt(i);
          funcName += "::";
        }
      }
      funcName += func->getName();
      error.errorTokens.push_back(funcName);
      m_errors << error;
    } else {
      for (std::shared_ptr<PropertyNode> property : func->getProperties()) {
        string propertyName = property->getName();
        vector<string> validPorts = getModulePropertyNames(declaration);
        // TODO "domain" port is being allowed forcefully. Should this be
        // specified in a different way?
        if (/*propertyName.at(0) != '_' && propertyName != "domain" &&*/
            std::find(validPorts.begin(), validPorts.end(), propertyName) ==
            validPorts.end()) {
          LangError error;
          error.type = LangError::InvalidPort;
          error.lineNumber = func->getLine();
          error.errorTokens.push_back(func->getName());
          error.errorTokens.push_back(propertyName);
          error.filename = func->getFilename();
          m_errors << error;
        }
      }
    }
  }

  for (ASTNode childNode : node->getChildren()) {
    auto frameworkNode = childNode->getCompilerProperty("framework");
    std::string frameworkName;
    if (frameworkNode) {
      frameworkName =
          static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    validateTypes(childNode, scopeStack, {}, frameworkName);
  }
}

void CodeValidator::validateStreamMembers(StreamNode *stream,
                                          ScopeStack scopeStack) {
  ASTNode member = stream->getLeft();
  QString name;
  while (member) {
    validateTypes(member, scopeStack);
    if (stream && stream->getRight()->getNodeType() == AST::Stream) {
      validateStreamMembers(static_cast<StreamNode *>(stream->getRight().get()),
                            scopeStack);
      return;
    } else {
      if (stream) {
        member = stream->getRight();
        stream = nullptr;
      } else {
        member = nullptr;
      }
    }
  }
}

void CodeValidator::validateBundleIndeces(ASTNode node, ScopeStack scope) {
  if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    PortType type = resolveNodeOutType(bundle->index(), scope, m_tree);
    if (type != ConstInt && type != Signal && type != PortProperty &&
        type != None) {
      LangError error;
      error.type = LangError::IndexMustBeInteger;
      error.lineNumber = bundle->getLine();
      error.errorTokens.push_back(bundle->getName());
      error.errorTokens.push_back(getPortTypeName(type).toStdString());
      m_errors << error;
    }
  }
  for (ASTNode child : node->getChildren()) {
    QVector<ASTNode> blocksInScope = getBlocksInScope(child, scope, m_tree);
    auto subScope = scope;
    subScope.push_back({streamMemberName(child), blocksInScope.toStdVector()});
    validateBundleIndeces(child, subScope);
  }
}

void CodeValidator::validateBundleSizes(ASTNode node, ScopeStack scope) {
  if (node->getNodeType() == AST::BundleDeclaration) {
    QList<LangError> errors;
    std::shared_ptr<DeclarationNode> declaration =
        static_pointer_cast<DeclarationNode>(node);
    // FIXME this needs to be rewritten looking at the port block size
    int size = getBlockDeclaredSize(declaration, scope, m_tree, errors);
    int datasize = getBlockDataSize(declaration, scope, errors);
    if (size != datasize && datasize > 1) {
      LangError error;
      error.type = LangError::BundleSizeMismatch;
      error.lineNumber = node->getLine();
      error.errorTokens.push_back(declaration->getBundle()->getName());
      error.errorTokens.push_back(QString::number(size).toStdString());
      error.errorTokens.push_back(QString::number(datasize).toStdString());
      m_errors << error;
    }

    // TODO : use this pass to store the computed value of constant int?
    m_errors << errors;
    auto decl = static_pointer_cast<DeclarationNode>(node);
    auto subScope = getBlockSubScope(decl);
    if (subScope) {
      scope.push_back({decl->getName(), subScope->getChildren()});
    }
  }

  QVector<ASTNode> children =
      QVector<ASTNode>::fromStdVector(node->getChildren());
  for (ASTNode node : children) {
    validateBundleSizes(node, scope);
  }
}

void CodeValidator::validateSymbolUniqueness(ScopeStack scope) {
  auto singleScope = scope.back();
  while (singleScope.second.size() > 0) {
    auto node = singleScope.second.front();
    singleScope.second.erase(singleScope.second.begin());
    for (ASTNode sibling : singleScope.second) {
      QString nodeName, siblingName;
      if (sibling->getNodeType() == AST::Declaration ||
          sibling->getNodeType() == AST::BundleDeclaration) {
        siblingName = QString::fromStdString(
            static_cast<DeclarationNode *>(sibling.get())->getName());
      }
      if (node->getNodeType() == AST::Declaration ||
          node->getNodeType() == AST::BundleDeclaration) {
        nodeName = QString::fromStdString(
            static_cast<DeclarationNode *>(node.get())->getName());
      }
      if (!nodeName.isEmpty() && !siblingName.isEmpty() &&
          nodeName == siblingName) {
        // Check if framework matches
        auto nodeFrameworkNode = node->getCompilerProperty("framework");
        auto siblingFrameworkNode = sibling->getCompilerProperty("framework");
        if (nodeFrameworkNode &&
            nodeFrameworkNode->getNodeType() == AST::String &&
            siblingFrameworkNode &&
            siblingFrameworkNode->getNodeType() == AST::String) {
          auto nodeFramework = static_pointer_cast<ValueNode>(nodeFrameworkNode)
                                   ->getStringValue();
          auto siblingFramework =
              static_pointer_cast<ValueNode>(siblingFrameworkNode)
                  ->getStringValue();
          if (nodeFramework != siblingFramework) {
            continue;
          }
        }

        // Check if scope matches
        auto nodeScopes = node->getNamespaceList();
        auto siblingScopes = sibling->getNamespaceList();
        if (nodeScopes.size() == siblingScopes.size()) {
          bool duplicateSymbol = true;
          for (size_t i = 0; i < nodeScopes.size(); i++) {
            if (nodeScopes[i] != siblingScopes[i]) {
              duplicateSymbol = false;
              break;
            }
          }
          if (duplicateSymbol) {
            LangError error;
            error.type = LangError::DuplicateSymbol;
            error.lineNumber = sibling->getLine();
            error.filename = sibling->getFilename();
            error.errorTokens.push_back(nodeName.toStdString());
            error.errorTokens.push_back(node->getFilename());
            error.errorTokens.push_back(std::to_string(node->getLine()));
            m_errors << error;
          }
        }
      }
    }
    if (node->getNodeType() == AST::Declaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "module" ||
          decl->getObjectType() == "reaction" ||
          decl->getObjectType() == "loop") {
        auto blocks = decl->getPropertyValue("blocks")->getChildren();
        auto ports = decl->getPropertyValue("ports")->getChildren();
        std::vector<ASTNode> subScope = blocks;
        subScope.insert(subScope.end(), ports.begin(), ports.end());
        scope.push_back({decl->getName(), subScope});
        validateSymbolUniqueness(scope);
        scope.pop_back();
      }
    }
  }
}

void CodeValidator::validateListTypeConsistency(ASTNode node,
                                                ScopeStack scope) {
  // Lists don't have to be consistent.
  //    if (node->getNodeType() == AST::List) {
  //        PortType type = resolveListType(static_cast<ListNode *>(node),
  //        scope, m_tree); if (type == Invalid) {
  //            LangError error;
  //            error.type = LangError::InconsistentList;
  //            error.lineNumber = node->getLine();
  //            // TODO: provide more information on inconsistent list
  ////            error.errorTokens <<
  //            m_errors << error;
  //        }
  //    }

  //    QVector<ASTNode > children = QVector<ASTNode
  //    >::fromStdVector(node->getChildren()); foreach(ASTNode node, children) {
  //        validateListTypeConsistency(node, children);
  //    }
}

void CodeValidator::validateStreamSizes(ASTNode tree, ScopeStack scope) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      StreamNode *stream = static_cast<StreamNode *>(node.get());
      validateStreamInputSize(stream, scope, m_errors);
    } else if (node->getNodeType() == AST::Declaration) {
      DeclarationNode *decl = static_cast<DeclarationNode *>(node.get());
      if (decl) {
        if (decl->getObjectType() == "module" ||
            decl->getObjectType() == "reaction" ||
            decl->getObjectType() == "loop") {
          auto subScope = scope;
          subScope.push_back({decl->getName(),
                              decl->getPropertyValue("blocks")->getChildren()});
          auto streams = decl->getPropertyValue("streams")->getChildren();
          for (auto node : streams) {
            if (node->getNodeType() == AST::Stream) {
              auto stream = std::static_pointer_cast<StreamNode>(node);
              validateStreamInputSize(stream.get(), subScope, m_errors);
            }
          }

          // FIXME we need to validate streams recursively within modules and
          // reactions
        }
      }
    }
  }
}

void CodeValidator::validateRates(ASTNode tree) {
  if ((m_options & NO_RATE_VALIDATION) == 0) {
    for (ASTNode node : tree->getChildren()) {
      validateNodeRate(node, tree);
    }
  }
}

void CodeValidator::validateNodeRate(ASTNode node, ASTNode tree) {
  if (node->getNodeType() == AST::Declaration ||
      node->getNodeType() == AST::BundleDeclaration) {
    DeclarationNode *decl = static_cast<DeclarationNode *>(node.get());
    if (decl->getObjectType() == "signal") {
      //            if (findRateInProperties(decl->getProperties(), {}, tree) <
      //            0) {
      //                LangError error;
      //                error.type = LangError::UnresolvedRate;
      //                error.lineNumber = node->getLine();
      //                error.filename = node->getFilename();
      //                error.errorTokens.push_back(decl->getName());
      //                m_errors << error;
      //            }
    }
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *stream = static_cast<StreamNode *>(node.get());
    validateNodeRate(stream->getLeft(), tree);
    validateNodeRate(stream->getRight(), tree);
  } else if (node->getNodeType() == AST::Expression) {
    for (ASTNode child : node->getChildren()) {
      validateNodeRate(child, tree);
    }
  } else if (node->getNodeType() == AST::Function) {
    for (std::shared_ptr<PropertyNode> prop :
         static_cast<FunctionNode *>(node.get())->getProperties()) {
      validateNodeRate(prop->getValue(), tree);
    }
  }
  // TODO also need to validate rates within module and reaction streams
}

bool errorLineIsLower(const LangError &err1, const LangError &err2) {
  return err1.lineNumber < err2.lineNumber;
}

void CodeValidator::sortErrors() {
  std::sort(m_errors.begin(), m_errors.end(), errorLineIsLower);
}

void CodeValidator::validateStreamInputSize(StreamNode *stream,
                                            ScopeStack scope,
                                            QList<LangError> &errors) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();

  int leftOutSize = getNodeNumOutputs(left, scope, m_tree, errors);
  int rightInSize = getNodeNumInputs(right, scope, m_tree, errors);

  auto leftDecl = CodeValidator::findDeclaration(
      CodeValidator::streamMemberName(left), scope, m_tree);
  std::shared_ptr<DeclarationNode> rightDecl;
  if (right->getNodeType() == AST::Stream) {
    auto nextStreamMember = static_pointer_cast<StreamNode>(right)->getLeft();
    rightDecl = CodeValidator::findDeclaration(
        CodeValidator::streamMemberName(nextStreamMember), scope, m_tree);
  } else {
    rightDecl = CodeValidator::findDeclaration(
        CodeValidator::streamMemberName(right), scope, m_tree);
  }
  if ((leftDecl && leftDecl->getObjectType() == "buffer") ||
      (rightDecl && rightDecl->getObjectType() == "buffer")) {
    // FIXME how should buffer size be validated?

  } else if ((leftDecl && leftDecl->getObjectType() == "table") ||
             (rightDecl && rightDecl->getObjectType() == "table")) {
    // FIXME how should table size be validated?

  } else {
    if (leftOutSize == -2 || rightInSize == -2) {
      // FIXME Hack while calculation of port portperties size is implemented
    } else {
      if ((leftOutSize != rightInSize &&
           ((int)(rightInSize / (double)leftOutSize)) !=
               (rightInSize / (double)leftOutSize)) ||
          rightInSize == 0) {
        LangError error;
        error.type = LangError::StreamMemberSizeMismatch;
        error.lineNumber = right->getLine();
        error.errorTokens.push_back(QString::number(leftOutSize).toStdString());
        error.errorTokens.push_back(getNodeText(left).toStdString());
        error.errorTokens.push_back(QString::number(rightInSize).toStdString());
        error.filename = left->getFilename();
        errors << error;
      }
    }
  }

  if (right->getNodeType() == AST::Stream) {
    validateStreamInputSize(static_cast<StreamNode *>(right.get()), scope,
                            errors);
  }
}

int CodeValidator::getBlockDeclaredSize(std::shared_ptr<DeclarationNode> block,
                                        ScopeStack scope, ASTNode tree,
                                        QList<LangError> &errors) {
  if (block->getNodeType() == AST::Declaration) {
    if (block->getObjectType() == "buffer") {
      auto sizeNode =
          static_pointer_cast<DeclarationNode>(block)->getPropertyValue("size");
      if (sizeNode) {
        QList<LangError> errors;
        auto size =
            CodeValidator::evaluateConstInteger(sizeNode, scope, tree, errors);
        return size;
      }
      return -1;
    } else {
      return 1;
    }
  }
  int size = -1;
  Q_ASSERT(block->getNodeType() == AST::BundleDeclaration);
  BundleNode *bundle = static_cast<BundleNode *>(block->getBundle().get());
  if (bundle->getNodeType() == AST::Bundle) {
    size = 0;
    ListNode *indexList = bundle->index().get();
    vector<ASTNode> indexExps = indexList->getChildren();
    for (ASTNode exp : indexExps) {
      if (exp->getNodeType() == AST::Range) {
        RangeNode *range = static_cast<RangeNode *>(exp.get());
        ASTNode start = range->startIndex();
        int startIndex, endIndex;
        PortType type = CodeValidator::resolveNodeOutType(start, scope, tree);
        if (type == ConstInt) {
          startIndex =
              CodeValidator::evaluateConstInteger(start, scope, tree, errors);
        } else {
          // TODO: Do something if not integer
          continue;
        }
        ASTNode end = range->startIndex();
        type = CodeValidator::resolveNodeOutType(end, scope, tree);
        if (type == ConstInt) {
          endIndex =
              CodeValidator::evaluateConstInteger(end, scope, tree, errors);
        } else {
          // TODO: Do something if not integer
          continue;
        }
        if (end > start) {
          size += endIndex + startIndex + 1;
        }

      } else {
        PortType type = CodeValidator::resolveNodeOutType(exp, scope, tree);
        if (type == ConstInt) {
          size += CodeValidator::evaluateConstInteger(exp, scope, tree, errors);
        } else if (type == PortProperty) {
          return -2; // FIXME calculate actual size from external connection
        } else {
          return -1;
        }

        // TODO: Something should be done if index isn't integer
      }
    }
  }
  return size;
}

int CodeValidator::getBlockDataSize(
    std::shared_ptr<DeclarationNode> declaration, ScopeStack scope,
    QList<LangError> &errors) {
  QVector<std::shared_ptr<PropertyNode>> ports =
      QVector<std::shared_ptr<PropertyNode>>::fromStdVector(
          declaration->getProperties());
  if (ports.size() == 0) {
    return 0;
  }
  int size = getNodeNumOutputs(ports.at(0)->getValue(), scope, m_tree, errors);
  for (std::shared_ptr<PropertyNode> port : ports) {
    ASTNode value = port->getValue();
    int newSize = getNodeNumOutputs(value, scope, m_tree, errors);
    if (size != newSize) {
      if (size == 1 && newSize > 0) {
        size = newSize;
      }
      if (newSize == 1) {
        // just ignore
      } else {
        //        size = -1;
      }
    }
  }
  return size;
}

int CodeValidator::getFunctionDataSize(std::shared_ptr<FunctionNode> func,
                                       ScopeStack scope, ASTNode tree,
                                       QList<LangError> &errors) {
  QVector<std::shared_ptr<PropertyNode>> ports =
      QVector<std::shared_ptr<PropertyNode>>::fromStdVector(
          func->getProperties());
  if (ports.size() == 0) {
    return 1;
  }
  int size = 1;
  for (std::shared_ptr<PropertyNode> port : ports) {
    ASTNode value = port->getValue();
    // FIXME need to decide the size by also looking at the port block size
    int newSize = CodeValidator::getNodeNumOutputs(value, scope, tree, errors);
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

int CodeValidator::getFunctionNumInstances(std::shared_ptr<FunctionNode> func,
                                           ScopeStack scope, ASTNode tree) {
  auto portProperties = func->getProperties();
  int numInstances = 1;
  for (auto propertyNode : portProperties) {
    auto portBlock = propertyNode->getValue();
    QList<LangError> errors;
    auto portName = propertyNode->getName();
    auto funcDecl = CodeValidator::findDeclaration(
        CodeValidator::streamMemberName(func), scope, tree);
    if (funcDecl) {
      if (funcDecl->getPropertyValue("blocks")) {
        scope.push_back({funcDecl->getName(),
                         funcDecl->getPropertyValue("blocks")->getChildren()});
      }
      auto ports = funcDecl->getPropertyValue("ports");
      if (ports) {
        // Match port in declaration to current property
        // This will tell us how many instances we should make
        for (auto port : ports->getChildren()) {
          if (port->getNodeType() == AST::Declaration) {
            auto portDecl = static_pointer_cast<DeclarationNode>(port);
            auto nameNode = portDecl->getPropertyValue("name");
            if (!(nameNode->getNodeType() == AST::String)) {
              continue;
            }
            if (static_pointer_cast<ValueNode>(nameNode)->getStringValue() ==
                portName) {
              int propertyBlockSize = 0;
              if (portDecl->getObjectType() == "mainInputPort" ||
                  portDecl->getObjectType() == "propertyInputPort") {
                propertyBlockSize = CodeValidator::getNodeNumOutputs(
                    portBlock, scope, tree, errors);
              } else {
                propertyBlockSize = CodeValidator::getNodeNumInputs(
                    portBlock, scope, tree, errors);
              }
              auto internalPortBlock = portDecl->getPropertyValue("block");
              if (internalPortBlock) {
                int internalBlockSize;

                if (portDecl->getObjectType() == "mainInputPort" ||
                    portDecl->getObjectType() == "propertyInputPort") {
                  internalBlockSize = CodeValidator::getNodeNumOutputs(
                      internalPortBlock, scope, tree, errors);
                } else {
                  internalBlockSize = CodeValidator::getNodeNumInputs(
                      internalPortBlock, scope, tree, errors);
                }
                int portNumInstances = propertyBlockSize / internalBlockSize;
                Q_ASSERT(propertyBlockSize / internalBlockSize ==
                         float(propertyBlockSize) / internalBlockSize);
                if (numInstances == 0) {
                  numInstances = portNumInstances;
                } else if (numInstances == 1 && portNumInstances > 1) {
                  numInstances = portNumInstances;
                } else if (numInstances != portNumInstances) {
                  qDebug() << "ERROR size mismatch in function ports";
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

int CodeValidator::getBundleSize(BundleNode *bundle, ScopeStack scope,
                                 ASTNode tree, QList<LangError> &errors) {
  std::shared_ptr<ListNode> indexList = bundle->index();
  int size = 0;
  vector<ASTNode> listExprs = indexList->getChildren();
  PortType type;

  for (ASTNode expr : listExprs) {
    switch (expr->getNodeType()) {
    case AST::Int:
      size += 1;
      break;
    case AST::Range:
      size +=
          evaluateConstInteger(static_cast<RangeNode *>(expr.get())->endIndex(),
                               scope, tree, errors) -
          evaluateConstInteger(
              static_cast<RangeNode *>(expr.get())->startIndex(), scope, tree,
              errors) +
          1;
      break;
    case AST::Expression:
      type = resolveExpressionType(static_cast<ExpressionNode *>(expr.get()),
                                   scope, tree);
      if (type == ConstInt) {
        size += 1;
      }
      break;
    case AST::Block:
      size += 1;
      break;

    case AST::PortProperty:
      size = -2;
      return size;
    default:
      break;
    }
  }

  return size;
}

QString CodeValidator::getNodeText(ASTNode node) {
  QString outText;
  if (node->getNodeType() == AST::Block) {
    outText =
        QString::fromStdString(static_cast<BlockNode *>(node.get())->getName());
  } else if (node->getNodeType() == AST::Bundle) {
    outText = QString::fromStdString(
        static_cast<BundleNode *>(node.get())->getName());
  } else if (node->getNodeType() == AST::Function) {
    outText = QString::fromStdString(
        static_cast<FunctionNode *>(node.get())->getName());
  } else if (node->getNodeType() == AST::List) {
    outText = "[ List ]";
  } else {
    qDebug() << "Unsupported type in getNodeText(ASTNode node)";
  }
  return outText;
}

ASTNode CodeValidator::getTree() const { return m_tree; }

void CodeValidator::setTree(const ASTNode &tree) { m_tree = tree; }

int CodeValidator::getLargestPropertySize(
    vector<std::shared_ptr<PropertyNode>> &properties, ScopeStack scope,
    ASTNode tree, QList<LangError> &errors) {
  int maxSize = 1;
  for (auto property : properties) {
    ASTNode value = property->getValue();
    if (value->getNodeType() == AST::Block) {
      BlockNode *name = static_cast<BlockNode *>(value.get());
      std::shared_ptr<DeclarationNode> block =
          findDeclaration(QString::fromStdString(name->getName()), scope, tree);
      if (block) {
        if (block->getNodeType() == AST::Declaration) {
          if (maxSize < 1) {
            maxSize = 1;
          }
        } else if (block->getNodeType() == AST::BundleDeclaration) {
          ASTNode index = block->getBundle()->index();
          int newSize = evaluateConstInteger(index, {}, tree, errors);
          if (newSize > maxSize) {
            maxSize = newSize;
          }
        }
      }
    } else if (value->getNodeType() == AST::List) {
      ListNode *list = static_cast<ListNode *>(value.get());
      int newSize = list->getChildren().size();
      if (newSize > maxSize) {
        maxSize = newSize;
      }

    } else if (value->getNodeType() == AST::Int ||
               value->getNodeType() == AST::Real ||
               value->getNodeType() == AST::String) {
      if (maxSize < 1) {
        maxSize = 1;
      }
    }
  }
  return maxSize;
}

ASTNode
CodeValidator::getBlockSubScope(std::shared_ptr<DeclarationNode> block) {
  ASTNode internalBlocks = nullptr;
  if (block->getObjectType() == "module") {
    internalBlocks = block->getPropertyValue("blocks");
  } else if (block->getObjectType() == "reaction") {
    internalBlocks = block->getPropertyValue("blocks");
  } else if (block->getObjectType() == "loop") {
    internalBlocks = block->getPropertyValue("blocks");
  }
  return internalBlocks;
}

int CodeValidator::getNodeNumOutputs(ASTNode node, const ScopeStack &scope,
                                     ASTNode tree, QList<LangError> &errors) {
  if (node->getNodeType() == AST::List) {
    int size = 0;
    for (ASTNode member : node->getChildren()) {
      size += CodeValidator::getNodeNumOutputs(member, scope, tree, errors);
    }
    return size;
  } else if (node->getNodeType() == AST::Bundle) {
    return getBundleSize(static_cast<BundleNode *>(node.get()), scope, tree,
                         errors);
  } else if (node->getNodeType() == AST::Int ||
             node->getNodeType() == AST::Real ||
             node->getNodeType() == AST::String ||
             node->getNodeType() == AST::Switch) {
    return 1;
  } else if (node->getNodeType() == AST::Expression) {
    auto expr = static_pointer_cast<ExpressionNode>(node);
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
        findDeclaration(QString::fromStdString(name->getName()), scope, tree);
    if (block) {
      return getTypeNumOutputs(block, scope, tree, errors);
    } else {
      return -1;
    }
  } else if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        static_pointer_cast<FunctionNode>(node);
    std::shared_ptr<DeclarationNode> platformFunc =
        CodeValidator::findDeclaration(QString::fromStdString(func->getName()),
                                       scope, tree);
    int dataSize =
        CodeValidator::getFunctionDataSize(func, scope, tree, errors);
    if (platformFunc) {
      return getTypeNumOutputs(platformFunc, scope, tree, errors) * dataSize;
    } else {
      return -1;
    }
  } else if (node->getNodeType() == AST::PortProperty) {
    std::shared_ptr<PortPropertyNode> portProp =
        static_pointer_cast<PortPropertyNode>(node);
    if (portProp->getPortName() == "size" ||
        portProp->getPortName() == "rate" ||
        portProp->getPortName() == "domain") {
      return 1;
    } else {
      qDebug()
          << "Unknown port property in getNodeNumOutputs() setting size to 1";
      return 1;
    }
  } else if (node->getNodeType() == AST::BundleDeclaration) {
    QList<LangError> errors;
    return CodeValidator::getBlockDeclaredSize(
        static_pointer_cast<DeclarationNode>(node), scope, tree, errors);
  } else if (node->getNodeType() == AST::BundleDeclaration) {
    QList<LangError> errors;
    return CodeValidator::getBlockDeclaredSize(
        static_pointer_cast<DeclarationNode>(node), scope, tree, errors);
  }
  return -100;
}

int CodeValidator::getNodeNumInputs(ASTNode node, ScopeStack scope,
                                    ASTNode tree, QList<LangError> &errors) {
  if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        static_pointer_cast<FunctionNode>(node);
    std::shared_ptr<DeclarationNode> platformFunc =
        CodeValidator::findDeclaration(QString::fromStdString(func->getName()),
                                       scope, tree);
    int dataSize =
        CodeValidator::getFunctionDataSize(func, scope, tree, errors);
    if (platformFunc) {
      if (platformFunc->getObjectType() == "reaction") {
        return 1; // Reactions always have one input as main port for trigger
      } else {
        ASTNode subScope = CodeValidator::getBlockSubScope(platformFunc);
        if (subScope) {
          scope.push_back({platformFunc->getName(), subScope->getChildren()});
        }
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
    int leftSize = CodeValidator::getNodeNumInputs(left, scope, tree, errors);
    return leftSize;
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> block =
        findDeclaration(QString::fromStdString(name->getName()), scope, tree);
    if (block) {
      return getTypeNumInputs(block, scope, tree, errors);
    } else {
      return -1;
    }
  } else if (node->getNodeType() == AST::Bundle) {
    return getBundleSize(static_cast<BundleNode *>(node.get()), scope, tree,
                         errors);
  } else if (node->getNodeType() == AST::List) {
    int size = 0;
    for (ASTNode member : node->getChildren()) {
      size += CodeValidator::getNodeNumInputs(member, scope, tree, errors);
    }
    return size;
  } else if (node->getNodeType() == AST::PortProperty) {
    qDebug() << "Unexpected write to port portperty";
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    return getTypeNumInputs(std::static_pointer_cast<DeclarationNode>(node),
                            scope, tree, errors);
  } else {
    return 0;
  }
  return -1;
}

int CodeValidator::getTypeNumOutputs(
    std::shared_ptr<DeclarationNode> blockDeclaration, const ScopeStack &scope,
    ASTNode tree, QList<LangError> &errors) {
  if (blockDeclaration->getNodeType() == AST::BundleDeclaration) {
    return getBlockDeclaredSize(blockDeclaration, scope, tree, errors);
  } else if (blockDeclaration->getNodeType() == AST::Declaration) {
    if (blockDeclaration->getObjectType() == "module") {
      std::shared_ptr<DeclarationNode> portBlock =
          getMainOutputPortBlock(blockDeclaration);

      BlockNode *outputName = nullptr;
      if (portBlock) {
        if (portBlock->getPropertyValue("block")->getNodeType() == AST::Block) {
          outputName = static_cast<BlockNode *>(
              portBlock->getPropertyValue("block").get());
        } else {
          qDebug() << "WARNING: Expecting name node for output block";
        }
      }
      if (!outputName || outputName->getNodeType() == AST::None) {
        return 0;
      }
      ASTNode subScope = CodeValidator::getBlockSubScope(
          static_pointer_cast<DeclarationNode>(blockDeclaration));
      if (subScope) {
        ListNode *blockList = static_cast<ListNode *>(subScope.get());
        Q_ASSERT(blockList->getNodeType() == AST::List);
        Q_ASSERT(outputName->getNodeType() == AST::Block);
        QString outputBlockName = QString::fromStdString(outputName->getName());
        foreach (ASTNode internalDeclarationNode, blockList->getChildren()) {
          if (internalDeclarationNode->getNodeType() ==
                  AST::BundleDeclaration ||
              internalDeclarationNode->getNodeType() == AST::Declaration) {
            QString blockName = QString::fromStdString(
                static_cast<DeclarationNode *>(internalDeclarationNode.get())
                    ->getName());
            if (blockName == outputBlockName) {
              std::shared_ptr<DeclarationNode> intBlock =
                  static_pointer_cast<DeclarationNode>(internalDeclarationNode);
              if (intBlock->getName() == outputBlockName.toStdString()) {
                if (internalDeclarationNode->getNodeType() ==
                    AST::BundleDeclaration) {
                  return getBlockDeclaredSize(intBlock, scope, tree, errors);
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
        return static_pointer_cast<ValueNode>(sizeNode)->getIntValue();
      }
    }
    return 1;
  }
  return 0;
}

int CodeValidator::getTypeNumInputs(
    std::shared_ptr<DeclarationNode> blockDeclaration, const ScopeStack &scope,
    ASTNode tree, QList<LangError> &errors) {
  if (blockDeclaration->getNodeType() == AST::BundleDeclaration) {
    return getBlockDeclaredSize(blockDeclaration, scope, tree, errors);
  } else if (blockDeclaration->getNodeType() == AST::Declaration) {
    if (blockDeclaration->getObjectType() == "module" ||
        blockDeclaration->getObjectType() == "loop") {
      ASTNode subScope = CodeValidator::getBlockSubScope(
          static_pointer_cast<DeclarationNode>(blockDeclaration));
      if (subScope) {
        ListNode *blockList = static_cast<ListNode *>(subScope.get());
        BlockNode *inputName = nullptr;

        std::shared_ptr<DeclarationNode> portBlock =
            getMainInputPortBlock(blockDeclaration);
        if (portBlock) {
          if (portBlock->getPropertyValue("block")->getNodeType() ==
              AST::Block) {
            inputName = static_cast<BlockNode *>(
                portBlock->getPropertyValue("block").get());
          } else {
            qDebug() << "WARNING: Expecting name node for input block";
          }
        }
        Q_ASSERT(blockList->getNodeType() == AST::List);
        if (!inputName || inputName->getNodeType() == AST::None) {
          return 0;
        }
        Q_ASSERT(inputName->getNodeType() == AST::Block);
        QString inputBlockName = QString::fromStdString(inputName->getName());
        foreach (ASTNode internalDeclarationNode, blockList->getChildren()) {
          //                Q_ASSERT(internalDeclarationNode->getNodeType() ==
          //                AST::BundleDeclaration ||
          //                internalDeclarationNode->getNodeType() ==
          //                AST::Declaration);
          if (!internalDeclarationNode) {
            return -1;
          }
          if (internalDeclarationNode->getNodeType() == AST::Declaration ||
              internalDeclarationNode->getNodeType() ==
                  AST::BundleDeclaration) {
            QString blockName = QString::fromStdString(
                static_cast<DeclarationNode *>(internalDeclarationNode.get())
                    ->getName());
            if (blockName == inputBlockName) {
              if (internalDeclarationNode->getNodeType() ==
                  AST::BundleDeclaration) {
                std::shared_ptr<DeclarationNode> intBlock =
                    static_pointer_cast<DeclarationNode>(
                        internalDeclarationNode);
                Q_ASSERT(intBlock->getNodeType() == AST::BundleDeclaration);
                return getBlockDeclaredSize(intBlock, scope, tree, errors);
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
      int size = getNodeSize(blockDeclaration, scope, tree);
      int internalSize = 1;
      auto typeDeclaration = findTypeDeclarationByName(
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

std::vector<std::shared_ptr<DeclarationNode>>
CodeValidator::findAllDeclarations(std::string objectName,
                                   const ScopeStack &scopeStack, ASTNode tree,
                                   vector<string> namespaces) {
  std::vector<std::shared_ptr<DeclarationNode>> decls;
  vector<string> scopesList;
  istringstream iss(objectName);
  copy(istream_iterator<string>(iss), istream_iterator<string>(),
       back_inserter(scopesList));

  if (objectName.size() > 0) {
    objectName = scopesList.back();
    scopesList.pop_back();
    for (string ns : namespaces) {
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
            static_pointer_cast<DeclarationNode>(scopeNode);
        std::string name = decl->getName();
        if (name == objectName) {
          auto frameworkNode = decl->getCompilerProperty("framework");
          std::string frameworkName;
          if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
            frameworkName =
                static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
          }
          // Force matching framework.
          if (CodeValidator::namespaceMatch(scopesList, decl, frameworkName)) {
            decls.push_back(decl);
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
            static_pointer_cast<DeclarationNode>(node);
        std::string name = decl->getName();
        if (name == objectName) {

          auto frameworkNode = decl->getCompilerProperty("framework");
          std::string frameworkName;
          if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
            frameworkName =
                static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
          }
          if (CodeValidator::namespaceMatch(scopesList, decl, frameworkName)) {
            decls.push_back(decl);
          }
        }
      }
    }
  }
  return decls;
}

std::shared_ptr<DeclarationNode> CodeValidator::findDeclaration(
    std::string objectName, const ScopeStack &scopeStack, ASTNode tree,
    vector<string> namespaces, std::string platform) {
  vector<string> scopesList;
  istringstream iss(objectName);
  copy(istream_iterator<string>(iss), istream_iterator<string>(),
       back_inserter(scopesList));

  if (objectName.size() > 0) {
    objectName = scopesList.back();
    scopesList.pop_back();
    for (string ns : namespaces) {
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
            static_pointer_cast<DeclarationNode>(scopeNode);
        std::string name = decl->getName();
        if (name == objectName) {
          if (CodeValidator::namespaceMatch(scopesList, decl, platform)) {
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
            static_pointer_cast<DeclarationNode>(node);
        std::string name = decl->getName();
        if (name == objectName) {

          auto frameworkNode = decl->getCompilerProperty("framework");
          std::string frameworkName;
          if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
            frameworkName =
                static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
          }
          if (CodeValidator::namespaceMatch(scopesList, decl, platform)) {
            if (platform.size() > 0) {
              auto platformNode = decl->getCompilerProperty("framework");
              if (platformNode && platformNode->getNodeType() == AST::String) {
                auto platformString =
                    static_pointer_cast<ValueNode>(platformNode)
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
  return nullptr;
}

std::shared_ptr<DeclarationNode>
CodeValidator::findDeclaration(QString objectName, const ScopeStack &scopeStack,
                               ASTNode tree, vector<string> namespaceList,
                               string framework) {
  return findDeclaration(objectName.toStdString(), scopeStack, tree,
                         namespaceList, framework);
}

std::shared_ptr<DeclarationNode> CodeValidator::getDeclaration(ASTNode node) {
  if (node->getNodeType() == AST::Int || node->getNodeType() == AST::Real ||
      node->getNodeType() == AST::String) {
    return nullptr;
  }
  return static_pointer_cast<DeclarationNode>(
      node->getCompilerProperty("declaration"));
}

std::string CodeValidator::streamMemberName(ASTNode node) {
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
    for (auto elem : node->getChildren()) {
      auto elemName = streamMemberName(elem);
      if (listCommonName.size() == 0) {
        listCommonName = elemName;
      } else if (elemName != listCommonName) {
        //                qDebug() << "List name mismatch";
        return std::string();
      }
    }
    return listCommonName;
  } else if (node->getNodeType() == AST::Declaration) {
    auto decl = static_pointer_cast<DeclarationNode>(node);
    return decl->getName();
  } else {
    //        qDebug() << "streamMemberName() error. Invalid stream member
    //        type.";
  }
  return std::string();
}

PortType CodeValidator::resolveBundleType(BundleNode *bundle,
                                          ScopeStack scopeStack, ASTNode tree) {
  QString bundleName = QString::fromStdString(bundle->getName());
  std::shared_ptr<DeclarationNode> declaration =
      findDeclaration(bundleName, scopeStack, tree);
  if (declaration) {
    if (declaration->getObjectType() == "constant") {
      std::shared_ptr<PropertyNode> property =
          CodeValidator::findPropertyByName(declaration->getProperties(),
                                            "value");
      if (property) {
        return resolveNodeOutType(property->getValue(), scopeStack, tree);
      }
    } else {
      //            return QString::fromStdString(declaration->getObjectType());
    }
  }
  return None;
}

PortType CodeValidator::resolveNameType(BlockNode *name, ScopeStack scopeStack,
                                        ASTNode tree) {
  QString nodeName = QString::fromStdString(name->getName());
  std::shared_ptr<DeclarationNode> declaration =
      findDeclaration(nodeName, scopeStack, tree);
  if (declaration) {
    if (declaration->getObjectType() == "constant") {
      vector<std::shared_ptr<PropertyNode>> properties =
          declaration->getProperties();
      std::shared_ptr<PropertyNode> property =
          CodeValidator::findPropertyByName(properties, "value");
      if (property) {
        return resolveNodeOutType(property->getValue(), scopeStack, tree);
      }
    } else if (declaration->getObjectType() == "signal") {
      vector<std::shared_ptr<PropertyNode>> properties =
          declaration->getProperties();
      std::shared_ptr<PropertyNode> property =
          CodeValidator::findPropertyByName(properties, "default");
      PortType defaultType =
          resolveNodeOutType(property->getValue(), scopeStack, tree);
      if (defaultType == ConstReal) {
        return Signal;
        //            } else if (defaultType == ConstInt) {
        //                return AudioInteger;
      } else {
        return Signal; // TODO this should be separated into SRP and SIP?R
      }
    } else {
      return Object;
    }
  }
  return None;
}

PortType CodeValidator::resolveNodeOutType(ASTNode node, ScopeStack scopeStack,
                                           ASTNode tree) {
  if (node->getNodeType() == AST::Int) {
    return ConstInt;
  } else if (node->getNodeType() == AST::Real) {
    return ConstReal;
  } else if (node->getNodeType() == AST::Switch) {
    return ConstBoolean;
  } else if (node->getNodeType() == AST::String) {
    return ConstString;
  } else if (node->getNodeType() == AST::List) {
    return resolveListType(static_cast<ListNode *>(node.get()), scopeStack,
                           tree);
  } else if (node->getNodeType() == AST::Bundle) {
    return resolveBundleType(static_cast<BundleNode *>(node.get()), scopeStack,
                             tree);
  } else if (node->getNodeType() == AST::Expression) {
    return resolveExpressionType(static_cast<ExpressionNode *>(node.get()),
                                 scopeStack, tree);
  } else if (node->getNodeType() == AST::Block) {
    return resolveNameType(static_cast<BlockNode *>(node.get()), scopeStack,
                           tree);
  } else if (node->getNodeType() == AST::Range) {
    return resolveRangeType(static_cast<RangeNode *>(node.get()), scopeStack,
                            tree);
  } else if (node->getNodeType() == AST::PortProperty) {
    return PortProperty;
    //        return resolvePortPropertyType(static_cast<PortPropertyNode
    //        *>(node.get()), scope, tree);
  }
  return None;
}

PortType CodeValidator::resolveListType(ListNode *listnode,
                                        ScopeStack scopeStack, ASTNode tree) {
  QVector<ASTNode> members =
      QVector<ASTNode>::fromStdVector(listnode->getChildren());
  if (members.isEmpty()) {
    return None;
  }
  ASTNode firstMember = members.takeFirst();
  PortType type = resolveNodeOutType(firstMember, scopeStack, tree);

  for (ASTNode member : members) {
    PortType nextPortType = resolveNodeOutType(member, scopeStack, tree);
    if (type != nextPortType) {
      if (type == ConstInt &&
          nextPortType == ConstReal) { // List becomes Real if Real found
        type = ConstReal;
      } else if (type == ConstReal &&
                 nextPortType == ConstInt) { // Int in Real list
                                             // Nothing here for now
      } else {                               // Invalid combination
        return Invalid;
      }
    }
  }

  return type;
}

PortType CodeValidator::resolveExpressionType(ExpressionNode *exprnode,
                                              ScopeStack scopeStack,
                                              ASTNode tree) {
  if (!exprnode->isUnary()) {
    ASTNode left = exprnode->getLeft();
    ASTNode right = exprnode->getRight();
    PortType leftType = resolveNodeOutType(left, scopeStack, tree);
    PortType rightType = resolveNodeOutType(right, scopeStack, tree);
    if (leftType == rightType) {
      return leftType;
    }
    // TODO implement toleraces between ints and reals

  } else {
    // TODO implement for unary
  }
  return None;
}

PortType CodeValidator::resolveRangeType(RangeNode *rangenode,
                                         ScopeStack scopeStack, ASTNode tree) {
  PortType leftType =
      resolveNodeOutType(rangenode->startIndex(), scopeStack, tree);
  PortType rightType =
      resolveNodeOutType(rangenode->endIndex(), scopeStack, tree);
  if (leftType == rightType) {
    return leftType;
  }
  return None;
}

PortType CodeValidator::resolvePortPropertyType(PortPropertyNode *portproperty,
                                                ScopeStack scopeStack,
                                                ASTNode tree) {
  // FIXME implement correctly
  if (portproperty->getPortName() == "size") {
    return ConstInt;
  } else {
    return ConstReal;
  }
}

shared_ptr<DeclarationNode> CodeValidator::resolveBlock(ASTNode node,
                                                        bool downStream) {
  if (!node) {
    return nullptr;
  }
  if (node->getNodeType() == AST::Declaration) { // Signal
    return static_pointer_cast<DeclarationNode>(node);
  } else if (node->getNodeType() == AST::Function) {
    auto funcDecl = static_pointer_cast<DeclarationNode>(
        node->getCompilerProperty("declaration"));
    if (funcDecl) {
      auto ports = funcDecl->getPropertyValue("ports");
      auto portDeclBlock = CodeValidator::getMainOutputPortBlock(funcDecl);
      if (portDeclBlock) {
        auto portBlock = portDeclBlock->getPropertyValue("block");
        if (portBlock) {
          auto funcName = funcDecl->getName();
          auto portBlockDecl = CodeValidator::findDeclaration(
              CodeValidator::streamMemberName(portBlock),
              {{funcName, static_pointer_cast<DeclarationNode>(funcDecl)
                              ->getPropertyValue("blocks")
                              ->getChildren()}},
              nullptr);
          if (portBlockDecl) {
            auto blockDomain =
                CodeValidator::getNodeDomain(portBlockDecl, {}, nullptr);
            if (blockDomain &&
                blockDomain->getNodeType() == AST::PortProperty) {
              auto portName =
                  static_pointer_cast<PortPropertyNode>(blockDomain)->getName();
              Q_ASSERT(static_pointer_cast<PortPropertyNode>(blockDomain)
                           ->getPortName() == "domain");
              // Now match the port name from the domain to the port declaration
              for (auto port : ports->getChildren()) {
                if (port->getNodeType() == AST::Declaration) {
                  auto portDecl = static_pointer_cast<DeclarationNode>(port);
                  if (portDecl->getName() ==
                      portName) { // Port match. Now get outer node
                    auto portType = static_pointer_cast<DeclarationNode>(port)
                                        ->getObjectType();
                    // TODO connect all port types
                    if (portType == "mainOutputPort" && downStream) {
                      auto outerBlock =
                          node->getCompilerProperty("outputBlock");
                      return resolveBlock(outerBlock, true);
                    } else if (portType == "mainInputPort" && !downStream) {
                      auto outerBlock = node->getCompilerProperty("inputBlock");
                      return resolveBlock(outerBlock, false);
                    }
                  }
                }
              }
            } else {
              qDebug() << "Unexpected port block domain";
            }
          }
        }
      }
    }
  }
  return shared_ptr<DeclarationNode>();
}

ASTNode CodeValidator::resolveDomain(ASTNode node, ScopeStack scopeStack,
                                     ASTNode tree, bool downStream) {
  auto blockDecl = resolveBlock(node, downStream);
  if (blockDecl) {
    return CodeValidator::getNodeDomain(blockDecl, scopeStack, tree);
  } else {
    return CodeValidator::getNodeDomain(node, scopeStack, tree);
  }
}

double CodeValidator::resolveRate(ASTNode node, ScopeStack scopeStack,
                                  ASTNode tree, bool downStream) {
  auto blockDecl = resolveBlock(node, downStream);
  if (blockDecl) {
    return CodeValidator::getNodeRate(blockDecl, scopeStack, tree);
  } else {
    return CodeValidator::getNodeRate(node, scopeStack, tree);
  }
}

int CodeValidator::evaluateConstInteger(ASTNode node, ScopeStack scope,
                                        ASTNode tree,
                                        QList<LangError> &errors) {
  int result = 0;
  if (node->getNodeType() == AST::Int) {
    return static_cast<ValueNode *>(node.get())->getIntValue();
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    ListNode *indexList = bundle->index().get();
    if (indexList->size() == 1) {
      int index = evaluateConstInteger(indexList->getChildren().at(0), scope,
                                       tree, errors);

      QString bundleName = QString::fromStdString(bundle->getName());
      std::shared_ptr<DeclarationNode> declaration =
          findDeclaration(bundleName, scope, tree);
      if (declaration && declaration->getNodeType() == AST::BundleDeclaration) {
        ASTNode member =
            getMemberfromBlockBundle(declaration.get(), index, errors);
        return evaluateConstInteger(member, scope, tree, errors);
      }
    }
    LangError error;
    error.type = LangError::InvalidType;
    error.lineNumber = bundle->getLine();
    error.errorTokens.push_back(bundle->getName());
    errors << error;
  } else if (node->getNodeType() == AST::Block) {
    QString name =
        QString::fromStdString(static_cast<BlockNode *>(node.get())->getName());
    std::shared_ptr<DeclarationNode> declaration =
        findDeclaration(name, scope, tree);
    if (declaration->getObjectType() == "constant") {
      return evaluateConstInteger(declaration->getPropertyValue("value"), scope,
                                  tree, errors);
    }
  } else if (node->getNodeType() == AST::Expression) {
    // TODO: check expression out
  } else {
    LangError error;
    error.type = LangError::InvalidType;
    error.lineNumber = node->getLine();
    error.errorTokens.push_back(
        getPortTypeName(resolveNodeOutType(node, scope, tree)).toStdString());
    errors << error;
  }
  return result;
}

double CodeValidator::evaluateConstReal(ASTNode node, ScopeStack scope,
                                        ASTNode tree,
                                        QList<LangError> &errors) {
  double result = 0;
  if (node->getNodeType() == AST::Real) {
    return static_cast<ValueNode *>(node.get())->getRealValue();
  } else if (node->getNodeType() == AST::Int) {
    return static_cast<ValueNode *>(node.get())->getIntValue();
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    QString bundleName = QString::fromStdString(bundle->getName());
    std::shared_ptr<DeclarationNode> declaration =
        findDeclaration(bundleName, scope, tree);
    int index = evaluateConstInteger(bundle->index(), scope, tree, errors);
    if (declaration && declaration->getNodeType() == AST::BundleDeclaration) {
      ASTNode member =
          getMemberfromBlockBundle(declaration.get(), index, errors);
      return evaluateConstReal(member, scope, tree, errors);
    }
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *blockNode = static_cast<BlockNode *>(node.get());
    QString name = QString::fromStdString(blockNode->getName());
    std::shared_ptr<DeclarationNode> declaration =
        findDeclaration(name, scope, tree, blockNode->getNamespaceList());
    if (!declaration) {
      LangError error;
      error.type = LangError::UndeclaredSymbol;
      error.lineNumber = node->getLine();
      //            error.errorTokens.push_back(blockNode->getName());
      //            error.errorTokens.push_back(blockNode->getNamespace());
      string blockName = "";
      if (blockNode->getScopeLevels()) {
        for (unsigned int i = 0; i < blockNode->getScopeLevels(); i++) {
          blockName += blockNode->getScopeAt(i);
          blockName += "::";
        }
      }
      blockName += blockNode->getName();
      error.errorTokens.push_back(blockName);
      errors << error;
    }
    if (declaration && declaration->getNodeType() == AST::Declaration) {
      ASTNode value = getValueFromConstBlock(declaration.get());
      if (value->getNodeType() == AST::Int ||
          value->getNodeType() == AST::Real) {
        return static_cast<ValueNode *>(value.get())->toReal();
      } else {
        // Do something?
      }
    }
  } else {
    LangError error;
    error.type = LangError::InvalidType;
    error.lineNumber = node->getLine();
    error.errorTokens.push_back(
        getPortTypeName(resolveNodeOutType(node, scope, tree)).toStdString());
    errors << error;
  }
  return result;
}

std::string CodeValidator::evaluateConstString(ASTNode node, ScopeStack scope,
                                               ASTNode tree,
                                               QList<LangError> &errors) {
  std::string result;
  if (node->getNodeType() == AST::String) {
    return static_pointer_cast<ValueNode>(node)->getStringValue();
  } else if (node->getNodeType() == AST::Bundle) {
    auto bundle = static_pointer_cast<BundleNode>(node);
    QString bundleName = QString::fromStdString(bundle->getName());
    std::shared_ptr<DeclarationNode> declaration =
        findDeclaration(bundleName, scope, tree);
    int index = evaluateConstInteger(bundle->index(), scope, tree, errors);
    if (declaration && declaration->getNodeType() == AST::BundleDeclaration) {
      ASTNode member =
          getMemberfromBlockBundle(declaration.get(), index, errors);
      return evaluateConstString(member, scope, tree, errors);
    }
  } else if (node->getNodeType() == AST::Block) {
    auto blockNode = static_pointer_cast<BlockNode>(node);
    QString name = QString::fromStdString(blockNode->getName());
    std::shared_ptr<DeclarationNode> declaration =
        findDeclaration(name, scope, tree, blockNode->getNamespaceList());
    if (!declaration) {
      LangError error;
      error.type = LangError::UndeclaredSymbol;
      error.lineNumber = node->getLine();
      //            error.errorTokens.push_back(blockNode->getName());
      //            error.errorTokens.push_back(blockNode->getNamespace());
      string blockName = "";
      if (blockNode->getScopeLevels()) {
        for (unsigned int i = 0; i < blockNode->getScopeLevels(); i++) {
          blockName += blockNode->getScopeAt(i);
          blockName += "::";
        }
      }
      blockName += blockNode->getName();
      error.errorTokens.push_back(blockName);
      errors << error;
    }
    if (declaration && declaration->getNodeType() == AST::Declaration) {
      ASTNode value = getValueFromConstBlock(declaration.get());
      if (value) {
        if (value->getNodeType() == AST::String ||
            value->getNodeType() == AST::Int ||
            value->getNodeType() == AST::Real) {
          return static_pointer_cast<ValueNode>(value)->toString();
        }
      } else {
        // Do something?
      }
    }
  } else if (node->getNodeType() == AST::PortProperty) {
    PortPropertyNode *propertyNode =
        static_cast<PortPropertyNode *>(node.get());
    std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(
        QString::fromStdString(propertyNode->getName()), scope, tree);
    if (block) {
      ASTNode propertyValue =
          block->getPropertyValue(propertyNode->getPortName());
      if (propertyValue) {
        //                || propertyValue->getNodeType() == AST::Block ||
        //                propertyValue->getNodeType() == AST::Bundle
        if (propertyValue->getNodeType() == AST::String ||
            propertyValue->getNodeType() == AST::Int ||
            propertyValue->getNodeType() == AST::Real) {
          return static_pointer_cast<ValueNode>(propertyValue)->toString();
        }
      }
    }
  }
  if (node->getNodeType() == AST::String || node->getNodeType() == AST::Int ||
      node->getNodeType() == AST::Real) {
    return static_pointer_cast<ValueNode>(node)->toString();
  } else {
    LangError error;
    error.type = LangError::InvalidType;
    error.lineNumber = node->getLine();
    error.errorTokens.push_back(
        getPortTypeName(resolveNodeOutType(node, scope, tree)).toStdString());
    errors << error;
  }
  return result;
}

ASTNode CodeValidator::getMemberfromBlockBundle(DeclarationNode *block,
                                                int index,
                                                QList<LangError> &errors) {
  ASTNode out = nullptr;
  if (block->getObjectType() == "constant") {
    QVector<std::shared_ptr<PropertyNode>> ports =
        QVector<std::shared_ptr<PropertyNode>>::fromStdVector(
            block->getProperties());
    for (std::shared_ptr<PropertyNode> port : ports) {
      if (port->getName() == "value") {
        ASTNode value = port->getValue();
        if (value->getNodeType() == AST::List) {
          return getMemberFromList(static_cast<ListNode *>(value.get()), index,
                                   errors);
        } else if (value->getNodeType() == AST::Bundle) {
          // TODO: do something here
        }
      }
    }
  } else {
    // TODO: What to do with other cases?
  }
  return out;
}

ASTNode CodeValidator::getValueFromConstBlock(DeclarationNode *block) {
  ASTNode out = nullptr;
  if (block->getObjectType() == "constant") {
    QVector<std::shared_ptr<PropertyNode>> ports =
        QVector<std::shared_ptr<PropertyNode>>::fromStdVector(
            block->getProperties());
    for (std::shared_ptr<PropertyNode> port : ports) {
      if (port->getName() == "value") {
        return port->getValue();
      }
    }
  } else {
    // Should something else be done?
  }
  return out;
}

ASTNode CodeValidator::getMemberFromList(ListNode *node, int index,
                                         QList<LangError> &errors) {
  if (index < 1 || index > (int)node->getChildren().size()) {
    LangError error;
    error.type = LangError::ArrayIndexOutOfRange;
    error.lineNumber = node->getLine();
    error.errorTokens.push_back(QString::number(index).toStdString());
    errors << error;
    return nullptr;
  }
  return node->getChildren()[index - 1];
}

std::shared_ptr<PropertyNode> CodeValidator::findPropertyByName(
    vector<std::shared_ptr<PropertyNode>> properties, QString propertyName) {
  for (auto property : properties) {
    if (property->getName() == propertyName.toStdString()) {
      return property;
    }
  }
  return nullptr;
}

QVector<ASTNode> CodeValidator::validTypesForPort(
    std::shared_ptr<DeclarationNode> typeDeclaration, QString portName,
    ScopeStack scope, ASTNode tree) {
  QVector<ASTNode> validTypes;
  QVector<ASTNode> portList =
      getPortsForTypeBlock(typeDeclaration, scope, tree);
  foreach (ASTNode node, portList) {
    DeclarationNode *portNode = static_cast<DeclarationNode *>(node.get());
    ValueNode *name =
        static_cast<ValueNode *>(portNode->getPropertyValue("name").get());
    Q_ASSERT(name->getNodeType() == AST::String);
    if (name->getStringValue() == portName.toStdString()) {
      ListNode *typesPort =
          static_cast<ListNode *>(portNode->getPropertyValue("types").get());
      Q_ASSERT(typesPort->getNodeType() == AST::List);
      for (ASTNode type : typesPort->getChildren()) {
        validTypes << type;
      }
    }
  }
  return validTypes;
}

std::shared_ptr<DeclarationNode> CodeValidator::findTypeDeclarationByName(
    string typeName, ScopeStack scope, ASTNode tree,
    std::vector<string> namespaces, std::string currentFramework) {
  auto getDecl =
      [&](ASTNode scopeMember, string typeName, std::vector<string> namespaces,
          std::string framework) -> std::shared_ptr<DeclarationNode> {
    std::shared_ptr<DeclarationNode> declarationNode =
        static_pointer_cast<DeclarationNode>(scopeMember);
    if (declarationNode->getObjectType() == "type" ||
        declarationNode->getObjectType() == "platformBlock") {
      ASTNode valueNode = declarationNode->getPropertyValue("typeName");
      if (valueNode && valueNode->getNodeType() == AST::String) {
        auto value = static_pointer_cast<ValueNode>(valueNode);
        if (typeName == value->getStringValue()) {
          if (CodeValidator::namespaceMatch(namespaces, declarationNode,
                                            currentFramework)) {
            return declarationNode;
          }
        }
      }
    } else if (declarationNode->getObjectType() == "platformModule") {
      if (typeName == declarationNode->getName()) {
        if (CodeValidator::namespaceMatch(namespaces, declarationNode,
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

std::shared_ptr<DeclarationNode>
CodeValidator::findTypeDeclaration(std::shared_ptr<DeclarationNode> block,
                                   ScopeStack scope, ASTNode tree,
                                   std::string currentFramework) {
  string typeName = block->getObjectType();
  return CodeValidator::findTypeDeclarationByName(
      typeName, scope, tree, block->getNamespaceList(), currentFramework);
}

std::shared_ptr<DeclarationNode>
CodeValidator::findDomainDeclaration(string domainName, ASTNode tree) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "_domainDefinition") {
        ASTNode domainNameValue = decl->getPropertyValue("domainName");
        if (domainNameValue->getNodeType() == AST::String) {
          if (domainName == static_cast<ValueNode *>(domainNameValue.get())
                                ->getStringValue()) {
            return decl;
          }
        }
      }
    }
  }
  return nullptr;
}

std::shared_ptr<DeclarationNode>
CodeValidator::findDataTypeDeclaration(string dataTypeName, ASTNode tree) {
  for (auto node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      auto nodeDecl = static_pointer_cast<DeclarationNode>(node);
      if (nodeDecl->getObjectType() == "platformDataType") {
        auto typeProp = nodeDecl->getPropertyValue("type");
        if (typeProp && typeProp->getNodeType() == AST::String) {
          if (static_pointer_cast<ValueNode>(typeProp)->getStringValue() ==
              dataTypeName) {
            return nodeDecl;
          }
        }
      }
    }
  }
  return nullptr;
}

QVector<ASTNode> CodeValidator::getPortsForType(string typeName,
                                                ScopeStack scope, ASTNode tree,
                                                std::vector<string> namespaces,
                                                std::string framework) {
  QVector<ASTNode> portList;

  std::shared_ptr<DeclarationNode> typeBlock =
      CodeValidator::findTypeDeclarationByName(typeName, scope, tree,
                                               namespaces, framework);

  if (typeBlock) {
    if (typeBlock->getObjectType() == "type" ||
        typeBlock->getObjectType() == "platformModule" ||
        typeBlock->getObjectType() == "platformBlock") {
      ValueNode *name = static_cast<ValueNode *>(
          typeBlock->getPropertyValue("typeName").get());
      if (name) {
        Q_ASSERT(name->getNodeType() == AST::String);
        Q_ASSERT(name->getStringValue() == typeName);
        QVector<ASTNode> newPortList =
            getPortsForTypeBlock(typeBlock, scope, tree);
        portList << newPortList;
      } else {
        qDebug()
            << "CodeValidator::getPortsForType type missing typeName port.";
      }
    }

    //        QVector<ASTNode > inheritedProperties =
    //        getInheritedPorts(typeBlock, scope, tree); portList <<
    //        inheritedProperties;
  }

  return portList;
}

QVector<ASTNode>
CodeValidator::getInheritedPorts(std::shared_ptr<DeclarationNode> block,
                                 ScopeStack scope, ASTNode tree) {
  QVector<ASTNode> inheritedProperties;
  auto inheritedTypes = CodeValidator::getInheritedTypes(block, scope, tree);
  QStringList inheritedName;
  for (auto typeDeclaration : inheritedTypes) {
    QVector<ASTNode> inheritedFromType =
        getPortsForTypeBlock(typeDeclaration, scope, tree);
    for (ASTNode property : inheritedFromType) {
      if (inheritedProperties.count(property) == 0) {
        inheritedProperties << property;
      }
      if (property->getNodeType() == AST::Declaration) {
        inheritedName << QString::fromStdString(
            std::static_pointer_cast<DeclarationNode>(property)->getName());
      }
    }
  }
  return inheritedProperties;
}

vector<std::shared_ptr<DeclarationNode>>
CodeValidator::getInheritedTypes(std::shared_ptr<DeclarationNode> block,
                                 ScopeStack scope, ASTNode tree) {
  vector<std::shared_ptr<DeclarationNode>> inheritedTypes;
  ASTNode inherits = block->getPropertyValue("inherits");
  if (inherits) {
    if (inherits->getNodeType() == AST::List) {
      for (ASTNode inheritsFromName : inherits->getChildren()) {
        if (inheritsFromName->getNodeType() == AST::String) {
          Q_ASSERT(0 == 1); // Disallowed
        } else if (inheritsFromName->getNodeType() == AST::Block) {
          auto inheritsBlock = static_pointer_cast<BlockNode>(inheritsFromName);
          std::shared_ptr<DeclarationNode> inheritedDeclaration =
              CodeValidator::findDeclaration(
                  QString::fromStdString(inheritsBlock->getName()), scope,
                  tree);
          if (inheritedDeclaration) {
            inheritedTypes.push_back(inheritedDeclaration);
            auto parentTypes = CodeValidator::getInheritedTypes(
                inheritedDeclaration, scope, tree);
            inheritedTypes.insert(inheritedTypes.end(), parentTypes.begin(),
                                  parentTypes.end());
          }
        }
      }
    } else if (inherits->getNodeType() == AST::String) {
      Q_ASSERT(0 == 1); // Disallowed
    } else if (inherits->getNodeType() == AST::Block) {
      auto inheritsBlock = static_pointer_cast<BlockNode>(inherits);
      std::shared_ptr<DeclarationNode> inheritedDeclaration =
          CodeValidator::findDeclaration(
              QString::fromStdString(inheritsBlock->getName()), scope, tree);
      if (inheritedDeclaration) {
        auto parentTypes =
            CodeValidator::getInheritedTypes(inheritedDeclaration, scope, tree);
        inheritedTypes.insert(inheritedTypes.end(), parentTypes.begin(),
                              parentTypes.end());
      }
    } else {
      qDebug() << "Unexpected type for inherits property";
    }
  }
  return inheritedTypes;
}

std::shared_ptr<DeclarationNode> CodeValidator::getMainOutputPortBlock(
    std::shared_ptr<DeclarationNode> moduleBlock) {
  ListNode *ports =
      static_cast<ListNode *>(moduleBlock->getPropertyValue("ports").get());
  if (ports && ports->getNodeType() == AST::List) {
    for (ASTNode port : ports->getChildren()) {
      std::shared_ptr<DeclarationNode> portBlock =
          static_pointer_cast<DeclarationNode>(port);
      if (portBlock->getObjectType() == "mainOutputPort") {
        return portBlock;
      }
    }
  } else if (ports && ports->getNodeType() == AST::None) {
    // If port list is None, then ignore
  } else {
    qDebug() << "ERROR! ports property must be a list or None!";
  }
  return nullptr;
}

std::shared_ptr<DeclarationNode> CodeValidator::getMainInputPortBlock(
    std::shared_ptr<DeclarationNode> moduleBlock) {
  ListNode *ports =
      static_cast<ListNode *>(moduleBlock->getPropertyValue("ports").get());
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
    qDebug() << "ERROR! ports property must be a list or None!";
  }
  return nullptr;
}

std::shared_ptr<DeclarationNode>
CodeValidator::getPort(std::shared_ptr<DeclarationNode> moduleBlock,
                       string name) {
  ListNode *ports =
      static_cast<ListNode *>(moduleBlock->getPropertyValue("ports").get());
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
          qDebug() << "WARNING name found in getPort() but unexpected type";
        }
      }
    }
  } else if (ports->getNodeType() == AST::None) {
    // If port list is None, then ignore
  } else {
    qDebug() << "ERROR! ports property must be a list or None!";
  }
  return nullptr;
}

QVector<ASTNode>
CodeValidator::getPortsForTypeBlock(std::shared_ptr<DeclarationNode> block,
                                    ScopeStack scope, ASTNode tree) {
  ASTNode portsValue = block->getPropertyValue("properties");
  QVector<ASTNode> outList;
  if (portsValue && portsValue->getNodeType() != AST::None) {
    Q_ASSERT(portsValue->getNodeType() == AST::List);
    ListNode *portList = static_cast<ListNode *>(portsValue.get());
    for (ASTNode port : portList->getChildren()) {
      outList << port;
    }
  }

  outList << getInheritedPorts(block, scope, tree);
  return outList;
}

int CodeValidator::numParallelStreams(StreamNode *stream,
                                      StrideSystem &platform,
                                      const ScopeStack &scope, ASTNode tree,
                                      QList<LangError> &errors) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();
  int numParallel = 0;

  int leftSize = 0;
  int rightSize = 0;

  if (left->getNodeType() == AST::Block) {
    leftSize = getNodeSize(left, scope, tree);
  } else if (left->getNodeType() == AST::List) {
    leftSize = getNodeSize(left, scope, tree);
  } else {
    leftSize = getNodeNumOutputs(left, scope, tree, errors);
  }
  if (right->getNodeType() == AST::Block || right->getNodeType() == AST::List) {
    rightSize = getNodeSize(right, scope, tree);
  } else if (right->getNodeType() == AST::Function) {
    int functionNodeSize = getNodeSize(right, scope, tree);
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
      rightSize = getNodeSize(firstMember, scope, tree);
    } else {
      rightSize = getNodeNumInputs(firstMember, scope, tree, errors);
    }
    if (firstMember->getNodeType() == AST::Function) {
      int functionNodeSize = getNodeSize(firstMember, scope, tree);
      if (functionNodeSize == 1) {
        rightSize = getNodeNumInputs(firstMember, scope, tree, errors);
        ;
      } else {
      }
    }
  } else {
    rightSize = getNodeNumInputs(right, scope, tree, errors);
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

int CodeValidator::getNodeSize(ASTNode node, const ScopeStack &scopeStack,
                               ASTNode tree) {
  int size = 1;
  if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    QList<LangError> errors;
    size = getBundleSize(bundle, scopeStack, tree, errors);
    if (errors.size() > 0) {
      return -1;
    }
    Q_ASSERT(errors.size() == 0);
    return size;
  } else if (node->getNodeType() == AST::Expression) {
    ExpressionNode *expr = static_cast<ExpressionNode *>(node.get());
    if (expr->isUnary()) {
      return getNodeSize(expr->getValue(), scopeStack, tree);
    } else {
      int leftSize = getNodeSize(expr->getLeft(), scopeStack, tree);
      int rightSize = getNodeSize(expr->getLeft(), scopeStack, tree);
      if (leftSize == rightSize) {
        return leftSize;
      } else {
        return -1;
      }
    }
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *blockNode = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> block = findDeclaration(
        QString::fromStdString(blockNode->getName()), scopeStack, tree);
    if (!block) {
      size = -1; // Block not declared
    } else if (block->getNodeType() == AST::BundleDeclaration) {
      QList<LangError> errors;
      size = getBlockDeclaredSize(block, {}, tree, errors);
    } else if (block->getNodeType() == AST::Declaration) {
      size = 1;
    } else {
      Q_ASSERT(0 == 1);
    }
  } else if (node->getNodeType() == AST::Function) {
    vector<std::shared_ptr<PropertyNode>> properties =
        static_cast<FunctionNode *>(node.get())->getProperties();
    QList<LangError> errors;
    size = getLargestPropertySize(properties, {}, tree, errors);
  } else if (node->getNodeType() == AST::List) {
    size = node->getChildren().size();
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *st = static_cast<StreamNode *>(node.get());
    size = getNodeSize(st->getLeft(), scopeStack, tree);
  }

  return size;
}

std::vector<string> CodeValidator::getModulePropertyNames(
    std::shared_ptr<DeclarationNode> blockDeclaration) {
  std::vector<string> portNames;
  if (blockDeclaration->getObjectType() == "module") {
    auto portsList = static_pointer_cast<ListNode>(
        blockDeclaration->getPropertyValue("ports"));
    if (portsList->getNodeType() == AST::List) {
      for (ASTNode portDeclaration : portsList->getChildren()) {
        if (portDeclaration->getNodeType() == AST::Declaration) {
          auto port = static_pointer_cast<DeclarationNode>(portDeclaration);
          ASTNode nameProperty = port->getPropertyValue("name");
          if (nameProperty) {
            Q_ASSERT(nameProperty->getNodeType() == AST::String);
            if (nameProperty->getNodeType() == AST::String) {
              portNames.push_back(
                  static_cast<ValueNode *>(nameProperty.get())->toString());
            }
          }
        }
      }
    }
  } else if (blockDeclaration->getObjectType() == "platformModule") {
    auto portsList = static_pointer_cast<ListNode>(
        blockDeclaration->getPropertyValue("ports"));
    if (portsList->getNodeType() == AST::List) {
      for (ASTNode portDeclaration : portsList->getChildren()) {
        if (portDeclaration->getNodeType() == AST::Declaration) {
          auto port = static_pointer_cast<DeclarationNode>(portDeclaration);
          ASTNode nameProperty = port->getPropertyValue("name");
          if (nameProperty) {
            Q_ASSERT(nameProperty->getNodeType() == AST::String);
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

QString CodeValidator::getPortTypeName(PortType type) {
  switch (type) {
  case Signal:
    return "signal";
  case ConstReal:
    return "CRP";
  case ConstInt:
    return "CIP";
  case ConstBoolean:
    return "CBP";
  case ConstString:
    return "CSP";
  case Object:
    return "Object";
  case None:
    return "none";
  case Invalid:
    return "";
  }
  return "";
}

ASTNode CodeValidator::getNodeDomain(ASTNode node, ScopeStack scopeStack,
                                     ASTNode tree) {
  ASTNode domainNode = nullptr;
  if (!node) {
    return nullptr;
  }

  if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(name->getName(), scopeStack, tree,
                                       name->getNamespaceList());

    if (declaration) {
      auto typeDeclaration =
          CodeValidator::findTypeDeclaration(declaration, scopeStack, tree);
      if (typeDeclaration &&
          typeDeclaration->getObjectType() == "platformModule") {
        domainNode = name->getCompilerProperty("domain");
      } else {
        domainNode = declaration->getDomain();
      }
    }
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *name = static_cast<BundleNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(name->getName(), scopeStack, tree,
                                       name->getNamespaceList());
    if (declaration) {
      domainNode = declaration->getDomain();
    }
  } else if (node->getNodeType() == AST::List) {
    std::vector<std::string> domainList;
    std::string tempDomainName;
    for (ASTNode member : node->getChildren()) {
      if (member->getNodeType() == AST::Block) {
        BlockNode *name = static_cast<BlockNode *>(member.get());
        std::shared_ptr<DeclarationNode> declaration =
            CodeValidator::findDeclaration(name->getName(), scopeStack, tree,
                                           name->getNamespaceList());
        if (declaration) {
          tempDomainName =
              CodeValidator::getNodeDomainName(declaration, scopeStack, tree);
        }
      } else if (member->getNodeType() == AST::Bundle) {
        BundleNode *name = static_cast<BundleNode *>(member.get());
        std::shared_ptr<DeclarationNode> declaration =
            CodeValidator::findDeclaration(
                QString::fromStdString(name->getName()), scopeStack, tree);
        if (declaration) {
          tempDomainName =
              CodeValidator::getNodeDomainName(declaration, scopeStack, tree);
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
            CodeValidator::getNodeDomainName(member, scopeStack, tree);
      }
      domainList.push_back(tempDomainName);
    }
    bool allEqual = true;
    for (unsigned int i = 1; i < domainList.size(); i++) {
      if (domainList[i - 1] != domainList[i]) {
        allEqual = false;
      }
    }
    if (allEqual && node->getChildren().size() > 0) {
      return CodeValidator::getNodeDomain(node->getChildren().at(0), scopeStack,
                                          tree);
    } else {
      qDebug() << "Non homogenous list";
      return CodeValidator::getNodeDomain(node->getChildren().at(0), scopeStack,
                                          tree);
    }
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    auto decl = static_cast<DeclarationNode *>(node.get());
    if (decl->getObjectType() == "reaction") {
      decl->getCompilerProperty("triggerDomain");

    } else {
      domainNode = decl->getDomain();
    }
  } else if (node->getNodeType() == AST::Function) {
    domainNode = node->getCompilerProperty(
        "domain"); // static_cast<FunctionNode *>(node.get())->getDomain();
    if (!domainNode) {
      auto funcDecl = CodeValidator::findDeclaration(
          static_pointer_cast<FunctionNode>(node)->getName(), scopeStack, tree,
          node->getNamespaceList());
      if (funcDecl && funcDecl->getObjectType() == "platformModule") {
        domainNode = funcDecl->getPropertyValue("domain");
        auto domainDecl = CodeValidator::findDomainDeclaration(
            CodeValidator::streamMemberName(domainNode), tree);
        if (domainDecl) {
          auto parentDomain = domainDecl->getPropertyValue("parentDomain");
          if (parentDomain && parentDomain->getNodeType() == AST::Block) {
            auto domainInstanceCountNode =
                parentDomain->getCompilerProperty("instances");
            if (!domainInstanceCountNode) {
              domainInstanceCountNode =
                  std::make_shared<ValueNode>(0, __FILE__, __LINE__);
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
          CodeValidator::getNodeDomain(expr->getValue(), scopeStack, tree);
    } else {
      return expr->getCompilerProperty("samplingDomain");
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
    domainNode = static_pointer_cast<ValueNode>(node)->getDomain();
  }

  return domainNode;
}

string CodeValidator::getNodeDomainName(ASTNode node, ScopeStack scopeStack,
                                        ASTNode tree) {
  std::string domainName;
  ASTNode domainNode = CodeValidator::getNodeDomain(node, scopeStack, tree);
  if (domainNode) {
    domainName =
        CodeValidator::getDomainIdentifier(domainNode, scopeStack, tree);
  }
  return domainName;
}

std::string CodeValidator::getDomainIdentifier(ASTNode domain,
                                               ScopeStack scopeStack,
                                               ASTNode tree) {
  std::string name;
  // To avoid inconsistencies, we rely on the block name rather than the
  // domainName property. This guarantees uniqueness within a scope.
  // TODO we should add scope markers to this identifier to avoid clashes
  if (domain) {
    if (domain->getNodeType() == AST::Block) {
      auto domainBlock = static_pointer_cast<BlockNode>(domain);
      std::shared_ptr<DeclarationNode> domainDeclaration =
          CodeValidator::findDomainDeclaration(domainBlock->getName(), tree);
      if (domainDeclaration) {
        if (domainDeclaration->getObjectType() == "_domainDefinition") {
          name = domainDeclaration->getName();
        } else if (domainDeclaration->getObjectType() == "PlatformDomain") {
          auto domainNameNode = domainDeclaration->getPropertyValue("value");
          name = getDomainIdentifier(domainNameNode, scopeStack, tree);
        }
      }
      auto domainInstanceIndex = domain->getCompilerProperty("domainInstance");
      if (domainInstanceIndex) {
        int index =
            static_pointer_cast<ValueNode>(domainInstanceIndex)->getIntValue();
        name += ":" + std::to_string(index);
      }
    } else if (domain->getNodeType() == AST::Bundle) {
      auto domainBlock = static_pointer_cast<BundleNode>(domain);
      auto domainDeclaration = CodeValidator::findDeclaration(
          QString::fromStdString(domainBlock->getName()), scopeStack, tree);
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
      name = static_pointer_cast<ValueNode>(domain)->getStringValue();
    } else if (domain->getNodeType() == AST::PortProperty) {
      // Should anything be added to the id? Scope?
      auto portProperty = static_pointer_cast<PortPropertyNode>(domain);
      name = portProperty->getName() + "_" + portProperty->getPortName();
    }
  }
  return name;
}
