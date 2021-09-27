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

#include "codevalidator.h"

#include <QDebug>
#include <QVector>
#include <memory>
#include <sstream>

#include "astfunctions.h"
#include "astquery.h"
#include "coderesolver.h"

#include "stridesystem.hpp"

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

std::vector<std::shared_ptr<SystemNode>>
CodeValidator::getSystemNodes(ASTNode tree) {
  //  Q_ASSERT(m_tree);
  std::vector<std::shared_ptr<SystemNode>> platformNodes;
  std::vector<ASTNode> nodes = tree->getChildren();
  for (ASTNode node : nodes) {
    if (node->getNodeType() == AST::Platform) {
      platformNodes.push_back(static_pointer_cast<SystemNode>(node));
    }
  }
  return platformNodes;
}

std::vector<std::shared_ptr<ImportNode>>
CodeValidator::getImportNodes(ASTNode tree) {
  std::vector<std::shared_ptr<ImportNode>> importList;

  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Import) {
      std::shared_ptr<ImportNode> import =
          static_pointer_cast<ImportNode>(node);
      // FIXME add namespace support here (e.g. import
      // Platform::Filters::Filter)
      bool imported = false;
      for (auto importNode : importList) {
        if ((static_pointer_cast<ImportNode>(importNode)->importName() ==
             import->importName()) &&
            (static_pointer_cast<ImportNode>(importNode)->importAlias() ==
             import->importAlias())) {
          imported = true;
          break;
        }
      }
      if (!imported) {
        importList.push_back(import);
      }
    }
  }
  return importList;
}

std::vector<ASTNode> CodeValidator::loadAllInDirectory(string path) {
  std::vector<ASTNode> nodes;
  QStringList libraryFiles =
      QDir(QString::fromStdString(path)).entryList(QStringList() << "*.stride");
  //      if (QFile::exists(QString::fromStdString(platformPath) + "/" +
  //                        importName + ".stride")) {
  //        libraryFiles << "../" + importName + ".stride";
  //      }
  for (QString file : libraryFiles) {
    QString fileName = QDir::cleanPath(QString::fromStdString(path) +
                                       QDir::separator() + file);
    auto newTree =
        ASTFunctions::parseFile(fileName.toLocal8Bit().data(), nullptr);
    if (newTree) {
      auto children = newTree->getChildren();
      nodes.insert(nodes.end(), children.begin(), children.end());
    } else {
      qDebug() << "ERROR importing tree:" << file;
      vector<LangError> errors = ASTFunctions::getParseErrors();
      for (LangError error : errors) {
        qDebug() << QString::fromStdString(error.getErrorText());
      }
    }
  }
  return nodes;
}

std::vector<std::string>
CodeValidator::listAvailableSystems(std::string strideroot) {
  QDir dir(QString::fromStdString(strideroot));
  dir.cd("systems");
  auto entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  std::vector<std::string> outEntries;
  for (auto entry : entries) {
    outEntries.push_back(entry.toStdString());
  }
  return outEntries;
}

void CodeValidator::validatePlatform(ASTNode tree, ScopeStack scopeStack) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Platform) {
      std::shared_ptr<SystemNode> platformNode =
          static_pointer_cast<SystemNode>(node);
    }
  }
}

std::vector<ASTNode> CodeValidator::getBlocksInScope(ASTNode root,
                                                     ScopeStack scopeStack,
                                                     ASTNode tree) {
  std::vector<ASTNode> blocks;
  if (root->getNodeType() == AST::Declaration ||
      root->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> decl =
        static_pointer_cast<DeclarationNode>(root);

    if (decl->getPropertyValue("ports")) {
      for (ASTNode block : decl->getPropertyValue("ports")->getChildren()) {
        blocks.push_back(block);
      }
    }
  } else if (root->getNodeType() == AST::List) {
    vector<ASTNode> elements =
        static_pointer_cast<ListNode>(root)->getChildren();
    for (ASTNode element : elements) {
      auto newBlocks = getBlocksInScope(element, scopeStack, tree);
      blocks.insert(blocks.end(), newBlocks.begin(), newBlocks.end());
    }
  } else if (root->getNodeType() == AST::Block) {
    std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(root);
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclaration(name->getName(), scopeStack, tree);
    if (declaration) {
      blocks = getBlocksInScope(declaration, scopeStack, tree);
    }
  } else if (root->getNodeType() == AST::Bundle) {
    std::shared_ptr<BundleNode> name = static_pointer_cast<BundleNode>(root);
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclaration(name->getName(), scopeStack, tree);
    if (declaration) {
      blocks = getBlocksInScope(declaration, scopeStack, tree);
    }
  } else {
    for (ASTNode child : root->getChildren()) {
      auto newBlocks = getBlocksInScope(child, scopeStack, tree);
      blocks.insert(blocks.end(), newBlocks.begin(), newBlocks.end());
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
  std::string domainFramework;
  auto separatorIndex = domainName.find("::");
  if (separatorIndex != std::string::npos) {
    domainFramework = domainName.substr(0, separatorIndex);
    domainName = domainName.substr(separatorIndex + 2);
  }
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      DeclarationNode *decl = static_cast<DeclarationNode *>(node.get());
      if (decl->getObjectType() == "_domainDefinition") {
        string declDomainName = decl->getName();
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
                ASTQuery::findDeclaration(fwBlock->getName(), {}, tree,
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
        ASTQuery::findDeclaration(block->getName(), scope, tree,
                                  block->getNamespaceList());
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
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        name->getName(), scope, tree, name->getNamespaceList());
    if (declaration) {
      ASTNode rateNode = declaration->getPropertyValue("rate");
      if (rateNode) {
        return CodeValidator::resolveRateToFloat(rateNode, scope, tree);
      }
    }
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        bundle->getName(), scope, tree, bundle->getNamespaceList());
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
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        name->getName(), scope, tree, name->getNamespaceList());
    if (declaration) {
      std::shared_ptr<ValueNode> value =
          std::make_shared<ValueNode>(rate, __FILE__, __LINE__);
      declaration->replacePropertyValue("rate", value);
    }
    return;
  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        bundle->getName(), scope, tree, bundle->getNamespaceList());
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
  auto ports = CodeValidator::getPortsForType(type, scope, tree, namespaces);
  if (ports.size() > 0) {
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
            "trigger" ||
        static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "string" ||
        static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
            "constant") {
      inst = block;
    } else if (static_pointer_cast<DeclarationNode>(block)->getObjectType() ==
               "reaction") {
      inst = block;
    } else {
      qDebug() << "Unexpected declaration in getInstance()";
    }
  } else if (block->getNodeType() == AST::Int ||
             block->getNodeType() == AST::Real ||
             block->getNodeType() == AST::String) {
    inst = block;
  } else {
    std::shared_ptr<DeclarationNode> decl =
        static_pointer_cast<DeclarationNode>(
            block->getCompilerProperty("declaration"));
    if (!decl) {
      decl = ASTQuery::findDeclaration(CodeValidator::streamMemberName(block),
                                       scopeStack, tree);
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
  if (ratePort->getNodeType() == AST::Real ||
      ratePort->getNodeType() == AST::Int) {
    double rate = static_pointer_cast<ValueNode>(ratePort)->toReal();
    return rate;
  }
  return -1;
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
    validateSymbolUniqueness({{nullptr, m_tree->getChildren()}});
    validateStreamSizes(m_tree, {});
    validateRates(m_tree);
    validateConstraints(m_tree);
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
      scopeStack.push_back({decl, blocks});
      for (auto port : ports) {
        QString portName = QString::fromStdString(port->getName());
        // Check if portname is valid
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
              auto decl = ASTQuery::findDeclaration(
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
                    static_cast<ValueNode *>(validType.get())->getStringValue();
                validTypeNames.push_back(typeCode);
              } else if (validType->getNodeType() == AST::Block) {
                auto blockNode = static_pointer_cast<BlockNode>(validType);
                std::shared_ptr<DeclarationNode> declaration =
                    ASTQuery::findDeclaration(blockNode->getName(), scopeStack,
                                              m_tree);
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
    scopeStack.push_back({node, subScope});

    frameworkNode = decl->getCompilerProperty("framework");
    if (frameworkNode) {
      frameworkName =
          std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    // For BundleDeclarations in particular, we need to ignore the bundle when
    // declaring types. The inner bundle has no scope set, and trying to find it
    // will fail if the declaration is scoped....
    for (auto property : decl->getProperties()) {
      if (property->getName() != "constraints") {
        // Ignore constraints streams as they play by different rules
        validateTypes(property->getValue(), scopeStack,
                      decl->getNamespaceList(), frameworkName);
      }
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
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        block->getName(), scopeStack, m_tree, namespaces, currentFramework);
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
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        bundle->getName(), scopeStack, m_tree, namespaces);
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
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        func->getName(), scopeStack, m_tree, func->getNamespaceList());
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
    //    auto frameworkNode = childNode->getCompilerProperty("framework");
    //    std::string frameworkName;
    //    if (frameworkNode) {
    //      frameworkName =
    //          static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    //    }
    validateTypes(childNode, scopeStack, {}, currentFramework);
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
    auto index = bundle->index();
    if (index->getNodeType() != AST::Int &&
        index->getNodeType() != AST::PortProperty &&
        index->getNodeType() != AST::Range &&
        index->getNodeType() != AST::List) {
      LangError error;
      error.type = LangError::InvalidIndexType;
      error.lineNumber = bundle->getLine();
      error.errorTokens.push_back(bundle->getName());
      m_errors << error;
    }
  }
  for (ASTNode child : node->getChildren()) {
    std::vector<ASTNode> blocksInScope = getBlocksInScope(child, scope, m_tree);
    auto subScope = scope;
    subScope.push_back({child, blocksInScope});
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
    //    auto decl = static_pointer_cast<DeclarationNode>(node);
    //    auto subScope = getBlockSubScope(decl);
    //    if (subScope) {
    //      scope.push_back({decl->getName(), subScope->getChildren()});
    //    }
  }

  std::vector<ASTNode> children = node->getChildren();
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
          // Checkt if _at matches
          auto nodeAt = node->getCompilerProperty("_at");
          auto siblingAt = sibling->getCompilerProperty("_at");
          bool atMatches = false;
          if (!nodeAt && !siblingAt) {
            atMatches = true;
          }
          if (nodeAt && siblingAt) {
            if (nodeAt->getNodeType() == AST::Int &&
                siblingAt->getNodeType() == AST::Int) {
              auto nodeAtInt =
                  static_pointer_cast<ValueNode>(nodeAt)->getIntValue();
              auto siblingAtInt =
                  static_pointer_cast<ValueNode>(siblingAt)->getIntValue();
              if (nodeAtInt == siblingAtInt) {
                atMatches = true;
              }
            } else if (nodeAt->getNodeType() == AST::String &&
                       siblingAt->getNodeType() == AST::String) {
              auto nodeAtString =
                  static_pointer_cast<ValueNode>(nodeAt)->getStringValue();
              auto siblingAtString =
                  static_pointer_cast<ValueNode>(siblingAt)->getStringValue();
              if (nodeAtString == siblingAtString) {
                atMatches = true;
              }
            }
          }
          if (duplicateSymbol && atMatches) {
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
        if (decl->getPropertyValue("blocks") &&
            decl->getPropertyValue("ports")) {
          auto blocks = decl->getPropertyValue("blocks")->getChildren();
          auto ports = decl->getPropertyValue("ports")->getChildren();
          std::vector<ASTNode> subScope = blocks;
          subScope.insert(subScope.end(), ports.begin(), ports.end());
          scope.push_back({node, subScope});
          validateSymbolUniqueness(scope);
          scope.pop_back();
        }
      }
    }
  }
}

void CodeValidator::validateStreamSizes(ASTNode tree, ScopeStack scope) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      StreamNode *stream = static_cast<StreamNode *>(node.get());
      validateStreamInputSize(stream, scope, m_errors);
    } else if (node->getNodeType() == AST::Declaration) {
      auto decl = static_pointer_cast<DeclarationNode>(node);
      if (decl) {
        if (decl->getObjectType() == "module" ||
            decl->getObjectType() == "reaction" ||
            decl->getObjectType() == "loop") {
          auto subScope = scope;
          subScope.push_back(
              {decl, decl->getPropertyValue("blocks")->getChildren()});
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

void CodeValidator::validateConstraints(ASTNode tree) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      validateConstraints(std::static_pointer_cast<StreamNode>(node),
                          ScopeStack(), tree);
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

void CodeValidator::validateConstraints(std::shared_ptr<StreamNode> stream,
                                        ScopeStack scopeStack, ASTNode tree) {
  auto node = stream->getLeft();
  while (node) {
    if (node->getNodeType() == AST::Function) {
      validateFunctionConstraints(std::static_pointer_cast<FunctionNode>(node),
                                  scopeStack, tree);
    }

    if (stream == nullptr) {
      node = nullptr;
    } else if (stream->getRight()->getNodeType() == AST::Stream) {
      stream = static_pointer_cast<StreamNode>(stream->getRight());
      node = stream->getLeft();
    } else {
      node = stream->getRight();
      stream = nullptr;
    }
  }
}

void CodeValidator::validateFunctionConstraints(
    std::shared_ptr<FunctionNode> function, ScopeStack scopeStack,
    ASTNode tree) {
  // FIXME use framework when searching for function.
  auto declaration = ASTQuery::findDeclaration(
      function->getName(), scopeStack, tree, function->getNamespaceList());
  if (!declaration) {
    return;
  }
  auto constraintsNode = declaration->getPropertyValue("constraints");
  auto blocks = declaration->getPropertyValue("blocks");

  if (blocks) {
    scopeStack.push_back({nullptr, blocks->getChildren()});
  } else {
    scopeStack.push_back({nullptr, {}});
  }
  if (constraintsNode) {
    for (auto constraint : constraintsNode->getChildren()) {
      if (constraint->getNodeType() == AST::Stream) {
        validateConstraintStream(static_pointer_cast<StreamNode>(constraint),
                                 function, declaration, scopeStack, tree);
      } else {
        qDebug() << "WARNING: Unexpected type in constraint, expecting stream";
      }
    }
  }
}

void CodeValidator::validateConstraintStream(
    std::shared_ptr<StreamNode> stream, std::shared_ptr<FunctionNode> function,
    std::shared_ptr<DeclarationNode> declaration, ScopeStack scopeStack,
    ASTNode tree) {
  auto node = stream->getLeft();
  std::vector<ASTNode> previous;
  while (node) {
    previous = resolveConstraintNode(node, previous, function, declaration,
                                     scopeStack, tree);
    if (!stream) {
      node = nullptr;
    } else {
      node = stream->getRight();
      if (node->getNodeType() == AST::Stream) {
        stream = static_pointer_cast<StreamNode>(stream->getRight());
        node = stream->getLeft();
      } else {
        stream = nullptr;
      }
    }
  }
  if (previous.size() > 0) {
    qDebug() << "ERROR analyzing contraint";
  }
}

std::vector<ASTNode> CodeValidator::processConstraintFunction(
    std::shared_ptr<FunctionNode> constraintFunction,
    std::shared_ptr<FunctionNode> functionInstance, std::vector<ASTNode> input,
    QList<LangError> &errors) {
  if (constraintFunction->getName() == "NotEqual") {
    if (input.size() == 2) {
      ASTNode outNode;
      auto ok = nodesAreNotEqual(input[0], input[1], &outNode);
      if (ok) {
        return {outNode};
      }
    }
    qDebug() << "ERROR: constraint function NotEqual fail.";
  } else if (constraintFunction->getName() == "Equal") {
    if (input.size() == 2) {
      ASTNode outNode;
      auto ok = nodesAreEqual(input[0], input[1], &outNode);
      if (ok) {
        return {outNode};
      }
    }
    qDebug() << "ERROR: constraint function Equal fail.";
  } else if (constraintFunction->getName() == "Greater") {
    if (input.size() == 2) {
      ASTNode outNode;
      auto ok = nodesAreNotEqual(input[0], input[1], &outNode);
      if (ok) {
        return {outNode};
      }
    }
    qDebug() << "ERROR: constraint function Greater fail.";
  } else if (constraintFunction->getName() == "GreaterOrEqual") {
    if (input.size() == 2) {
      ASTNode outNode, outNode2;
      auto ok = nodesIsGreater(input[0], input[1], &outNode);
      auto ok2 = nodesAreEqual(input[0], input[1], &outNode2);
      if (ok && ok2) {
        ASTNode finalOut;
        auto finalOk = nodesAnd(outNode, outNode2, &finalOut);
        if (finalOk) {
          return {finalOut};
        }
      }
    }
    qDebug() << "ERROR: constraint function GreaterOrEqual fail.";
  } else if (constraintFunction->getName() == "Less") {
    if (input.size() == 2) {
      ASTNode outNode;
      auto ok = nodesIsLesser(input[0], input[1], &outNode);
      if (ok) {
        return {outNode};
      }
    }
    qDebug() << "ERROR: constraint function Less fail.";
  } else if (constraintFunction->getName() == "LessOrEqual") {
    if (input.size() == 2) {
      ASTNode outNode, outNode2;
      auto ok = nodesIsLesser(input[0], input[1], &outNode);
      auto ok2 = nodesAreEqual(input[0], input[1], &outNode2);
      if (ok && ok2) {
        ASTNode finalOut;
        auto finalOk = nodesOr(outNode, outNode2, &finalOut);
        if (finalOk) {
          return {finalOut};
        }
      }
    }
    qDebug() << "ERROR: constraint function LessOrEqual fail.";
  } else if (constraintFunction->getName() == "IsNone") {
    if (input.size() == 1) {
      return {std::make_shared<ValueNode>(input[0]->getNodeType() == AST::None,
                                          __FILE__, __LINE__)};
      qDebug() << "ERROR: constraint function LessOrEqual fail.";
    }
  } else if (constraintFunction->getName() == "IsNotNone") {
    if (input.size() == 1) {
      return {std::make_shared<ValueNode>(input[0]->getNodeType() != AST::None,
                                          __FILE__, __LINE__)};
      qDebug() << "ERROR: constraint function LessOrEqual fail.";
    }
  } else if (constraintFunction->getName() == "Error") {
    if (input.size() == 1 && input[0]->getNodeType() == AST::Switch &&
        static_pointer_cast<ValueNode>(input[0])->getSwitchValue()) {
      //      qDebug() << "Constraint: ERROR";
      LangError err;
      err.type = LangError::ConstraintFail;
      err.filename = functionInstance->getFilename();
      err.lineNumber = functionInstance->getLine();
      err.errorTokens.push_back(functionInstance->getName());
      err.errorTokens.push_back(constraintFunction->getFilename());
      err.errorTokens.push_back(std::to_string(constraintFunction->getLine()));

      auto errorMsg = constraintFunction->getPropertyValue("message");
      if (errorMsg && errorMsg->getNodeType() == AST::String) {
        err.errorTokens.push_back(
            std::static_pointer_cast<ValueNode>(errorMsg)->getStringValue());
      } else {
        err.errorTokens.push_back("Unspecified error");
      }
      errors.push_back(err);
    }
  } else if (constraintFunction->getName() == "Or") {
    assert(0 == 1);
    // TODO implement bitwise operators in constraints
  } else if (constraintFunction->getName() == "And") {
    assert(0 == 1);
  } else if (constraintFunction->getName() == "Xor") {
    assert(0 == 1);
  } else if (constraintFunction->getName() == "Not") {
    assert(0 == 1);
  } else {
  }
  return {};
}

std::vector<ASTNode> CodeValidator::resolveConstraintNode(
    ASTNode node, std::vector<ASTNode> previous,
    std::shared_ptr<FunctionNode> function,
    std::shared_ptr<DeclarationNode> declaration, ScopeStack scopeStack,
    ASTNode tree) {
  if (node->getNodeType() == AST::PortProperty) {
    auto pp = static_pointer_cast<PortPropertyNode>(node);
    if (pp->getPortName() == "size") {
      return {std::make_shared<ValueNode>(
          (int64_t)resolveSizePortProperty(pp->getName(), scopeStack,
                                           declaration, function, tree),
          __FILE__, __LINE__)};
    }
    if (pp->getPortName() == "rate") {
      return {std::make_shared<ValueNode>(
          resolveRatePortProperty(pp->getName(), scopeStack, declaration,
                                  function, tree),
          __FILE__, __LINE__)};
    } else {
      return {std::make_shared<AST>()};
    }

  } else if (node->getNodeType() == AST::Function) {
    return processConstraintFunction(
        std::static_pointer_cast<FunctionNode>(node), function, previous,
        m_errors);
  } else if (node->getNodeType() == AST::Block) {

  } else if (node->getNodeType() == AST::Function) {
    return processConstraintFunction(
        std::static_pointer_cast<FunctionNode>(node), function, previous,
        m_errors);
  } else if (node->getNodeType() == AST::List) {
    std::vector<ASTNode> resolvedList;
    size_t i = 0;
    for (auto elem : node->getChildren()) {
      ASTNode previousNode;
      if (previous.size() > i) {
        previousNode = previous[i];
      }
      auto resolved = resolveConstraintNode(elem, {previousNode}, function,
                                            declaration, scopeStack, tree);
      resolvedList.insert(resolvedList.end(), resolved.begin(), resolved.end());
      i++;
    }
    return resolvedList;
  } else {
    return {node};
  }
  return std::vector<ASTNode>();
}

void CodeValidator::sortErrors() {
  std::sort(m_errors.begin(), m_errors.end(),
            [](const LangError &err1, const LangError &err2) {
              return err1.lineNumber < err2.lineNumber;
            });
}

void CodeValidator::validateStreamInputSize(StreamNode *stream,
                                            ScopeStack scope,
                                            QList<LangError> &errors) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();

  int leftOutSize = getNodeNumOutputs(left, scope, m_tree, errors);
  int rightInSize = getNodeNumInputs(right, scope, m_tree, errors);

  auto leftDecl = ASTQuery::findDeclaration(
      CodeValidator::streamMemberName(left), scope, m_tree);
  std::shared_ptr<DeclarationNode> rightDecl;
  if (right->getNodeType() == AST::Stream) {
    auto nextStreamMember = static_pointer_cast<StreamNode>(right)->getLeft();
    rightDecl = ASTQuery::findDeclaration(
        CodeValidator::streamMemberName(nextStreamMember), scope, m_tree);
  } else {
    rightDecl = ASTQuery::findDeclaration(
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
    auto indexList = bundle->index();
    vector<ASTNode> indexExps = indexList->getChildren();
    for (ASTNode exp : indexExps) {
      if (exp->getNodeType() == AST::Range) {
        RangeNode *range = static_cast<RangeNode *>(exp.get());
        ASTNode start = range->startIndex();
        int startIndex, endIndex;
        startIndex =
            CodeValidator::evaluateConstInteger(start, scope, tree, errors);
        ASTNode end = range->startIndex();
        endIndex =
            CodeValidator::evaluateConstInteger(end, scope, tree, errors);
        if (end > start) {
          size += endIndex + startIndex + 1;
        }

      } else if (exp->getNodeType() == AST::PortProperty) {
        return -2; // FIXME calculate actual size from external connection
      } else {
        QList<LangError> internalErrors;
        size += CodeValidator::evaluateConstInteger(exp, scope, tree,
                                                    internalErrors);
        if (internalErrors.size() > 0) {
          LangError error;
          error.type = LangError::InvalidIndexType;
          error.lineNumber = exp->getLine();
          error.errorTokens.push_back(bundle->getName());
          errors << error;
        }
      }
    }
  }
  return size;
}

int CodeValidator::getBlockDataSize(
    std::shared_ptr<DeclarationNode> declaration, ScopeStack scope,
    QList<LangError> &errors) {
  std::vector<std::shared_ptr<PropertyNode>> ports =
      declaration->getProperties();
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
  auto ports = func->getProperties();
  if (ports.size() == 0) {
    return 1;
  }
  int size = 1;
  for (std::shared_ptr<PropertyNode> port : ports) {
    ASTNode value = port->getValue();
    // FIXME need to decide the size by also looking at the port block size
    int newSize = CodeValidator::getNodeNumOutputs(value, scope, tree, errors);
    auto decl = ASTQuery::findDeclaration(func->getName(), scope, tree,
                                          func->getNamespaceList());
    if (decl && decl->getPropertyValue("ports")) {
      std::shared_ptr<DeclarationNode> declaredPort;
      for (auto portDecl : decl->getPropertyValue("ports")->getChildren()) {
        if (portDecl->getNodeType() == AST::Declaration) {
          auto nameNode =
              static_pointer_cast<DeclarationNode>(portDecl)->getPropertyValue(
                  "name");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            if (port->getName() ==
                static_pointer_cast<ValueNode>(nameNode)->getStringValue()) {
              declaredPort = static_pointer_cast<DeclarationNode>(portDecl);
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
                static_pointer_cast<BlockNode>(portBlockNode)->getName();
          } else if (portBlockNode->getNodeType() == AST::Bundle) {
            portBlockName =
                static_pointer_cast<BundleNode>(portBlockNode)->getName();
          }
          auto portBlockDecl = ASTQuery::findDeclaration(
              portBlockName, {{nullptr, blocks->getChildren()}}, nullptr);
          if (declaredPort->getObjectType() == "mainInputPort" ||
              declaredPort->getObjectType() == "propertyInputPort") {
            QList<LangError> errors;
            size = CodeValidator::getNodeNumInputs(
                portBlockNode, {{nullptr, blocks->getChildren()}}, tree,
                errors);
          } else if (declaredPort->getObjectType() == "mainOutputPort" ||
                     declaredPort->getObjectType() == "propertyOutputPort") {
            QList<LangError> errors;
            size = CodeValidator::getNodeNumOutputs(
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

int CodeValidator::getFunctionNumInstances(std::shared_ptr<FunctionNode> func,
                                           ScopeStack scope, ASTNode tree) {
  auto portProperties = func->getProperties();
  int numInstances = 1;
  for (auto propertyNode : portProperties) {
    auto portBlock = propertyNode->getValue();
    QList<LangError> errors;
    auto portName = propertyNode->getName();
    auto funcDecl = ASTQuery::findDeclaration(
        CodeValidator::streamMemberName(func), scope, tree);
    if (funcDecl) {
      if (funcDecl->getPropertyValue("blocks")) {
        scope.push_back(
            {func, funcDecl->getPropertyValue("blocks")->getChildren()});
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
                //                numInstances = 1;
                if (internalBlockSize == -2) {
                  // FIXME this assumes that the block is given the size of its
                  // own port, but it is possible it could be give the size of
                  // another port, causing an error or a different number of
                  // instances
                  return 1;
                }
                int portNumInstances = propertyBlockSize / internalBlockSize;
                Q_ASSERT(propertyBlockSize / internalBlockSize ==
                         float(propertyBlockSize) / internalBlockSize);
                if (numInstances == 0) {
                  numInstances = portNumInstances;
                } else if (numInstances == 1 && portNumInstances > 1) {
                  numInstances = portNumInstances;
                } else if (numInstances != portNumInstances &&
                           portNumInstances != 1) {
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
  std::string type;

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
      if (type == "_IntLiteral" || type == "_IntType") {
        size += 1;
      } else {
        qDebug() << "Error, expression does not resolve to int";
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
          ASTQuery::findDeclaration(name->getName(), scope, tree);
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
        ASTQuery::findDeclaration(name->getName(), scope, tree);
    if (block) {
      return getTypeNumOutputs(block, scope, tree, errors);
    } else {
      return -1;
    }
  } else if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        static_pointer_cast<FunctionNode>(node);
    std::shared_ptr<DeclarationNode> platformFunc =
        ASTQuery::findDeclaration(func->getName(), scope, tree);
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
        ASTQuery::findDeclaration(func->getName(), scope, tree);
    int dataSize =
        CodeValidator::getFunctionDataSize(func, scope, tree, errors);
    if (platformFunc) {
      if (platformFunc->getObjectType() == "reaction") {
        return 1; // Reactions always have one input as main port for trigger
      } else {
        ASTNode subScope = CodeValidator::getBlockSubScope(platformFunc);
        if (subScope) {
          scope.push_back({func, subScope->getChildren()});
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
        ASTQuery::findDeclaration(name->getName(), scope, tree);
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
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    auto decl = static_pointer_cast<DeclarationNode>(node);
    return decl->getName();
  } else {
    //        qDebug() << "streamMemberName() error. Invalid stream member
    //        type.";
  }
  return std::string();
}

std::string CodeValidator::resolveBundleType(BundleNode *bundle,
                                             ScopeStack scopeStack,
                                             ASTNode tree) {
  std::shared_ptr<DeclarationNode> declaration =
      ASTQuery::findDeclaration(bundle->getName(), scopeStack, tree);
  if (declaration) {
    if (declaration->getObjectType() == "constant") {
      std::shared_ptr<PropertyNode> property =
          CodeValidator::findPropertyByName(declaration->getProperties(),
                                            "value");
      if (property) {
        return resolveNodeOutType(property->getValue(), scopeStack, tree);
      }
    } else {
      //            return
      QString::fromStdString(declaration->getObjectType());
    }
  }
  return "";
}

std::string CodeValidator::resolveBlockType(BlockNode *name,
                                            ScopeStack scopeStack,
                                            ASTNode tree) {
  std::shared_ptr<DeclarationNode> declaration =
      ASTQuery::findDeclaration(name->getName(), scopeStack, tree);
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
      auto defaultType =
          resolveNodeOutType(property->getValue(), scopeStack, tree);
      return defaultType;
    } else {
      return "";
    }
  }
  return "";
}

std::string CodeValidator::resolveNodeOutType(ASTNode node,
                                              ScopeStack scopeStack,
                                              ASTNode tree) {
  if (node->getNodeType() == AST::Int) {
    return "_IntLiteral";
  } else if (node->getNodeType() == AST::Real) {
    return "_RealLiteral";
  } else if (node->getNodeType() == AST::Switch) {
    return "_SwitchLiteral";
  } else if (node->getNodeType() == AST::String) {
    return "_StringhLiteral";
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
    return resolveBlockType(static_cast<BlockNode *>(node.get()), scopeStack,
                            tree);
  } else if (node->getNodeType() == AST::Range) {
    return resolveRangeType(static_cast<RangeNode *>(node.get()), scopeStack,
                            tree);
  } else if (node->getNodeType() == AST::PortProperty) {
    return "_PortProperty";
    //        return resolvePortPropertyType(static_cast<PortPropertyNode
    //        *>(node.get()), scope, tree);
  }
  return "";
}

std::string CodeValidator::resolveListType(ListNode *listnode,
                                           ScopeStack scopeStack,
                                           ASTNode tree) {
  std::vector<ASTNode> members = listnode->getChildren();
  if (members.size() == 0) {
    return "";
  }
  ASTNode firstMember = members.at(0);
  auto type = resolveNodeOutType(firstMember, scopeStack, tree);

  for (ASTNode member : members) {
    auto nextPortType = resolveNodeOutType(member, scopeStack, tree);
    if (type != nextPortType) {
      if (type == "_IntLiteral" &&
          nextPortType == "_RealLiteral") { // List becomes Real if Real found
        type = "_RealLiteral";
      } else if (type == "_RealLiteral" &&
                 nextPortType == "_IntLiteral") { // Int in Real list
                                                  // Nothing here for now
      } else {                                    // Invalid combination
        return "";
      }
    }
  }
  return type;
}

std::string CodeValidator::resolveExpressionType(ExpressionNode *exprnode,
                                                 ScopeStack scopeStack,
                                                 ASTNode tree) {
  if (!exprnode->isUnary()) {
    ASTNode left = exprnode->getLeft();
    ASTNode right = exprnode->getRight();
    auto leftType = resolveNodeOutType(left, scopeStack, tree);
    auto rightType = resolveNodeOutType(right, scopeStack, tree);
    if (leftType == rightType) {
      return leftType;
    }
    // TODO implement toleraces between ints and reals

  } else {
    // TODO implement for unary
  }
  return "";
}

std::string CodeValidator::resolveRangeType(RangeNode *rangenode,
                                            ScopeStack scopeStack,
                                            ASTNode tree) {
  auto leftType = resolveNodeOutType(rangenode->startIndex(), scopeStack, tree);
  auto rightType = resolveNodeOutType(rangenode->endIndex(), scopeStack, tree);
  if (leftType == rightType) {
    return leftType;
  }
  return "";
}

std::string
CodeValidator::resolvePortPropertyType(PortPropertyNode *portproperty,
                                       ScopeStack scopeStack, ASTNode tree) {
  // FIXME implement correctly. Should be read from framework?
  if (portproperty->getPortName() == "size") {
    return "_IntLiteral";
  } else if (portproperty->getPortName() == "rate") {
    return "_RealLiteral";
  }
  { return "_RealLiteral"; }
}

shared_ptr<DeclarationNode>
CodeValidator::resolveConnectionBlock(ASTNode node, ScopeStack scopeStack,
                                      ASTNode tree, bool downStream) {
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
          auto portBlockDecl = ASTQuery::findDeclaration(
              CodeValidator::streamMemberName(portBlock),
              {{node, static_pointer_cast<DeclarationNode>(funcDecl)
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
              // Now match the port name from the domain to the port
              // declaration
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
                      if (outerBlock->getNodeType() == AST::Block ||
                          outerBlock->getNodeType() == AST::Bundle) {
                        auto decl = ASTQuery::findDeclaration(
                            CodeValidator::streamMemberName(outerBlock),
                            scopeStack, tree);
                        if (decl) {
                          return decl;
                        } else {
                          return shared_ptr<DeclarationNode>();
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
              qDebug() << "Unexpected port block domain";
            }
          }
        }
      }
    }
  }
  return shared_ptr<DeclarationNode>();
}

ASTNode CodeValidator::getMatchedOuterInstance(
    std::shared_ptr<FunctionNode> functionNode,
    std::shared_ptr<DeclarationNode> blockDecl,
    std::shared_ptr<DeclarationNode> funcDecl, ScopeStack scopeStack,
    ASTNode tree) {
  // Instance is declaration for block, but stream node for function
  ASTNode matchedInst = nullptr;
  auto internalBlocks = funcDecl->getPropertyValue("blocks")->getChildren();
  for (auto port : funcDecl->getPropertyValue("ports")->getChildren()) {
    if (port->getNodeType() == AST::Declaration) {
      auto portDecl = static_pointer_cast<DeclarationNode>(port);

      auto portBlock =
          static_pointer_cast<BlockNode>(portDecl->getPropertyValue("block"));

      // Search only within internal scope for port block
      auto portBlockDecl =
          ASTQuery::findDeclaration(CodeValidator::streamMemberName(portBlock),
                                    {{functionNode, internalBlocks}}, nullptr);

      if (portBlockDecl && portBlockDecl->getName() == blockDecl->getName()) {
        // If it's a main port, we can get the external block from the
        // compiler properties of the FunctionNode
        if (portDecl->getObjectType() == "mainOutputPort") {
          ASTNode extOutputBlockInst;
          auto outputBlock = functionNode->getCompilerProperty("outputBlock");
          if (outputBlock) {
            extOutputBlockInst =
                CodeValidator::getInstance(outputBlock, scopeStack, tree);
          }
          matchedInst = extOutputBlockInst;
          break;
        } else if (portDecl->getObjectType() == "mainInputPort") {
          ASTNode extInputBlockInst;
          auto inputBlock = functionNode->getCompilerProperty("inputBlock");
          if (inputBlock) {
            extInputBlockInst =
                CodeValidator::getInstance(inputBlock, scopeStack, tree);
          }
          matchedInst = extInputBlockInst;
          break;
        } else {
          // If it's a secondary port, we get the block from the FunctionNode
          // properties
          auto propertyName = static_pointer_cast<ValueNode>(
              portDecl->getPropertyValue("name"));
          if (propertyName->getNodeType() == AST::String) {
            auto outerBlock =
                functionNode->getPropertyValue(propertyName->toString());
            if (outerBlock) {
              if (outerBlock->getNodeType() == AST::Block ||
                  outerBlock->getNodeType() == AST::Bundle) {
                matchedInst = ASTQuery::findDeclaration(
                    CodeValidator::streamMemberName(outerBlock), scopeStack,
                    tree);
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

int CodeValidator::resolveSizePortProperty(
    string targetPortName, ScopeStack scopeStack,
    std::shared_ptr<DeclarationNode> decl, std::shared_ptr<FunctionNode> func,
    ASTNode tree) {
  auto portsNode = decl->getPropertyValue("ports");
  auto blocksNode = decl->getPropertyValue("blocks");
  int portSize = 0;
  if (portsNode && blocksNode) {
    std::vector<ASTNode> ports = portsNode->getChildren();
    std::vector<ASTNode> blocks = blocksNode->getChildren();
    for (auto port : ports) {
      if (port->getNodeType() == AST::Declaration) {
        auto portDecl = static_pointer_cast<DeclarationNode>(port);
        auto portName = portDecl->getName();
        //                std::string portDomainName = portName + "_domain";
        //        scopeStack.push_back({func, blocks});
        if (portName == targetPortName) {
          auto portBlock = portDecl->getPropertyValue("block");
          auto portNameNode = portDecl->getPropertyValue("name");
          std::string portName;

          if (portNameNode && portNameNode->getNodeType() == AST::String) {
            portName =
                static_pointer_cast<ValueNode>(portNameNode)->getStringValue();
          }
          std::string portBlockName;
          if (portBlock && (portBlock->getNodeType() == AST::Block ||
                            portBlock->getNodeType() == AST::Bundle)) {
            if (portDecl->getObjectType() == "mainInputPort") {
              QList<LangError> errors;
              portSize = CodeValidator::getNodeNumInputs(portBlock, scopeStack,
                                                         tree, errors);
              if (portSize == -2) {
                std::pair<ASTNode, std::vector<ASTNode>> innerScope =
                    scopeStack.back();
                scopeStack.pop_back();
                // We need to look one scope up.
                portSize = CodeValidator::getNodeNumOutputs(
                    func->getCompilerProperty("inputBlock"), scopeStack, tree,
                    errors);
                scopeStack.push_back(innerScope);
              }

            } else if (portDecl->getObjectType() == "mainOutputPort") {
              QList<LangError> errors;
              portSize = CodeValidator::getNodeNumOutputs(portBlock, scopeStack,
                                                          tree, errors);
              if (portSize == -2) {
                std::pair<ASTNode, std::vector<ASTNode>> innerScope =
                    scopeStack.back();
                scopeStack.pop_back();
                // We need to look one scope up.
                portSize = CodeValidator::getNodeNumOutputs(
                    func->getPropertyValue(portName), scopeStack, tree, errors);
                scopeStack.push_back(innerScope);
              }
            } else if (portDecl->getObjectType() == "propertyInputPort") {
              QList<LangError> errors;
              portSize = CodeValidator::getNodeNumInputs(portBlock, scopeStack,
                                                         tree, errors);
              if (portSize == -2) {
                std::pair<ASTNode, std::vector<ASTNode>> innerScope =
                    scopeStack.back();
                scopeStack.pop_back();
                // We need to look one scope up.
                portSize = CodeValidator::getNodeNumOutputs(
                    func->getPropertyValue(portName), scopeStack, tree, errors);
                scopeStack.push_back(innerScope);
              }

            } else if (portDecl->getObjectType() == "propertyOutputPort") {
              QList<LangError> errors;
              portSize = CodeValidator::getNodeNumOutputs(portBlock, scopeStack,
                                                          tree, errors);

              if (portSize == -2) {
                std::pair<ASTNode, std::vector<ASTNode>> innerScope =
                    scopeStack.back();
                scopeStack.pop_back();
                // We need to look one scope up.
                portSize = CodeValidator::getNodeNumOutputs(
                    func->getPropertyValue(portName), scopeStack, tree, errors);
                scopeStack.push_back(innerScope);
              }
            }
          } else {
            qDebug() << "Unexpected port block entry";
          }
        }
      }
    }
  }
  return portSize;
}

double CodeValidator::resolveRatePortProperty(
    std::string targetPortName, ScopeStack scopeStack,
    std::shared_ptr<DeclarationNode> decl, std::shared_ptr<FunctionNode> func,
    ASTNode tree) {
  auto portsNode = decl->getPropertyValue("ports");
  auto blocksNode = decl->getPropertyValue("blocks");

  std::vector<ASTNode> ports = portsNode->getChildren();
  std::vector<ASTNode> blocks = blocksNode->getChildren();
  for (auto port : ports) {
    if (port->getNodeType() == AST::Declaration) {
      auto portDecl = static_pointer_cast<DeclarationNode>(port);
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
          portName =
              static_pointer_cast<ValueNode>(portNameNode)->getStringValue();
        }
        std::string portBlockName;
        if (portBlock && (portBlock->getNodeType() == AST::Block ||
                          portBlock->getNodeType() == AST::Bundle)) {
          if (portDecl->getObjectType() == "mainInputPort") {
            double rate =
                CodeValidator::resolveRate(portBlock, scopeStack, tree, false);
            if (rate == -1) {
              auto resolvedBlock = CodeValidator::getInstance(
                  func->getCompilerProperty("inputBlock"), scopeStack, tree);
              auto scopeStackOuter = scopeStack;
              for (auto subScope : scopeStack) {
                rate = CodeValidator::resolveRate(resolvedBlock, scopeStack,
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
                    static_pointer_cast<FunctionNode>(subScope.first);
                // TODO check framework too
                std::string framework;
                auto funcDecl = ASTQuery::findDeclaration(
                    functionNode->getName(), scopeStackOuter, tree,
                    functionNode->getNamespaceList(), framework);
                if (resolvedBlock->getNodeType() == AST::Declaration ||
                    resolvedBlock->getNodeType() == AST::BundleDeclaration) {
                  resolvedBlock = getMatchedOuterInstance(
                      functionNode,
                      static_pointer_cast<DeclarationNode>(resolvedBlock),
                      funcDecl, scopeStackOuter, tree);
                  scopeStackOuter.pop_back();
                }
              }
            }
            return rate;

          } else if (portDecl->getObjectType() == "mainOutputPort") {
            double rate =
                CodeValidator::resolveRate(portBlock, scopeStack, tree, false);
            if (rate == -1) {
              auto resolvedBlock = CodeValidator::getInstance(
                  func->getCompilerProperty("outputBlock"), scopeStack, tree);
              auto scopeStackOuter = scopeStack;
              for (auto subScope : scopeStack) {
                rate = CodeValidator::resolveRate(resolvedBlock, scopeStack,
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
                    static_pointer_cast<FunctionNode>(subScope.first);
                // TODO check framework too
                std::string framework;
                auto funcDecl = ASTQuery::findDeclaration(
                    functionNode->getName(), scopeStackOuter, tree,
                    functionNode->getNamespaceList(), framework);
                if (resolvedBlock->getNodeType() == AST::Declaration ||
                    resolvedBlock->getNodeType() == AST::BundleDeclaration) {
                  resolvedBlock = getMatchedOuterInstance(
                      functionNode,
                      static_pointer_cast<DeclarationNode>(resolvedBlock),
                      funcDecl, scopeStackOuter, tree);
                  scopeStackOuter.pop_back();
                }
              }
            }
            return rate;

          } else if (portDecl->getObjectType() == "propertyInputPort" ||
                     portDecl->getObjectType() == "propertyOutputPort") {
            double rate =
                CodeValidator::resolveRate(portBlock, scopeStack, tree, false);
            if (rate == -1) {
              auto resolvedBlock =
                  CodeValidator::getInstance(portBlock, scopeStack, tree);
              auto scopeStackOuter = scopeStack;
              for (auto subScope : scopeStack) {
                rate = CodeValidator::resolveRate(resolvedBlock, scopeStack,
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
                    static_pointer_cast<FunctionNode>(subScope.first);
                // TODO check framework too
                std::string framework;
                auto funcDecl = ASTQuery::findDeclaration(
                    functionNode->getName(), scopeStackOuter, tree,
                    functionNode->getNamespaceList(), framework);
                if (resolvedBlock->getNodeType() == AST::Declaration ||
                    resolvedBlock->getNodeType() == AST::BundleDeclaration) {
                  resolvedBlock = getMatchedOuterInstance(
                      functionNode,
                      static_pointer_cast<DeclarationNode>(resolvedBlock),
                      funcDecl, scopeStackOuter, tree);
                  scopeStackOuter.pop_back();
                }
              }
            }
            return rate;
          }
        } else {
          qDebug() << "Unexpected port block entry";
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
  //                  auto rate = CodeValidator::resolveRate(
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
  //                          rate = CodeValidator::resolveRate(
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
  //                          rate = CodeValidator::resolveRate(
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
  //                auto rate = CodeValidator::resolveRate(
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
  //                          rate = CodeValidator::resolveRate(
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
  //                          rate = CodeValidator::resolveRate(
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

ASTNode CodeValidator::resolveDomain(ASTNode node, ScopeStack scopeStack,
                                     ASTNode tree, bool downStream) {
  auto blockDecl = resolveConnectionBlock(node, scopeStack, tree, downStream);
  if (blockDecl) {
    return CodeValidator::getNodeDomain(blockDecl, scopeStack, tree);
  } else {
    return CodeValidator::getNodeDomain(node, scopeStack, tree);
  }
}

double CodeValidator::resolveRate(ASTNode node, ScopeStack scopeStack,
                                  ASTNode tree, bool downStream) {
  auto blockDecl = resolveConnectionBlock(node, scopeStack, tree, downStream);
  if (blockDecl) {
    return CodeValidator::getNodeRate(blockDecl, scopeStack, tree);
  } else if (node) {
    return CodeValidator::getNodeRate(node, scopeStack, tree);
  } else {
    return -1;
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
      std::shared_ptr<DeclarationNode> declaration =
          ASTQuery::findDeclaration(bundle->getName(), scope, tree);
      if (declaration && declaration->getNodeType() == AST::BundleDeclaration) {
        ASTNode member = getMemberfromBlockBundleConst(declaration, index, tree,
                                                       scope, errors);
        QList<LangError> internalErrors;
        auto integer =
            evaluateConstInteger(member, scope, tree, internalErrors);
        if (internalErrors.size() == 0) {
          return integer;
        }
        LangError error;
        error.type = LangError::InvalidIndexType;
        error.lineNumber = bundle->index()->getLine();
        error.errorTokens.push_back(bundle->getName());
        errors << error;
        return 0;
      }
    }
    LangError error;
    error.type = LangError::InvalidType;
    error.lineNumber = bundle->getLine();
    error.errorTokens.push_back(bundle->getName());
    errors << error;
  } else if (node->getNodeType() == AST::Block) {
    std::shared_ptr<BlockNode> nameNode =
        std::static_pointer_cast<BlockNode>(node);
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclaration(nameNode->getName(), scope, tree);
    if (declaration && declaration->getObjectType() == "constant") {
      return evaluateConstInteger(declaration->getPropertyValue("value"), scope,
                                  tree, errors);
    }
  } else if (node->getNodeType() == AST::Expression) {
    // FIXME: check expression out
    return 0;
  } else {
    LangError error;
    error.type = LangError::InvalidType;
    error.lineNumber = node->getLine();
    error.errorTokens.push_back("");
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
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclaration(bundle->getName(), scope, tree);
    int index = evaluateConstInteger(bundle->index(), scope, tree, errors);
    if (declaration && declaration->getNodeType() == AST::BundleDeclaration) {
      ASTNode member = getMemberfromBlockBundleConst(declaration, index, tree,
                                                     scope, errors);
      return evaluateConstReal(member, scope, tree, errors);
    }
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *blockNode = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        blockNode->getName(), scope, tree, blockNode->getNamespaceList());
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
    error.errorTokens.push_back("");
    errors << error;
  }
  return result;
}

std::string CodeValidator::evaluateConstString(ASTNode node, ScopeStack scope,
                                               ASTNode tree,
                                               std::string currentFramework,
                                               QList<LangError> &errors) {
  std::string result;
  if (node->getNodeType() == AST::String) {
    return static_pointer_cast<ValueNode>(node)->getStringValue();
  } else if (node->getNodeType() == AST::Bundle) {
    auto bundle = static_pointer_cast<BundleNode>(node);
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclaration(bundle->getName(), scope, tree);
    int index = evaluateConstInteger(bundle->index(), scope, tree, errors);
    if (declaration && declaration->getNodeType() == AST::BundleDeclaration) {
      ASTNode member = getMemberfromBlockBundleConst(declaration, index, tree,
                                                     scope, errors);
      return evaluateConstString(member, scope, tree, currentFramework, errors);
    }
  } else if (node->getNodeType() == AST::Block) {
    auto blockNode = static_pointer_cast<BlockNode>(node);
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        blockNode->getName(), scope, tree, blockNode->getNamespaceList(),
        currentFramework);
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
    std::shared_ptr<DeclarationNode> block = ASTQuery::findDeclaration(
        propertyNode->getName(), scope, tree, {}, currentFramework);
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
    error.errorTokens.push_back("");
    errors << error;
  }
  return result;
}

ASTNode CodeValidator::getMemberfromBlockBundleConst(
    std::shared_ptr<DeclarationNode> blockDecl, int index, ASTNode tree,
    ScopeStack scopeStack, QList<LangError> &errors) {
  ASTNode out = nullptr;
  if (blockDecl->getObjectType() == "constant") {
    auto ports = blockDecl->getProperties();
    for (std::shared_ptr<PropertyNode> port : ports) {
      if (port->getName() == "value") {
        ASTNode value = port->getValue();
        if (value->getNodeType() == AST::List) {
          return getMemberFromList(static_cast<ListNode *>(value.get()), index,
                                   errors);
        } else if (value->getNodeType() == AST::Bundle) {
          auto decl = ASTQuery::findDeclaration(
              CodeValidator::streamMemberName(blockDecl), scopeStack, tree,
              blockDecl->getNamespaceList());
          auto indexList = static_pointer_cast<BundleNode>(value);
          if (indexList->getChildren().size() == 1) {
            auto previousErrors = errors.size();
            auto index = evaluateConstInteger(indexList->getChildren()[0],
                                              scopeStack, tree, errors);
            if (previousErrors == errors.size()) {
              return getMemberfromBlockBundleConst(decl, index, tree,
                                                   scopeStack, errors);
            }
          }
        }
      }
    }
  }
  return out;
}

ASTNode CodeValidator::getValueFromConstBlock(DeclarationNode *block) {
  ASTNode out = nullptr;
  if (block->getObjectType() == "constant") {
    auto ports = block->getProperties();
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
  auto portList = getPortsForTypeBlock(typeDeclaration, scope, tree);
  for (ASTNode node : portList) {
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

std::shared_ptr<DeclarationNode>
CodeValidator::findTypeDeclaration(std::shared_ptr<DeclarationNode> block,
                                   ScopeStack scope, ASTNode tree,
                                   std::string currentFramework) {
  string typeName = block->getObjectType();
  return ASTQuery::findTypeDeclarationByName(
      typeName, scope, tree, block->getNamespaceList(), currentFramework);
}

std::shared_ptr<DeclarationNode>
CodeValidator::findDomainDeclaration(string domainName, string framework,
                                     ASTNode tree) {
  std::string domainFramework;
  auto separatorIndex = domainName.find("::");
  if (separatorIndex != std::string::npos) {
    domainFramework = domainName.substr(0, separatorIndex);
    domainName = domainName.substr(separatorIndex + 2);
  }

  if (domainFramework != framework && domainFramework != "") {
    qDebug() << "Unexpected domain mismatch";
  }
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "_domainDefinition") {
        if (domainName == decl->getName()) {
          auto domainDeclFramework = decl->getCompilerProperty("framework");
          if (domainDeclFramework &&
              domainDeclFramework->getNodeType() == AST::String) {
            if (framework == static_pointer_cast<ValueNode>(domainDeclFramework)
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
CodeValidator::findDomainDeclaration(string domainId, ASTNode tree) {
  std::string domainFramework;

  auto separatorIndex = domainId.find("::");
  if (separatorIndex != std::string::npos) {
    domainFramework = domainId.substr(0, separatorIndex);
    if (domainFramework !=
        CodeValidator::getFrameworkForDomain(domainId, tree)) {
      qDebug() << "ERROR framework mismatch";
    }
    domainId = domainId.substr(separatorIndex + 2);
  }
  auto trimmedDomain = domainId.substr(0, domainId.find(':'));
  return findDomainDeclaration(trimmedDomain, domainFramework, tree);
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

std::string
CodeValidator::getDataTypeForDeclaration(std::shared_ptr<DeclarationNode> decl,
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
    qDebug() << __FILE__ << ":" << __LINE__ << " ERROR unsupported object type";
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

std::vector<ASTNode>
CodeValidator::getPortsForType(string typeName, ScopeStack scope, ASTNode tree,
                               std::vector<string> namespaces,
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
        Q_ASSERT(name->getNodeType() == AST::String);
        Q_ASSERT(name->getStringValue() == typeName);
        auto newPortList = getPortsForTypeBlock(typeBlock, scope, tree);
        portList.insert(portList.end(), newPortList.begin(), newPortList.end());
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

std::vector<ASTNode>
CodeValidator::getInheritedPorts(std::shared_ptr<DeclarationNode> block,
                                 ScopeStack scope, ASTNode tree) {
  std::vector<ASTNode> inheritedProperties;
  auto inheritedTypes = ASTQuery::getInheritedTypes(block, scope, tree);
  QStringList inheritedName;
  for (auto typeDeclaration : inheritedTypes) {
    auto inheritedFromType = getPortsForTypeBlock(typeDeclaration, scope, tree);
    for (ASTNode property : inheritedFromType) {
      if (std::find(inheritedProperties.begin(), inheritedProperties.end(),
                    property) == inheritedProperties.end()) {
        inheritedProperties.push_back(property);
      }
      if (property->getNodeType() == AST::Declaration) {
        inheritedName << QString::fromStdString(
            std::static_pointer_cast<DeclarationNode>(property)->getName());
      }
    }
  }
  return inheritedProperties;
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
      qDebug() << "ERROR! ports property must be a list or None!";
    }
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

std::vector<ASTNode>
CodeValidator::getPortsForTypeBlock(std::shared_ptr<DeclarationNode> block,
                                    ScopeStack scope, ASTNode tree) {
  ASTNode portsValue = block->getPropertyValue("properties");
  std::vector<ASTNode> outList;
  if (portsValue && portsValue->getNodeType() != AST::None) {
    Q_ASSERT(portsValue->getNodeType() == AST::List);
    ListNode *portList = static_cast<ListNode *>(portsValue.get());
    for (ASTNode port : portList->getChildren()) {
      outList.push_back(port);
    }
  }

  auto inherited = getInheritedPorts(block, scope, tree);
  outList.insert(outList.end(), inherited.begin(), inherited.end());
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
    std::shared_ptr<DeclarationNode> block =
        ASTQuery::findDeclaration(blockNode->getName(), scopeStack, tree);
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

ASTNode CodeValidator::getNodeDomain(ASTNode node, ScopeStack scopeStack,
                                     ASTNode tree) {
  ASTNode domainNode = nullptr;
  if (!node) {
    return nullptr;
  }

  if (node->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        name->getName(), scopeStack, tree, name->getNamespaceList());

    if (declaration) {
      auto typeDeclaration =
          CodeValidator::findTypeDeclaration(declaration, scopeStack, tree);
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

    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        name->getName(), scopeStack, tree, name->getNamespaceList());
    if (declaration && declaration->getDomain()) {
      domainNode = declaration->getDomain()->deepCopy();
      domainNode->setNamespaceList(node->getNamespaceList());
    }
  } else if (node->getNodeType() == AST::List) {
    std::vector<std::string> domainList;
    std::string tempDomainName;
    for (ASTNode member : node->getChildren()) {
      if (member->getNodeType() == AST::Block) {
        BlockNode *name = static_cast<BlockNode *>(member.get());
        std::shared_ptr<DeclarationNode> declaration =
            ASTQuery::findDeclaration(name->getName(), scopeStack, tree,
                                      name->getNamespaceList());
        if (declaration) {
          tempDomainName =
              CodeValidator::getNodeDomainName(declaration, scopeStack, tree);
        }
      } else if (member->getNodeType() == AST::Bundle) {
        BundleNode *name = static_cast<BundleNode *>(member.get());
        std::shared_ptr<DeclarationNode> declaration =
            ASTQuery::findDeclaration(name->getName(), scopeStack, tree);
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
    // There is no real resolution for domain from a list. The caller
    // should query each member for the domain. For now, assuming the first
    // element of the list determines domain.
    return CodeValidator::getNodeDomain(node->getChildren().at(0), scopeStack,
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
              static_pointer_cast<ValueNode>(frameworkNode)->getStringValue());
        }
        //        domainNode->setNamespaceList(node->getNamespaceList());
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    domainNode = node->getCompilerProperty(
        "domain"); // static_cast<FunctionNode *>(node.get())->getDomain();
    if (!domainNode) {
      auto funcDecl = ASTQuery::findDeclaration(
          static_pointer_cast<FunctionNode>(node)->getName(), scopeStack, tree,
          node->getNamespaceList());
      if (funcDecl && funcDecl->getObjectType() == "platformModule") {
        domainNode = funcDecl->getPropertyValue("domain");

        auto domainId =
            CodeValidator::getDomainIdentifier(domainNode, {}, tree);
        std::string domainFramework;
        auto separatorIndex = domainId.find("::");
        if (separatorIndex != std::string::npos) {
          domainFramework = domainId.substr(0, separatorIndex);
          domainId = domainId.substr(separatorIndex + 2);
        }

        auto domainDecl = CodeValidator::findDomainDeclaration(
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
          CodeValidator::getNodeDomain(expr->getValue(), scopeStack, tree);
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
    domainNode = static_pointer_cast<ValueNode>(node)->getDomain();
  } else if (node->getNodeType() == AST::PortProperty) {

    domainNode = node->getCompilerProperty("domain");
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
      std::string framework;
      if (domainBlock->getScopeLevels() > 0) {
        framework = domainBlock->getNamespaceList()[0];
      }
      std::shared_ptr<DeclarationNode> domainDeclaration =
          CodeValidator::findDomainDeclaration(domainBlock->getName(),
                                               framework, tree);
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
          frameworkName =
              static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
          if (frameworkName.size() > 0) {
            name = frameworkName + "::" + name;
          }
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
      auto domainDeclaration =
          ASTQuery::findDeclaration(domainBlock->getName(), scopeStack, tree);
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
      qDebug() << "DEPRECATED: String domain no longer allowed.";
      //      assert(0 == 1); // Should not be allowed
    } else if (domain->getNodeType() == AST::PortProperty) {
      // Should anything be added to the id? Scope?
      auto portProperty = static_pointer_cast<PortPropertyNode>(domain);
      name = portProperty->getName() + "_" + portProperty->getPortName();
    }
  }
  return name;
}

void CodeValidator::setDomainForNode(ASTNode node, ASTNode domain,
                                     ScopeStack scopeStack, ASTNode tree,
                                     bool force) {
  auto existingDomain = getNodeDomain(node, scopeStack, tree);
  if (existingDomain && existingDomain->getNodeType() != AST::None && !force) {
    return;
  }
  if (node->getNodeType() == AST::Declaration ||
      node->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> block =
        static_pointer_cast<DeclarationNode>(node);
    if (block) {
      block->replacePropertyValue("domain", domain);
    }
  } else if (node->getNodeType() == AST::Block ||
             node->getNodeType() == AST::Bundle) {
    std::shared_ptr<DeclarationNode> declaration = ASTQuery::findDeclaration(
        CodeValidator::streamMemberName(node), scopeStack, tree);
    if (declaration) {
      auto typeDeclaration =
          CodeValidator::findTypeDeclaration(declaration, scopeStack, tree);
      if (typeDeclaration &&
          typeDeclaration->getObjectType() == "platformModule") {
        node->setCompilerProperty("domain", domain);
      } else {
        declaration->replacePropertyValue("domain", domain);
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        static_pointer_cast<FunctionNode>(node);
    if (func) {
      func->setCompilerProperty("domain", domain);
    }
  } else if (node->getNodeType() == AST::Real ||
             node->getNodeType() == AST::Int ||
             node->getNodeType() == AST::String ||
             node->getNodeType() == AST::Switch) {
    std::shared_ptr<ValueNode> val = static_pointer_cast<ValueNode>(node);
    if (val) {
      val->setDomain(domain);
    }
  } else if (node->getNodeType() == AST::List ||
             node->getNodeType() == AST::Expression) {
    for (ASTNode member : node->getChildren()) {
      setDomainForNode(member, domain, scopeStack, tree, force);
    }
  } else if (node->getNodeType() == AST::PortProperty) {
    node->setCompilerProperty("domain", domain);
  }
}

bool CodeValidator::nodesAreEqual(ASTNode node1, ASTNode node2,
                                  ASTNode *output) {
  if (node1->getNodeType() == AST::Int && node2->getNodeType() == AST::Int) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() ==
            std::static_pointer_cast<ValueNode>(node2)->getIntValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Real) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() ==
            std::static_pointer_cast<ValueNode>(node2)->getRealValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Int) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() ==
            std::static_pointer_cast<ValueNode>(node2)->getIntValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Int &&
             node2->getNodeType() == AST::Real) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() ==
            std::static_pointer_cast<ValueNode>(node2)->getRealValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Switch &&
             node2->getNodeType() == AST::Switch) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getSwitchValue() ==
            std::static_pointer_cast<ValueNode>(node2)->getSwitchValue(),
        __FILE__, __LINE__);
    return true;
  }
  return false;
}

bool CodeValidator::nodesAreNotEqual(ASTNode node1, ASTNode node2,
                                     ASTNode *output) {
  bool ok = nodesAreEqual(node1, node2, output);
  if (ok) {
    *output = std::make_shared<ValueNode>(
        !std::static_pointer_cast<ValueNode>(*output)->getSwitchValue(),
        __FILE__, __LINE__);
  }
  return ok;
}

bool CodeValidator::nodesIsGreater(ASTNode node1, ASTNode node2,
                                   ASTNode *output) {
  if (node1->getNodeType() == AST::Int && node2->getNodeType() == AST::Int) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() >
            std::static_pointer_cast<ValueNode>(node2)->getIntValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Real) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() >
            std::static_pointer_cast<ValueNode>(node2)->getRealValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Int) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() >
            std::static_pointer_cast<ValueNode>(node2)->getIntValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Int &&
             node2->getNodeType() == AST::Real) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() >
            std::static_pointer_cast<ValueNode>(node2)->getRealValue(),
        __FILE__, __LINE__);
    return true;
  }
  return false;
}

bool CodeValidator::nodesIsNotGreater(ASTNode node1, ASTNode node2,
                                      ASTNode *output) {
  bool ok = nodesIsGreater(node1, node2, output);
  if (ok) {
    *output = std::make_shared<ValueNode>(
        !std::static_pointer_cast<ValueNode>(*output)->getSwitchValue(),
        __FILE__, __LINE__);
  }
  return ok;
}

bool CodeValidator::nodesIsLesser(ASTNode node1, ASTNode node2,
                                  ASTNode *output) {
  if (node1->getNodeType() == AST::Int && node2->getNodeType() == AST::Int) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() <
            std::static_pointer_cast<ValueNode>(node2)->getIntValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Real) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() <
            std::static_pointer_cast<ValueNode>(node2)->getRealValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Real &&
             node2->getNodeType() == AST::Int) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getRealValue() <
            std::static_pointer_cast<ValueNode>(node2)->getIntValue(),
        __FILE__, __LINE__);
    return true;
  } else if (node1->getNodeType() == AST::Int &&
             node2->getNodeType() == AST::Real) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getIntValue() <
            std::static_pointer_cast<ValueNode>(node2)->getRealValue(),
        __FILE__, __LINE__);
    return true;
  }
  return false;
}

bool CodeValidator::nodesIsNotLesser(ASTNode node1, ASTNode node2,
                                     ASTNode *output) {
  bool ok = nodesIsLesser(node1, node2, output);
  if (ok) {
    *output = std::make_shared<ValueNode>(
        !std::static_pointer_cast<ValueNode>(*output)->getSwitchValue(),
        __FILE__, __LINE__);
  }
  return ok;
}

bool CodeValidator::nodesOr(ASTNode node1, ASTNode node2, ASTNode *output) {
  if (node1->getNodeType() == AST::Switch &&
      node2->getNodeType() == AST::Switch) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getSwitchValue() ||
            std::static_pointer_cast<ValueNode>(node2)->getSwitchValue(),
        __FILE__, __LINE__);
    return true;
  }
  return false;
}

bool CodeValidator::nodesAnd(ASTNode node1, ASTNode node2, ASTNode *output) {
  if (node1->getNodeType() == AST::Switch &&
      node2->getNodeType() == AST::Switch) {
    *output = std::make_shared<ValueNode>(
        std::static_pointer_cast<ValueNode>(node1)->getSwitchValue() &&
            std::static_pointer_cast<ValueNode>(node2)->getSwitchValue(),
        __FILE__, __LINE__);
    return true;
  }
  return false;
}

bool CodeValidator::nodesXor(ASTNode node1, ASTNode node2, ASTNode *output) {
  if (node1->getNodeType() == AST::Switch &&
      node2->getNodeType() == AST::Switch) {
    return nodesAreNotEqual(node1, node2, output);
  }
  return false;
}

bool CodeValidator::nodeNot(ASTNode node1, ASTNode *output) {
  if (node1->getNodeType() == AST::Switch) {
    *output = std::make_shared<ValueNode>(
        !std::static_pointer_cast<ValueNode>(node1)->getSwitchValue(), __FILE__,
        __LINE__);
    return true;
  }
  return false;
}

bool CodeValidator::nodeIsNone(ASTNode node1, ASTNode *output) {
  *output = std::make_shared<ValueNode>(node1->getNodeType() == AST::None,
                                        __FILE__, __LINE__);
  return true;
}
