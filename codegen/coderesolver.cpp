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

#include "coderesolver.h"

#include <QDebug>
#include <iostream>

#include "codevalidator.h"
#include "stridesystem.hpp"

CodeResolver::CodeResolver(ASTNode tree, QString striderootDir,
                           SystemConfiguration systemConfig)
    : m_systemConfig(systemConfig), m_tree(tree), m_connectorCounter(0) {
  std::vector<std::shared_ptr<ImportNode>> importList =
      CodeValidator::getImportNodes(tree);
  std::vector<std::shared_ptr<SystemNode>> systems =
      CodeValidator::getSystemNodes(tree);

  if (systems.size() > 0) {
    std::shared_ptr<SystemNode> platformNode = systems.at(0);
    m_system = std::make_shared<StrideSystem>(
        striderootDir, QString::fromStdString(platformNode->platformName()),
        platformNode->majorVersion(), platformNode->minorVersion(), importList);
    m_system->m_systemConfig = systemConfig;
    for (size_t i = 1; i < systems.size(); i++) {
      qDebug() << "Ignoring system: "
               << QString::fromStdString(platformNode->platformName());
      LangError error;
      error.type = LangError::SystemRedefinition;
      error.errorTokens.push_back(platformNode->platformName());
      error.filename = platformNode->getFilename();
      error.lineNumber = platformNode->getLine();
      //      m_errors.append(error);
    }
  } else {  // Make a default platform that only inlcudes the common library
    m_system = std::make_shared<StrideSystem>(striderootDir, __FILE__, __LINE__,
                                              -1, importList);
  }
}

CodeResolver::~CodeResolver() {}

void CodeResolver::process() {
  processSystem();
  // Insert objects
  insertBuiltinObjects();
  fillDefaultProperties();
  processAnoymousDeclarations();
  declareModuleInternalBlocks();

  // Resolve and massage tree
  expandParallel();  // Find better name this expands bundles, functions and
                     // declares undefined bundles
                     //    processResets();
  resolveStreamSymbols();
  insertBuiltinObjects();

  if (m_systemConfig.testing) {
    enableTesting();
    insertBuiltinObjects();
    fillDefaultProperties();
  }
  resolveConstants();
  //  printTree();

  processDeclarations();
  processDomains();
  resolveRates();
  // Prepare additional metadata
  storeDeclarations();
  analyzeConnections();
  analyzeParents();
}

void CodeResolver::processSystem() {
  std::vector<std::shared_ptr<ImportNode>> importList;
  for (ASTNode node : m_tree->getChildren()) {
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

  // Process includes for system
  // Import root library
  ASTNode importTree = m_system->loadImportTree("", "");
  m_importTrees[""] = std::vector<ASTNode>();
  if (importTree) {
    m_importTrees[""].push_back(importTree);
  }

  auto frameworkNames = m_system->getFrameworkNames();
  frameworkNames.insert(frameworkNames.begin(),
                        std::string());  // Add library as well

  for (auto import : importList) {
    std::string importName = import->importName();
    std::string importAlias = import->importAlias();
    ASTNode importTree = m_system->loadImportTree(importName, importAlias);
    if (importTree) {
      m_importTrees[importAlias].push_back(importTree);
    } else {
      vector<LangError> errors = AST::getParseErrors();
      if (errors.size() > 0) {
        qDebug() << "Cannot import '" << QString::fromStdString(importName)
                 << "'. Ignoring.";
        for (LangError error : errors) {
          qDebug() << QString::fromStdString(error.getErrorText());
        }
      }
    }
  }

  auto platformDomain = m_system->getPlatformDomain();

  if (platformDomain) {
    for (auto objects : m_system->getBuiltinObjectsReference()) {
      auto domainDecl = CodeValidator::findDeclaration(
          CodeValidator::streamMemberName(platformDomain),
          {{std::string(), objects.second}}, m_tree);
      if (domainDecl) {
        m_tree->addChild(domainDecl);
        break;
      }
    }
  }
}

void CodeResolver::resolveRates() {
  vector<ASTNode> children = m_tree->getChildren();
  // First go through backwards to prioritize pull
  vector<ASTNode>::reverse_iterator rit = children.rbegin();
  while (rit != children.rend()) {
    ASTNode node = *rit;
    if (node->getNodeType() == AST::Stream) {
      resolveStreamRatesReverse(static_pointer_cast<StreamNode>(node));
    }
    rit++;
  }
  // Then do it again from the top to try to resolve the rest
  for (ASTNode node : children) {
    if (node->getNodeType() == AST::Stream) {
      resolveStreamRates(static_pointer_cast<StreamNode>(node));
    }
  }
}

void CodeResolver::resolveStreamRatesReverse(
    std::shared_ptr<StreamNode> stream) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();
  double rate = CodeValidator::getNodeRate(left, {}, m_tree);
  double rightRate = -1;
  if (right->getNodeType() == AST::Stream) {
    resolveStreamRatesReverse(static_pointer_cast<StreamNode>(right));
    rightRate = CodeValidator::getNodeRate(
        static_cast<StreamNode *>(right.get())->getLeft(), {}, m_tree);
  } else {
    rightRate = CodeValidator::getNodeRate(right, {}, m_tree);
  }
  if (rate < 0 && rightRate >= 0) {
    CodeValidator::setNodeRate(left, rightRate, {}, m_tree);
  }
  //    Q_ASSERT(rate != -1);
  //    stream->setRate(rate);
}

void CodeResolver::resolveStreamRates(std::shared_ptr<StreamNode> stream) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();
  double rate = CodeValidator::getNodeRate(left, {}, m_tree);
  if (rate < 0) {
    // FIXME we should resolve rate to rate in the block's domain, then force to
    // platform rate Force node rate to platform rate
    if (m_system && m_system->getPlatformDomain()) {
      std::shared_ptr<DeclarationNode> domainDeclaration =
          CodeValidator::findDomainDeclaration(
              CodeValidator::streamMemberName(m_system->getPlatformDomain()),
              m_tree);
      if (domainDeclaration) {
        ASTNode rateValue = domainDeclaration->getPropertyValue("rate");
        if (rateValue->getNodeType() == AST::Int ||
            rateValue->getNodeType() == AST::Real) {
          rate = static_cast<ValueNode *>(rateValue.get())->toReal();
          CodeValidator::setNodeRate(left, rate, {}, m_tree);
        } else if (rateValue->getNodeType() == AST::PortProperty) {
          if (left->getNodeType() == AST::Declaration) {
            auto decl = static_pointer_cast<DeclarationNode>(left);
            decl->replacePropertyValue("rate", rateValue->deepCopy());
          }
        } else {
          qDebug() << "Unexpected type for rate in domain declaration: "
                   << QString::fromStdString(CodeValidator::streamMemberName(
                          m_system->getPlatformDomain()));
        }
      }
    }
  }
  double rightRate = -1;
  if (right->getNodeType() == AST::Stream) {
    rightRate = CodeValidator::getNodeRate(
        static_cast<StreamNode *>(right.get())->getLeft(), {}, m_tree);
    if (rightRate <= 0 && rate >= 0) {
      CodeValidator::setNodeRate(
          static_cast<StreamNode *>(right.get())->getLeft(), rate, {}, m_tree);
    }
    resolveStreamRates(static_pointer_cast<StreamNode>(right));
  } else {
    rightRate = CodeValidator::getNodeRate(right, {}, m_tree);
    if (rightRate <= 0) {
      if (rate >= 0) {
        CodeValidator::setNodeRate(right, rate, {}, m_tree);
      } else {
        auto rightDomain = CodeValidator::getNodeDomain(right, {}, m_tree);
        if (rightDomain) {
          auto domainDecl = CodeValidator::findDomainDeclaration(
              CodeValidator::getDomainIdentifier(rightDomain, {}, m_tree),
              m_tree);
          if (domainDecl) {
            auto defaultRate = CodeValidator::getDomainDefaultRate(domainDecl);
            CodeValidator::setNodeRate(right, defaultRate, {}, m_tree);

          } else {
            std::cout << __FILE__ << ":" << __LINE__
                      << " ERROR: can't find domain declaration" << std::endl;
          }
        }
      }
    }
  }
}

void CodeResolver::fillDefaultPropertiesForNode(ASTNode node, ASTNode tree) {
  if (node->getNodeType() == AST::Declaration ||
      node->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> destBlock =
        static_pointer_cast<DeclarationNode>(node);
    vector<std::shared_ptr<PropertyNode>> blockProperties =
        destBlock->getProperties();
    auto frameworkNode = node->getCompilerProperty("framework");
    std::string frameworkName;
    if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
      frameworkName =
          static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    QVector<ASTNode> typeProperties = CodeValidator::getPortsForType(
        destBlock->getObjectType(), {}, tree, destBlock->getNamespaceList(),
        frameworkName);
    if (typeProperties.isEmpty()) {
      qDebug()
          << "ERROR: fillDefaultPropertiesForNode() No type definition for "
          << QString::fromStdString(destBlock->getObjectType());
      return;
    }
    for (auto property : blockProperties) {
      fillDefaultPropertiesForNode(property->getValue(), tree);
    }

    for (ASTNode propertyListMember : typeProperties) {
      Q_ASSERT(propertyListMember->getNodeType() == AST::Declaration);
      DeclarationNode *portDescription =
          static_cast<DeclarationNode *>(propertyListMember.get());
      ASTNode propName = portDescription->getPropertyValue("name");
      Q_ASSERT(propName->getNodeType() == AST::String);
      string propertyName =
          static_cast<ValueNode *>(propName.get())->getStringValue();
      bool propertySet = false;
      for (auto blockProperty : blockProperties) {
        if (blockProperty->getName() == propertyName) {
          propertySet = true;
          break;
        }
      }
      if (!propertySet) {
        ASTNode defaultValueNode = portDescription->getPropertyValue("default");
        std::shared_ptr<PropertyNode> newProperty =
            std::make_shared<PropertyNode>(
                propertyName, defaultValueNode->deepCopy(),
                portDescription->getFilename().data(),
                portDescription->getLine());
        destBlock->addProperty(newProperty);
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> destFunc =
        static_pointer_cast<FunctionNode>(node);
    vector<std::shared_ptr<PropertyNode>> blockProperties =
        destFunc->getProperties();
    std::shared_ptr<DeclarationNode> functionModule =
        CodeValidator::findDeclaration(
            QString::fromStdString(destFunc->getName()), {}, tree);
    if (functionModule) {
      if (functionModule->getObjectType() == "module" ||
          functionModule->getObjectType() == "reaction" ||
          functionModule->getObjectType() == "loop") {
        vector<ASTNode> typeProperties =
            functionModule->getPropertyValue("ports")->getChildren();
        if (!functionModule->getPropertyValue("ports")) {
          qDebug() << "ERROR: fillDefaultProperties() No type definition for "
                   << QString::fromStdString(destFunc->getName());
          return;
        }
        for (std::shared_ptr<PropertyNode> property : blockProperties) {
          fillDefaultPropertiesForNode(property->getValue(), tree);
        }

        for (ASTNode propertyListMember : typeProperties) {
          Q_ASSERT(propertyListMember->getNodeType() == AST::Declaration);
          DeclarationNode *propertyDecl =
              static_cast<DeclarationNode *>(propertyListMember.get());

          if (propertyDecl->getObjectType().substr(0, 8) == "property") {
            auto propertyNameValue = propertyDecl->getPropertyValue("name");
            if (propertyNameValue &&
                propertyNameValue->getNodeType() == AST::String) {
              bool propertySet = false;
              string propertyName =
                  static_pointer_cast<ValueNode>(propertyNameValue)
                      ->getStringValue();
              for (std::shared_ptr<PropertyNode> blockProperty :
                   blockProperties) {
                if (blockProperty->getName() == propertyName) {
                  propertySet = true;
                  break;
                }
              }
              if (!propertySet) {
                ASTNode defaultValueNode =
                    propertyDecl->getPropertyValue("default");
                if (defaultValueNode) {
                  std::shared_ptr<PropertyNode> newProperty =
                      std::make_shared<PropertyNode>(
                          propertyName, defaultValueNode,
                          propertyDecl->getFilename().data(),
                          propertyDecl->getLine());
                  destFunc->addProperty(newProperty);
                }
              }
            }
          }
        }

      } else if (functionModule->getObjectType() == "platformModule") {
        // Is there anything to do here for platform modules?
      }
    }
  } else if (node->getNodeType() == AST::List ||
             node->getNodeType() == AST::Expression) {
    ListNode *list = static_cast<ListNode *>(node.get());
    for (ASTNode listElement : list->getChildren()) {
      fillDefaultPropertiesForNode(listElement, tree);
    }
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *stream = static_cast<StreamNode *>(node.get());
    for (ASTNode streamElement : stream->getChildren()) {
      fillDefaultPropertiesForNode(streamElement, tree);
    }
  }
}

void CodeResolver::analyzeChildConnections(ASTNode node,
                                           ScopeStack scopeStack) {
  for (ASTNode object : node->getChildren()) {
    //        We need to check streams on the root but also streams within
    //        modules and reactions
    if (object->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          static_pointer_cast<DeclarationNode>(object);
      if (decl->getObjectType() == "module" ||
          decl->getObjectType() == "reaction" ||
          decl->getObjectType() == "loop") {
        std::vector<ASTNode> streams = getModuleStreams(decl);
        ASTNode blocks = decl->getPropertyValue("blocks");
        scopeStack.push_back({decl->getName(), blocks->getChildren()});
        for (ASTNode stream : streams) {
          //          Q_ASSERT(stream->getNodeType() == AST::Stream);
          if (stream->getNodeType() == AST::Stream) {
            checkStreamConnections(static_pointer_cast<StreamNode>(stream),
                                   scopeStack);
          }
        }
        // prepend current scope
        analyzeChildConnections(blocks, scopeStack);
      }

    } else if (object->getNodeType() == AST::Stream) {
      checkStreamConnections(static_pointer_cast<StreamNode>(object),
                             ScopeStack());
    }
  }
}

void CodeResolver::fillDefaultProperties() {
  vector<ASTNode> nodes = m_tree->getChildren();
  for (unsigned int i = 0; i < nodes.size(); i++) {
    ASTNode node = nodes.at(i);
    fillDefaultPropertiesForNode(node, m_tree);
  }
}

void CodeResolver::enableTesting() {
  auto treeChildren = m_tree->getChildren();
  for (auto platform : m_system->getFrameworks()) {
    vector<ASTNode> testingObjs = platform->getPlatformTestingObjectsRef();
    for (size_t i = 0; i < treeChildren.size(); i++) {
      if (treeChildren[i]->getNodeType() == AST::Declaration ||
          treeChildren[i]->getNodeType() == AST::BundleDeclaration) {
        std::shared_ptr<DeclarationNode> decl =
            static_pointer_cast<DeclarationNode>(treeChildren[i]);
        for (ASTNode testingObj : testingObjs) {
          if (testingObj->getNodeType() == AST::Declaration ||
              testingObj->getNodeType() == AST::BundleDeclaration) {
            std::shared_ptr<DeclarationNode> testDecl =
                static_pointer_cast<DeclarationNode>(testingObj);
            if (decl->getName() ==
                testDecl
                    ->getName()) {  // FIXME we need to check for namespace too
              treeChildren[i] = testDecl;
              break;
            }
          } else {
            qDebug() << "Unexpected node in testing file.";
          }
        }
      }
    }
  }
  m_tree->setChildren(treeChildren);
}

void CodeResolver::declareModuleInternalBlocks() {
  for (ASTNode node : m_tree->getChildren()) {
    declareInternalBlocksForNode(node, {});
  }

  //    populateContextDomains(m_tree->getChildren());
}

void CodeResolver::expandParallelStream(std::shared_ptr<StreamNode> stream,
                                        ScopeStack scopeStack, ASTNode tree) {
  QList<LangError> errors;
  std::shared_ptr<StreamNode> subStream = stream;

  // Figure out stream IO sizes
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();
  QVector<QPair<int, int>> IOs;
  while (right) {
    if (left->getNodeType() ==
        AST::Function) {  // Expand from properties size to list
      ASTNode newFunctions = expandFunctionFromProperties(
          static_pointer_cast<FunctionNode>(left), scopeStack, tree);
      if (newFunctions) {
        subStream->setLeft(newFunctions);
        left = subStream->getLeft();
      }
    }
    QPair<int, int> io;
    io.first = CodeValidator::getNodeNumInputs(left, scopeStack, tree, errors);
    io.second =
        CodeValidator::getNodeNumOutputs(left, scopeStack, tree, errors);
    IOs << io;
    if (right->getNodeType() == AST::Stream) {
      subStream = static_pointer_cast<StreamNode>(right);
      left = subStream->getLeft();
      right = subStream->getRight();
    } else {
      if (right->getNodeType() == AST::Function) {
        ASTNode newFunctions = expandFunctionFromProperties(
            static_pointer_cast<FunctionNode>(right), scopeStack, tree);
        if (newFunctions) {
          subStream->setRight(newFunctions);
          right = subStream->getRight();
        }
      }
      io.first =
          CodeValidator::getNodeNumInputs(right, scopeStack, tree, errors);
      io.second =
          CodeValidator::getNodeNumOutputs(right, scopeStack, tree, errors);
      IOs << io;
      right = nullptr;
    }
  }
  // Now go through comparing number of outputs to number of inputs to figure
  // out if we need to duplicate any members
  QVector<int> numCopies;
  numCopies << 1;
  for (int i = 1; i < IOs.size(); ++i) {
    int numPrevOut = IOs[i - 1].second * numCopies.back();
    int numCurIn = IOs[i].first;
    if (numPrevOut == -1) {  // Found undeclared block
      numCopies << 1;
      continue;
    }
    if (numPrevOut > numCurIn) {  // Need to clone next
      if (numCurIn > 0) {
        if (numPrevOut / (float)numCurIn == numPrevOut / numCurIn) {
          numCopies << numPrevOut / numCurIn;
        } else {
          // Stream size mismatch. Stop expansion. The error will be reported
          // later by CodeValidator.
          numCopies << 1;
          qDebug() << "Could not clone " << IOs[i - 1].second * numCopies.back()
                   << " outputs into " << IOs[i].first << " inputs.";
        }
      } else {
        // Cloning with size 1
        numCopies << 1;
      }
    } else if (numPrevOut < numCurIn &&
               numPrevOut > 0) {  // Need to clone all existing left side
      if (numCurIn / (float)numPrevOut == numCurIn / numPrevOut) {
        //                        int newNumCopies = numCurIn/numPrevOut;
        //                        for(int i = 0; i < numCopies.size(); ++i) {
        //                            numCopies[i] *= newNumCopies;
        //                        }
        // Should not be expanded but connected recursively and interleaved
        // (according to the rules of the language)
        numCopies << 1;
      } else {
        // Stream size mismatch. Stop expansion. The error will be reported
        // later by CodeValidator.
        qDebug() << "Could not clone " << IOs[i - 1].second << " outputs into "
                 << IOs[i].first << " inputs.";
        numCopies << 1;
      }

    } else {  // Size match, no need to clone
      numCopies << 1;
    }
  }
  if (numCopies.size() ==
      IOs.size()) {  // Expansion calculation went fine, so expand
                     //                qDebug() << "Will expand";
    expandStreamToSizes(stream, numCopies, -1, scopeStack);
  }
}

void CodeResolver::processDeclarations() {
  //    For block and bundle declarations move streams and blocks outside
  //    properties to the "streams" and "blocks" properties

  std::function<void(std::vector<ASTNode>)> processDeclarationsForTree =
      [&](std::vector<ASTNode> children) {
        for (ASTNode node : children) {
          if (node->getNodeType() == AST::Declaration ||
              node->getNodeType() == AST::BundleDeclaration) {
            std::shared_ptr<DeclarationNode> decl =
                static_pointer_cast<DeclarationNode>(node);
            std::vector<ASTNode> toDelete;
            for (auto prop : decl->getChildren()) {
              if (prop->getNodeType() == AST::Stream) {
                toDelete.push_back(prop);
                if (!decl->getPropertyValue("streams") ||
                    decl->getPropertyValue("streams")->getNodeType() ==
                        AST::None) {
                  auto streamList = std::make_shared<ListNode>(
                      prop, prop->getFilename().c_str(), prop->getLine());
                  decl->setPropertyValue("streams", streamList);
                } else if (decl->getPropertyValue("streams")->getNodeType() !=
                           AST::List) {
                  auto streamList = std::make_shared<ListNode>(
                      decl->getPropertyValue("streams"), __FILE__, __LINE__);
                  streamList->addChild(node);
                  decl->setPropertyValue("streams", streamList);
                } else {
                  decl->getPropertyValue("streams")->addChild(node);
                }
              } else if (prop->getNodeType() == AST::Declaration ||
                         prop->getNodeType() == AST::BundleDeclaration) {
                toDelete.push_back(prop);
                if (!decl->getPropertyValue("blocks") ||
                    decl->getPropertyValue("blocks")->getNodeType() ==
                        AST::None) {
                  auto streamList = std::make_shared<ListNode>(
                      prop, prop->getFilename().c_str(), prop->getLine());
                  decl->setPropertyValue("blocks", streamList);
                } else if (decl->getPropertyValue("blocks")->getNodeType() !=
                           AST::List) {
                  auto streamList = std::make_shared<ListNode>(
                      decl->getPropertyValue("blocks"), __FILE__, __LINE__);
                  streamList->addChild(node);
                  decl->setPropertyValue("blocks", streamList);
                } else {
                  decl->getPropertyValue("blocks")->addChild(node);
                }
              } else if (prop->getNodeType() == AST::Property) {
                if (static_pointer_cast<PropertyNode>(prop)
                        ->getValue()
                        ->getNodeType() == AST::List) {
                  processDeclarationsForTree(
                      static_pointer_cast<PropertyNode>(prop)
                          ->getValue()
                          ->getChildren());
                } else {
                  processDeclarationsForTree(std::vector<ASTNode>{
                      static_pointer_cast<PropertyNode>(prop)->getValue()});
                }
              }
            }
            for (auto deleteme : toDelete) {
              decl->removeProperty(deleteme);
            }
          } else {
            processDeclarationsForTree(node->getChildren());
          }
        }
      };
  processDeclarationsForTree(m_tree->getChildren());
}

void CodeResolver::expandParallel() {
  std::vector<ASTNode> children = m_tree->getChildren();
  for (ASTNode node : children) {
    ScopeStack scopeStack;
    if (node->getNodeType() == AST::Stream) {
      std::shared_ptr<StreamNode> stream =
          static_pointer_cast<StreamNode>(node);
      expandParallelStream(stream, scopeStack, m_tree);
      // We need to process unknown symbols to make sure we can expand the
      // following streams to the right size.
      std::vector<ASTNode> declarations =
          declareUnknownStreamSymbols(stream, nullptr, ScopeStack(), m_tree);
      for (ASTNode decl : declarations) {
        m_tree->addChild(decl);
      }
    }
  }
}

void CodeResolver::processAnoymousDeclarations() {
  auto processAnonDeclInStream = [&](std::shared_ptr<StreamNode> stream,
                                     ASTNode scopeTree) {
    auto node = stream->getLeft();
    do {
      if (node->getNodeType() == AST::Declaration) {
        auto decl = static_pointer_cast<DeclarationNode>(node);
        auto newBlock = std::make_shared<FunctionNode>(
            decl->getName(), std::make_shared<ListNode>(__FILE__, __LINE__),
            __FILE__, __LINE__);
        scopeTree->addChild(node);
        if (node == stream->getLeft()) {
          stream->setLeft(newBlock);
        } else {
          stream->setRight(newBlock);
          break;
        }
      }
      if (stream->getRight() == node) {
        node = nullptr;
      } else if (stream->getRight()->getNodeType() == AST::Stream) {
        stream = static_pointer_cast<StreamNode>(stream->getRight());
        node = stream->getLeft();
      } else {
        node = stream->getRight();
      }
    } while (node);
  };

  std::function<ASTNode(ASTNode)> processAnonDecls = [&](ASTNode scopeTree) {
    ASTNode newBlocks = std::make_shared<ListNode>(__FILE__, __LINE__);
    for (auto node : scopeTree->getChildren()) {
      if (node->getNodeType() == AST::Stream) {
        processAnonDeclInStream(static_pointer_cast<StreamNode>(node),
                                newBlocks);
      } else if (node->getNodeType() == AST::Declaration ||
                 node->getNodeType() == AST::BundleDeclaration) {
        auto decl = static_pointer_cast<DeclarationNode>(node);
        if (decl->getObjectType() == "reaction" ||
            decl->getObjectType() == "module" ||
            decl->getObjectType() == "loop") {
          auto streams = decl->getPropertyValue("streams");
          auto blocks = decl->getPropertyValue("blocks");
          Q_ASSERT(streams && blocks);
          if (streams && blocks) {
            for (auto stream : streams->getChildren()) {
              if (stream->getNodeType() == AST::Stream) {
                processAnonDeclInStream(static_pointer_cast<StreamNode>(stream),
                                        blocks);
              }
            }

            auto newInternalBlocks = processAnonDecls(blocks);
            for (auto node : newInternalBlocks->getChildren()) {
              blocks->addChild(node);
            }
          }
        }
      }
    }

    return newBlocks;
  };

  auto newBlocks = processAnonDecls(m_tree);

  for (auto node : newBlocks->getChildren()) {
    m_tree->addChild(node);
  }
}

void CodeResolver::expandStreamToSizes(std::shared_ptr<StreamNode> stream,
                                       QVector<int> &neededCopies,
                                       int previousOutSize,
                                       ScopeStack scopeStack) {
  ASTNode left = stream->getLeft();
  //    int leftSize = CodeValidator::getNodeSize(left, scopeStack, m_tree);
  if (previousOutSize == -1) {
    previousOutSize = 1;
  }

  if (left->getNodeType() == AST::Function) {
    int numCopies = neededCopies.front();
    //         if (leftSize < 0 && left->getNodeType() == AST::Block) {
    //            std::vector<ASTNode > newDeclaration =
    //            declareUnknownName(static_pointer_cast<BlockNode>(left),
    //            previousOutSize, scopeStack, m_tree); for(ASTNode
    //            decl:newDeclaration) {
    //                m_tree->addChild(decl);
    //            }
    //        }
    if (numCopies > 1 && left->getNodeType() == AST::Function) {
      std::shared_ptr<ListNode> newLeft = std::make_shared<ListNode>(
          left, left->getFilename().data(), left->getLine());
      for (int i = 1; i < numCopies; i++) {
        newLeft->addChild(left->deepCopy());
      }
      stream->setLeft(newLeft);
    }
  }
  QList<LangError> errors;
  previousOutSize = CodeValidator::getNodeNumOutputs(
      stream->getLeft(), scopeStack, m_tree, errors);
  if (previousOutSize < 0) {
    previousOutSize = 1;
  }
  neededCopies.pop_front();
  ASTNode right = stream->getRight();
  if (right->getNodeType() == AST::Stream) {
    expandStreamToSizes(static_pointer_cast<StreamNode>(right), neededCopies,
                        previousOutSize, scopeStack);
  } else {
    int rightSize = CodeValidator::getNodeSize(right, scopeStack, m_tree);
    if (right->getNodeType() == AST::Block ||
        right->getNodeType() == AST::Function) {
      int numCopies = neededCopies.front();
      if (rightSize < 0 && right->getNodeType() == AST::Block) {
        std::vector<ASTNode> newDeclaration =
            declareUnknownName(static_pointer_cast<BlockNode>(right),
                               previousOutSize, scopeStack, m_tree);
        for (ASTNode decl : newDeclaration) {
          m_tree->addChild(decl);
        }
      } else if (numCopies > 1 && right->getNodeType() ==
                                      AST::Function) {  // Only functions should
                                                        // be expanded this way
        std::shared_ptr<ListNode> newRight = std::make_shared<ListNode>(
            right, right->getFilename().data(), right->getLine());
        for (int i = 1; i < numCopies; i++) {
          newRight->addChild(right);
        }
        stream->setRight(newRight);
      }
    }
    neededCopies.pop_front();
    Q_ASSERT(neededCopies.size() ==
             0);  // This is the end of the stream there should be no sizes left
  }
}

ASTNode CodeResolver::expandFunctionFromProperties(
    std::shared_ptr<FunctionNode> func, ScopeStack scopeStack, ASTNode tree) {
  QList<LangError> errors;
  std::shared_ptr<ListNode> newFunctions = nullptr;
  //    int dataSize = CodeValidator::getFunctionDataSize(func, scopeStack,
  //    tree, errors);
  int dataSize = CodeValidator::getFunctionNumInstances(func, scopeStack, tree);
  if (dataSize > 1) {
    vector<std::shared_ptr<PropertyNode>> props = func->getProperties();
    newFunctions = std::make_shared<ListNode>(
        nullptr, func->getFilename().c_str(), func->getLine());
    for (int i = 0; i < dataSize;
         ++i) {  // FIXME this assumes each function takes a single input. Need
                 // to check the actual input size.
      newFunctions->addChild(func->deepCopy());
    }
    for (auto prop : props) {
      ASTNode value = prop->getValue();
      int numOuts =
          CodeValidator::getNodeNumOutputs(value, scopeStack, tree, errors);
      if (numOuts != 1 && numOuts != dataSize) {
        LangError error;
        error.type = LangError::BundleSizeMismatch;
        error.filename = func->getFilename();
        error.lineNumber = func->getLine();
        error.errorTokens.push_back(func->getName());
        error.errorTokens.push_back(QString::number(numOuts).toStdString());
        error.errorTokens.push_back(QString::number(dataSize).toStdString());
        errors << error;
        return nullptr;
      }
      if (numOuts == 1) {  // Single value given, duplicate for all copies.
        for (ASTNode newFunction : newFunctions->getChildren()) {
          newFunction->addChild(prop->deepCopy());
        }
      } else {
        if (value->getNodeType() == AST::Bundle) {
          // FIXME write support for ranges

        } else if (value->getNodeType() == AST::Block) {
          BlockNode *name = static_cast<BlockNode *>(value.get());
          std::shared_ptr<DeclarationNode> block =
              CodeValidator::findDeclaration(
                  QString::fromStdString(name->getName()), scopeStack, tree,
                  name->getNamespaceList());
          int size = CodeValidator::getBlockDeclaredSize(block, scopeStack,
                                                         tree, errors);
          Q_ASSERT(size == dataSize);
          for (int i = 0; i < size; ++i) {
            std::shared_ptr<PropertyNode> newProp =
                static_pointer_cast<PropertyNode>(prop->deepCopy());
            std::shared_ptr<ListNode> indexList = std::make_shared<ListNode>(
                std::make_shared<ValueNode>(i + 1, prop->getFilename().c_str(),
                                            prop->getLine()),
                prop->getFilename().c_str(), prop->getLine());
            std::shared_ptr<BundleNode> newBundle =
                std::make_shared<BundleNode>(name->getName(), indexList,
                                             prop->getFilename().c_str(),
                                             prop->getLine());
            newProp->replaceValue(newBundle);
            static_pointer_cast<FunctionNode>(newFunctions->getChildren()[i])
                ->addChild(newProp);
          }

        } else if (value->getNodeType() == AST::List) {
          // FIXME we need to split the list according to the expected size.
          // This currently assumes size == 1
          vector<ASTNode> values =
              static_pointer_cast<ListNode>(value)->getChildren();
          vector<ASTNode> functions = newFunctions->getChildren();
          Q_ASSERT(values.size() == functions.size());
          for (size_t i = 0; i < (size_t)dataSize; ++i) {
            std::shared_ptr<PropertyNode> newProp =
                static_pointer_cast<PropertyNode>(prop);
            static_pointer_cast<FunctionNode>(functions[i])
                ->setPropertyValue(newProp->getName(), values[i]->deepCopy());
          }
        } else {
          qDebug() << "Error. Don't know how to expand property.";
        }
      }
    }
  }
  return newFunctions;
}

void CodeResolver::insertBuiltinObjects() {
  QList<std::shared_ptr<DeclarationNode>> usedDeclarations;

  // First pass to add the fundamental types
  //  for (ASTNode object : m_importTrees[""]) {
  //    if (object->getNodeType() == AST::Declaration) {
  //      std::shared_ptr<DeclarationNode> block =
  //          static_pointer_cast<DeclarationNode>(object);
  //      if (block->getObjectType() == "type") {
  //        ASTNode nameNode = block->getPropertyValue("typeName");
  //        if (nameNode->getNodeType() == AST::String) {
  //          ValueNode *typeName = static_cast<ValueNode *>(nameNode.get());
  //          if (typeName->getStringValue() == "signal"
  //                            /*|| typeName->getStringValue() ==
  //                            "propertyInputPort"
  //                            || typeName->getStringValue() ==
  //                            "propertyOutputPort"
  //                            || typeName->getStringValue() == "reaction"
  //                            || typeName->getStringValue() ==
  //                            "signalbridge"*/) {
  //            m_tree->addChild(block);
  //            usedDeclarations << block;
  //          }
  //          if (typeName->getStringValue() == "type" ||
  //              typeName->getStringValue() == "platformModule" ||
  //              typeName->getStringValue() == "platformBlock") {
  //            m_tree->addChild(block);
  //            usedDeclarations << block;
  //          }
  //        }
  //      } else if (block->getObjectType() == "alias") {
  //        if (block->getName() == "PlatformDomain" ||
  //            block->getName() == "PlatformRate") {
  //          m_tree->addChild(block);
  //          usedDeclarations << block;
  //        }
  //      } else if (block->getObjectType() == "_domainDefinition" ||
  //                 block->getObjectType() ==
  //                     "_frameworkDescription") // Hack... this should be
  //                                              // getting inserted when
  //                                              // resolving symbols...)
  //      {
  //        m_tree->addChild(block);
  //        usedDeclarations << block;
  //      } else if (/*block->getName() == "_PlatformDomainProcessing" ||*/
  //                 block->getName() == "_GlobalInitTag") {
  //        m_tree->addChild(block);
  //        usedDeclarations << block;
  //      }
  //      //            continue;
  //    }
  //  }
  //  for (ASTNode decl : usedDeclarations) {
  //    m_importTrees[""].erase(
  //        std::find(m_importTrees[""].begin(), m_importTrees[""].end(),
  //        decl));
  //    insertBuiltinObjectsForNode(decl, m_importTrees, m_tree);
  //  }

  // Second pass to add elements that depend on the user's code
  auto children = m_tree->getChildren();
  for (ASTNode object : children) {
    insertBuiltinObjectsForNode(object, m_importTrees, m_tree);
  }
}

void CodeResolver::processDomains() {
  // Fill missing domain information (propagate domains)
  // First we need to traverse the streams backwards to make sure we propagate
  // the domains from the furthest point down the line
  vector<ASTNode> children = m_tree->getChildren();
  vector<ASTNode>::reverse_iterator rit = children.rbegin();
  ScopeStack scopeStack;  // = QVector<AST*>::fromStdVector(children);

  ASTNode domainBlock = m_system->getPlatformDomain();
  mContextDomainStack.push_back(domainBlock);
  while (rit != children.rend()) {
    ASTNode node = *rit;
    propagateDomainsForNode(node, scopeStack);
    rit++;
  }

  // Any signals without domain are assigned to the platform domain
  // TODO: Do we need this? It seems that any signals without domain at this
  // point are unused
  //    if (domainDecl) {
  //        setContextDomain(m_tree->getChildren(),domainDecl);
  //    }
  // Now split streams when there is a domain change
  vector<ASTNode> new_tree;
  for (ASTNode node : m_tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      std::vector<ASTNode> streams = sliceStreamByDomain(
          static_pointer_cast<StreamNode>(node), ScopeStack());
      for (ASTNode stream : streams) {
        new_tree.push_back(stream);
      }
    } else if (node->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> module =
          static_pointer_cast<DeclarationNode>(node);
      sliceDomainsInNode(module, ScopeStack());
      new_tree.push_back(module);
    } else {
      new_tree.push_back(node);
    }
  }

  m_tree->setChildren(new_tree);
}

void CodeResolver::analyzeConnections() {
  analyzeChildConnections(m_tree);

  std::vector<std::shared_ptr<DeclarationNode>> knownDomains;
  for (ASTNode node : m_tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "_domainDefinition") {
        knownDomains.push_back(decl);
        Q_ASSERT(!decl->getCompilerProperty("domainReads"));
        Q_ASSERT(!decl->getCompilerProperty("domainWrites"));
        decl->setCompilerProperty(
            "domainReads", std::make_shared<ListNode>(__FILE__, __LINE__));
        decl->setCompilerProperty(
            "domainWrites", std::make_shared<ListNode>(__FILE__, __LINE__));
      }
    }
  }

  // Now go through the tree to match domain reads and domain writes
  for (ASTNode node : m_tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl;
      if (node->getCompilerProperty("reads")) {
        for (auto domain : node->getCompilerProperty("reads")->getChildren()) {
          if (domain) {
            if (domain->getNodeType() == AST::String) {
              std::string domainName =
                  static_pointer_cast<ValueNode>(domain)->getStringValue();
              for (auto knownDomain : knownDomains) {
                ASTNode domainNameValue =
                    knownDomain->getPropertyValue("domainName");
                if (domainNameValue &&
                    domainNameValue->getNodeType() == AST::String) {
                  if (domainName ==
                      static_cast<ValueNode *>(domainNameValue.get())
                          ->getStringValue()) {
                    decl = knownDomain;
                    break;
                  }
                }
              }
              if (decl) {
                decl->getCompilerProperty("domainReads")->addChild(node);
              }
            } else if (domain->getNodeType() == AST::Block) {
              decl = CodeValidator::findDeclaration(
                  CodeValidator::streamMemberName(domain), {}, m_tree);
              if (decl) {
                decl->getCompilerProperty("domainReads")->addChild(node);
              }
            } else if (domain->getNodeType() == AST::None) {
              qDebug() << "Read domain is none. Read code will be generated "
                          "in declared domain.";
              auto domainNode =
                  static_pointer_cast<DeclarationNode>(node)->getPropertyValue(
                      "domain");
              if (domainNode && domainNode->getNodeType() != AST::None) {
                auto domainId =
                    CodeValidator::getDomainIdentifier(domainNode, {}, m_tree);
                auto domainDecl =
                    CodeValidator::findDomainDeclaration(domainId, m_tree);
                if (domainDecl) {
                  domainDecl->getCompilerProperty("domainReads")
                      ->addChild(node);
                }
              }

            } else {
              Q_ASSERT(domain->getNodeType() == AST::PortProperty);
            }
          }
        }
      }

      if (node->getCompilerProperty("writes")) {
        for (auto domain : node->getCompilerProperty("writes")->getChildren()) {
          if (domain) {
            if (domain->getNodeType() == AST::String) {
              std::string domainName =
                  static_pointer_cast<ValueNode>(domain)->getStringValue();
              for (auto knownDomain : knownDomains) {
                ASTNode domainNameValue =
                    knownDomain->getPropertyValue("domainName");
                if (domainNameValue &&
                    domainNameValue->getNodeType() == AST::String) {
                  if (domainName ==
                      static_cast<ValueNode *>(domainNameValue.get())
                          ->getStringValue()) {
                    decl = knownDomain;
                    break;
                  }
                }
              }
              if (decl) {
                decl->getCompilerProperty("domainWrites")->addChild(node);
              }
            } else if (domain->getNodeType() == AST::Block) {
              decl = CodeValidator::findDeclaration(
                  CodeValidator::streamMemberName(domain), {}, m_tree);
              if (decl) {
                decl->getCompilerProperty("domainReads")->addChild(node);
              }
            } else {
              if (domain->getNodeType() != AST::PortProperty &&
                  domain->getNodeType() != AST::None) {
                qDebug() << "Expected port property domain";
              }
            }
          }
        }
      }
    }
  }
}

void CodeResolver::storeDeclarations() {
  for (auto node : m_tree->getChildren()) {
    storeDeclarationsForNode(node, {}, m_tree);
  }
}

void CodeResolver::analyzeParents() {
  for (auto node : m_tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration ||
        node->getNodeType() == AST::BundleDeclaration) {
      appendParent(static_pointer_cast<DeclarationNode>(node), nullptr);
    }
  }
}

void CodeResolver::printTree() {
  for (auto node : m_tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration ||
        node->getNodeType() == AST::BundleDeclaration) {
      auto decl = static_pointer_cast<DeclarationNode>(node);

      std::cout << decl->getObjectType() << " " << decl->getName();
      auto framework = decl->getCompilerProperty("framework");
      if (framework && framework->getNodeType() == AST::String) {
        std::cout
            << "@"
            << static_pointer_cast<ValueNode>(framework)->getStringValue();
      }

      std::cout << std::endl;
    }
  }
}

void CodeResolver::insertDependentTypes(
    std::shared_ptr<DeclarationNode> typeDeclaration,
    map<string, vector<ASTNode>> &objects, ASTNode tree) {
  QList<std::shared_ptr<DeclarationNode>> blockList;
  //    std::shared_ptr<DeclarationNode> existingDecl =
  //    CodeValidator::findTypeDeclaration(typeDeclaration, ScopeStack(),
  //    m_tree);
  for (auto it = objects.begin(); it != objects.end(); it++) {
    // To avoid redundant checking here we should mark nodes that have already
    // been processed
    auto inheritedTypes = CodeValidator::getInheritedTypes(
        typeDeclaration, {{it->first, it->second}}, tree);

    for (auto inheritedType : inheritedTypes) {
      auto children = tree->getChildren();
      if (std::find(children.begin(), children.end(), inheritedType) !=
          children.end()) {
        // nothing
      } else {
        tree->addChild(inheritedType);
        auto findPos =
            std::find(it->second.begin(), it->second.end(), inheritedType);
        if (findPos != it->second.end()) {
          it->second.erase(findPos);
        }
      }
      insertDependentTypes(inheritedType, objects, tree);
    }
  }
}

void CodeResolver::insertBuiltinObjectsForNode(
    ASTNode node, map<string, vector<ASTNode>> &objects, ASTNode tree,
    std::string currentFramework) {
  QList<std::shared_ptr<DeclarationNode>> blockList;
  if (node->getNodeType() == AST::List) {
    for (ASTNode child : node->getChildren()) {
      insertBuiltinObjectsForNode(child, objects, tree);
    }
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *stream = static_cast<StreamNode *>(node.get());
    insertBuiltinObjectsForNode(stream->getLeft(), objects, tree);
    insertBuiltinObjectsForNode(stream->getRight(), objects, tree);
  } else if (node->getNodeType() == AST::Expression) {
    ExpressionNode *expr = static_cast<ExpressionNode *>(node.get());
    if (expr->isUnary()) {
      insertBuiltinObjectsForNode(expr->getValue(), objects, tree);
    } else {
      insertBuiltinObjectsForNode(expr->getLeft(), objects, tree);
      insertBuiltinObjectsForNode(expr->getRight(), objects, tree);
    }
  } else if (node->getNodeType() == AST::Function) {
    FunctionNode *func = static_cast<FunctionNode *>(node.get());
    for (auto it = objects.begin(); it != objects.end(); it++) {
      for (auto &objectTree : it->second) {
        //        auto existingDecl = CodeValidator::findDeclaration(
        //            QString::fromStdString(func->getName()), {}, tree,
        //            {it->first}, currentFramework);
        // for now, insert all declarations from all frameworks.
        // FIXME check outer domain and framework to only import needed modules

        std::vector<std::shared_ptr<DeclarationNode>> alldecls =
            CodeValidator::findAllDeclarations(
                func->getName(), {{it->first, objectTree->getChildren()}},
                nullptr, func->getNamespaceList());
        for (auto decl : alldecls) {
          if (!blockList.contains(decl)) {
            blockList << decl;
          }
        }
      }
    }
    // Look for declarations of blocks present in function properties
    for (auto property : func->getProperties()) {
      insertBuiltinObjectsForNode(property->getValue(), objects, tree);
    }
    for (std::shared_ptr<DeclarationNode> usedBlock : blockList) {
      // Add declarations to tree if not there
      auto usedBlockFramework = usedBlock->getCompilerProperty("framework");
      std::string fw;

      if (usedBlockFramework &&
          usedBlockFramework->getNodeType() == AST::String) {
        fw = static_pointer_cast<ValueNode>(usedBlockFramework)
                 ->getStringValue();
      }
      if (!CodeValidator::findDeclaration(usedBlock->getName(), ScopeStack(),
                                          tree, usedBlock->getNamespaceList(),
                                          fw)) {
        tree->addChild(usedBlock);
        //        for (auto it = objects.begin(); it != objects.end(); it++) {
        //          vector<ASTNode> &namespaceObjects = it->second;
        //          auto position = std::find(namespaceObjects.begin(),
        //                                    namespaceObjects.end(),
        //                                    usedBlock);
        //          if (position != namespaceObjects.end()) {
        //            namespaceObjects.erase(position);
        //            break;
        //          }
        //        }
      }
      for (std::shared_ptr<PropertyNode> property :
           usedBlock->getProperties()) {
        insertBuiltinObjectsForNode(property->getValue(), objects, tree, fw);
      }
      insertBuiltinObjectsForNode(usedBlock, objects, tree, fw);
    }
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> declaration =
        static_pointer_cast<DeclarationNode>(node);
    //    vector<ASTNode> namespaceObjects;
    if (declaration->getScopeLevels() > 0) {
      //      namespaceObjects = objects[declaration->getScopeAt(
      //          0)]; // TODO need to implement for namespaces that are more
      //          than one
      // level deep...
    }
    //    auto typeDecl = CodeValidator::findTypeDeclarationByName(
    //        declaration->getObjectType(), {}, tree);
    //    if (!typeDecl) {

    auto frameworkNode = node->getCompilerProperty("framework");
    std::string framework;
    if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
      framework =
          static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    for (auto it = objects.begin(); it != objects.end(); it++) {
      auto existingTypeDecl = CodeValidator::findTypeDeclarationByName(
          declaration->getObjectType(), {}, tree, {it->first}, framework);
      for (auto objectTree : it->second) {
        // FIXME get namespaces for declaration, not objects to insert
        auto typeDecl = CodeValidator::findTypeDeclarationByName(
            declaration->getObjectType(),
            {{it->first, objectTree->getChildren()}}, nullptr, {}, framework);
        if (!typeDecl) {  // try root namespace
          typeDecl = CodeValidator::findTypeDeclarationByName(
              declaration->getObjectType(),
              {{it->first, objectTree->getChildren()}}, nullptr, {});
        }
        if (typeDecl && !existingTypeDecl) {
          tree->addChild(typeDecl);
          // FIXME instead of removing we must make sure that objects are not
          // inserted in tree if already there
          std::vector<ASTNode> children = objectTree->getChildren();
          auto position = std::find(children.begin(), children.end(), typeDecl);
          if (position != children.end()) {
            children.erase(position);
            objectTree->setChildren(children);
          }
          insertBuiltinObjectsForNode(typeDecl, objects, tree);
          insertDependentTypes(typeDecl, objects, tree);
        }
      }
    }
    if (declaration /*&& !CodeValidator::findDeclaration(
                  QString::fromStdString(name->getName()), {},
                  tree, name->getNamespaceList())*/) {
      //                declaration->setRootScope(it->first);
      if (!blockList.contains(declaration)) {
        blockList << declaration;
        // Insert needed objects for things in module properties
        for (std::shared_ptr<PropertyNode> property :
             declaration->getProperties()) {
          insertBuiltinObjectsForNode(property->getValue(), objects, tree,
                                      framework);
        }
        // Process index for bundle declarations
        if (node->getNodeType() == AST::BundleDeclaration) {
          insertBuiltinObjectsForNode(declaration->getBundle()->index(),
                                      objects, tree, framework);
        }
      }
    }
    //  }
    //    }
  } else if (node->getNodeType() == AST::Block) {
    //    QList<std::shared_ptr<DeclarationNode>> blockList;
    BlockNode *name = static_cast<BlockNode *>(node.get());

    for (auto it = objects.begin(); it != objects.end(); it++) {
      for (auto objectTree : it->second) {
        auto declaration = CodeValidator::findDeclaration(
            name->getName(), {{it->first, objectTree->getChildren()}}, tree,
            name->getNamespaceList(), currentFramework);
        if (!declaration) {
          declaration = CodeValidator::findDeclaration(
              name->getName(), {{it->first, objectTree->getChildren()}}, tree,
              name->getNamespaceList());
        }
        if (declaration) {
          if (!blockList.contains(declaration)) {
            blockList << declaration;
          }
        }
      }
    }
    for (auto usedBlock : blockList) {
      auto frameworkNode = usedBlock->getCompilerProperty("framework");
      std::string framework;
      if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
        framework =
            static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
      }
      auto namespaceList = usedBlock->getNamespaceList();
      if (namespaceList.size() > 0 && namespaceList[0] == framework) {
        namespaceList.erase(namespaceList.begin());
      }
      auto existingDeclaration = CodeValidator::findDeclaration(
          usedBlock->getName(), {}, tree, namespaceList, framework);
      if (!existingDeclaration) {
        tree->addChild(usedBlock);
        insertBuiltinObjectsForNode(usedBlock, objects, tree);
      }
    }
  } else if (node->getNodeType() == AST::Bundle) {
    QList<std::shared_ptr<DeclarationNode>> blockList;
    auto bundle = static_pointer_cast<BundleNode>(node);
    for (auto it = objects.begin(); it != objects.end(); it++) {
      for (auto objectTree : it->second) {
        // FIXME This will get all declarations from all frameworks and it's
        // likely they won't all be needed
        auto declaration = CodeValidator::findDeclaration(
            bundle->getName(), {{it->first, objectTree->getChildren()}}, tree,
            bundle->getNamespaceList(), currentFramework);
        if (!declaration) {
          declaration = CodeValidator::findDeclaration(
              bundle->getName(), {{it->first, objectTree->getChildren()}}, tree,
              bundle->getNamespaceList());
        }
        if (declaration) {
          if (!blockList.contains(declaration)) {
            blockList << declaration;
          }
        }
      }
    }
    for (auto usedBlock : blockList) {
      auto frameworkNode = usedBlock->getCompilerProperty("framework");
      std::string framework;
      if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
        framework =
            static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
      }
      auto existingDeclaration = CodeValidator::findDeclaration(
          usedBlock->getName(), {}, tree, usedBlock->getNamespaceList(),
          framework);
      if (!existingDeclaration) {
        tree->addChild(usedBlock);
        insertBuiltinObjectsForNode(usedBlock, objects, tree);
      }
    }
  } else if (node->getNodeType() == AST::Property) {
    std::shared_ptr<PropertyNode> prop =
        std::static_pointer_cast<PropertyNode>(node);
    insertBuiltinObjectsForNode(prop->getValue(), objects, tree);
  }
}

void CodeResolver::resolveDomainsForStream(std::shared_ptr<StreamNode> stream,
                                           ScopeStack scopeStack,
                                           ASTNode contextDomainNode) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();
  QList<ASTNode> domainStack;
  ASTNode previousDomain =
      CodeValidator::getNodeDomain(left, scopeStack, m_tree);
  std::string previousDomainId =
      CodeValidator::getDomainIdentifier(previousDomain, scopeStack, m_tree);

  //    std::string contextDomain =
  //    CodeValidator::getDomainIdentifier(contextDomainNode, scopeStack,
  //    m_tree);

  if (!previousDomain || previousDomain->getNodeType() == AST::None) {
    previousDomain = contextDomainNode;
  }
  ASTNode domainNode;
  while (right) {
    domainNode = processDomainsForNode(left, scopeStack, domainStack);
    std::string domainId;
    if (domainNode) {
      domainId =
          CodeValidator::getDomainIdentifier(domainNode, scopeStack, m_tree);
    }
    if (left == right &&
        (!domainNode ||
         domainNode->getNodeType() ==
             AST::None)) {  // If this is is the last pass then set the unknown
                            // domains to context domain
      if (left->getNodeType() == AST::Function &&
          (!domainNode || domainNode->getNodeType() == AST::None)) {
        // Have functions/loops/reactions be assigned to whatever they are
        // connected to if they are final in a stream.
        // FIXME we should look inside the modules themselves to see if we can
        // infer the domain better
        if (previousDomain && previousDomain->getNodeType() != AST::None) {
          domainNode = previousDomain;
        } else {
          previousDomain = contextDomainNode;
        }
      } else if ((left->getNodeType() == AST::Block ||
                  left->getNodeType() == AST::Bundle) &&
                 (!domainNode || domainNode->getNodeType() == AST::None)) {
        if (previousDomain && previousDomain->getNodeType() != AST::None) {
          //              domainNode = previousDomain;
        } else {
          previousDomain = contextDomainNode;
        }
      } else {
        // FIXME This is needed in cases where signals with no set domain are
        // connected to input ports on other objects. Domains are not
        // propagated across secondary ports, but they should be

        previousDomain = contextDomainNode;
      }
      // Unneeded as it gets added in the above call to processDomainsForNode
      //            domainStack << left;
    }

    if (domainNode && domainNode->getNodeType() != AST::None &&
        previousDomainId != domainId) {
      auto resolvingInstance =
          CodeValidator::getInstance(left, scopeStack, m_tree);
      setDomainForStack(domainStack, resolvingInstance, domainNode, scopeStack);
      domainStack.clear();
    }
    if (left->getNodeType() == AST::Expression ||
        left->getNodeType() == AST::List) {
      auto samplingDomain =
          CodeValidator::getNodeDomain(stream->getRight(), scopeStack, m_tree);
      if (samplingDomain) {
        function<void(ASTNode node, ASTNode samplingDomain)> func =
            [&](ASTNode node, ASTNode samplingDomain) {
              for (auto child : node->getChildren()) {
                if (child->getNodeType() == AST::Expression ||
                    child->getNodeType() == AST::List) {
                  child->setCompilerProperty("samplingDomain", samplingDomain);
                  func(child, samplingDomain);
                } else {
                  if (child->getNodeType() == AST::Block ||
                      child->getNodeType() == AST::Bundle ||
                      child->getNodeType() == AST::PortProperty) {
                    // Check if expression element has no domain set. If it
                    // doesn't assign to samplingDomain
                    // TODO this does not cover the case where sampling domain
                    // is not resolvable at this point.
                    auto domain =
                        CodeValidator::getNodeDomain(child, scopeStack, m_tree);
                    if (!domain || domain->getNodeType() == AST::None) {
                      auto instance =
                          CodeValidator::getInstance(child, scopeStack, m_tree);
                      if (instance) {
                        if (instance->getNodeType() == AST::Declaration ||
                            instance->getNodeType() == AST::BundleDeclaration) {
                          auto decl =
                              static_pointer_cast<DeclarationNode>(instance);
                          decl->setPropertyValue("domain", samplingDomain);
                        }
                      }
                    }
                  } else if (child->getNodeType() == AST::Function) {
                    // TODO complete
                  }
                }
              }
            };
        left->setCompilerProperty("samplingDomain", samplingDomain);
        func(left, samplingDomain);
      }
    } else if (left->getNodeType() == AST::Function) {
      auto func = static_pointer_cast<FunctionNode>(left);

      auto decl = CodeValidator::findDeclaration(
          CodeValidator::streamMemberName(left), scopeStack, m_tree);

      if (decl) {
        if (decl->getObjectType() == "module" ||
            decl->getObjectType() == "reaction" ||
            decl->getObjectType() == "loop") {
          //

          domainNode = processDomainsForNode(decl, scopeStack, domainStack);
          auto internalStreams = decl->getPropertyValue("streams");
          auto internalBlocks = decl->getPropertyValue("blocks");
          auto internalScopeStack = scopeStack;

          if (internalStreams && internalBlocks) {
            internalScopeStack.push_back(
                {decl->getName(), internalBlocks->getChildren()});
            for (auto internalStream : internalStreams->getChildren()) {
              Q_ASSERT(internalStream->getNodeType() == AST::Stream);
              if (internalStream->getNodeType() == AST::Stream) {
                // No context domain as all domains within modules must be
                // resolvable internally
                resolveDomainsForStream(
                    std::static_pointer_cast<StreamNode>(internalStream),
                    internalScopeStack, nullptr);
              }
            }
          }
        }
      }
      // -- Process domains for connected ports
      ASTNode samplingDomainName;
      for (auto property : func->getProperties()) {
        samplingDomainName = processDomainsForNode(
            std::static_pointer_cast<StreamNode>(property->getValue()),
            scopeStack, domainStack);
      }
    }

    if (left == right) {
      if (previousDomain && previousDomain->getNodeType() != AST::None) {
        auto resolvingInstance =
            CodeValidator::getInstance(left, scopeStack, m_tree);
        setDomainForStack(domainStack, resolvingInstance, previousDomain,
                          scopeStack);
        domainStack.clear();
      }
      right = left = nullptr;  // End
    } else if (right->getNodeType() == AST::Stream) {
      stream = static_pointer_cast<StreamNode>(right);
      left = stream->getLeft();
      right = stream->getRight();
    } else {
      if (left->getNodeType() == AST::Expression ||
          left->getNodeType() == AST::List) {
        //        auto domainNode =
        //            CodeValidator::getNodeDomain(right, scopeStack, m_tree);
        //        if (!domainNode) {
        //          domainNode = std::make_shared<ValueNode>(__FILE__,
        //          __LINE__);
        //        }
        //        left->setCompilerProperty("samplingDomain", domainNode);
      }
      left = right;  // Last pass (process right, call it left)
    }
    previousDomain = domainNode;
    previousDomainId = domainId;
  }
}

ASTNode CodeResolver::processDomainsForNode(ASTNode node, ScopeStack scopeStack,
                                            QList<ASTNode> &domainStack) {
  ASTNode currentDomain;
  resolveDomainForStreamNode(node, scopeStack);
  if (node->getNodeType() == AST::Block || node->getNodeType() == AST::Bundle) {
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(CodeValidator::streamMemberName(node),
                                       scopeStack, m_tree,
                                       node->getNamespaceList());

    if (declaration) {
      ASTNode domain = declaration->getDomain();
      auto typeDeclaration =
          CodeValidator::findTypeDeclaration(declaration, scopeStack, m_tree);
      if (typeDeclaration &&
          typeDeclaration->getObjectType() == "platformModule") {
        // The instance for a platform module is the block itself, not the
        // declaration
        domainStack << node;
      } else {
        if (!domain || domain->getNodeType() == AST::None) {
          // Put declaration in stack to set domain once domain is resolved
          domainStack << declaration;
        } else if (domain->getNodeType() == AST::String ||
                   domain->getNodeType() == AST::Block ||
                   domain->getNodeType() == AST::PortProperty) {
          currentDomain = domain->deepCopy();
          currentDomain->setNamespaceList(node->getNamespaceList());
          //            currentDomain->addScope()
        } else {
          qDebug() << "unrecognized domain type";
        }
      }
      if (node->getNodeType() == AST::Bundle) {
        auto indexNode = static_pointer_cast<BundleNode>(node)->index();
        for (auto indexElement : indexNode->getChildren()) {
          resolveDomainForStreamNode(indexElement, scopeStack);
          QList<ASTNode> indexDomainStack;
          processDomainsForNode(indexElement, scopeStack, indexDomainStack);
          if (indexElement->getNodeType() == AST::String ||
              indexElement->getNodeType() == AST::Int ||
              indexElement->getNodeType() == AST::Real) {
            // No need to do resolve domain.
          } else if (indexElement->getNodeType() != AST::List &&
                     indexElement->getNodeType() != AST::Expression &&
                     indexElement->getNodeType() != AST::Range) {
            ASTNode domain =
                CodeValidator::getNodeDomain(indexElement, scopeStack, m_tree);
            if (domain && domain->getNodeType() != AST::None) {
              CodeValidator::setDomainForNode(indexElement, domain, scopeStack,
                                              m_tree);
            } else {
              qDebug() << "WARNING: domain has not resolved for bundle when "
                          "resolving index";
            }

          } else {
            // FIXME implement
            assert(0 == 1);
          }
        }
      }
      // Check if declared in the current scope. If declared here then store
      // parent list

      if (scopeStack.size() > 0) {
        std::shared_ptr<DeclarationNode> scopeDeclaration =
            CodeValidator::findDeclaration(
                CodeValidator::streamMemberName(node), {scopeStack.back()},
                nullptr);
        if (scopeDeclaration) {
          std::shared_ptr<ListNode> parentList =
              std::make_shared<ListNode>(__FILE__, __LINE__);
          if (declaration->getCompilerProperty("parentInstances") == nullptr) {
            for (auto subScope : scopeStack) {
              parentList->addChild(std::make_shared<ValueNode>(
                  subScope.first, __FILE__, __LINE__));
            }
            declaration->setCompilerProperty("parentInstances", parentList);
          }
        }
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    auto func = static_pointer_cast<FunctionNode>(node);
    ASTNode domain = CodeValidator::getNodeDomain(node, scopeStack, m_tree);
    if (!domain) {
      // Put declaration in stack to set domain once domain is resolved
      domainStack << node;
    } else {
      if (domain->getNodeType() == AST::String ||
          domain->getNodeType() == AST::Block ||
          domain->getNodeType() == AST::PortProperty) {
        currentDomain = domain;
      } else {
        domainStack << node;
      }
    }
    for (auto member : func->getProperties()) {
      ASTNode propertyDomainName =
          processDomainsForNode(member->getValue(), scopeStack, domainStack);
      if (!propertyDomainName ||
          propertyDomainName->getNodeType() ==
              AST::None) {  // Include property block in domain stack because
                            // domain undefined
        domainStack << member->getValue();
      }
    }
  } else if (node->getNodeType() == AST::List) {
    QList<ASTNode> listDomainStack;
    ASTNode resolvingInstance;
    for (ASTNode member : node->getChildren()) {
      ASTNode memberDomainName =
          processDomainsForNode(member, scopeStack, domainStack);

      if (!currentDomain || currentDomain->getNodeType() == AST::None) {
        // list takes domain from first element that has a domain
        if (memberDomainName &&
            (memberDomainName->getNodeType() == AST::String ||
             memberDomainName->getNodeType() == AST::Block ||
             memberDomainName->getNodeType() == AST::PortProperty)) {
          currentDomain = memberDomainName;
          resolvingInstance = member;
        } else {
          domainStack << node;
        }
      }
      if (!memberDomainName || memberDomainName->getNodeType() == AST::None) {
        // Put declaration in stack to set domain once domain is resolved

        listDomainStack << member;
        // FIMXE: This is very simplistic (or plain wrong....)
        // It assumes that the next found domain affects all elements
        // in the list that don't have domains. This is likely a
        // common case but list elements should inherit domains from
        // the port to which they are connected.
      }
    }
    if (currentDomain && currentDomain->getNodeType() != AST::None) {
      setDomainForStack(listDomainStack, resolvingInstance, currentDomain,
                        scopeStack);
    } else {
      domainStack << listDomainStack;  // If list has no defined domain, pass
                                       // all elements to be set later
    }
  } else if (node->getNodeType() == AST::Expression) {
    ASTNode samplingDomain;
    for (ASTNode member : node->getChildren()) {
      ASTNode newDomainName =
          processDomainsForNode(member, scopeStack, domainStack);
      // if member doesn't have a domain, put in the stack to resolve later.
      if (!newDomainName || newDomainName->getNodeType() == AST::None) {
        // Put declaration in stack to set domain once domain is resolved
        domainStack << member;
      } else {
        samplingDomain = newDomainName;
        currentDomain = newDomainName;
        // Take first domain available
        break;
      }
    }
    if (samplingDomain) {
      node->setCompilerProperty("samplingDomain", samplingDomain);
    } else {
    }
    if (scopeStack.size() > 0) {
      if (node->getCompilerProperty("parentInstances") == nullptr) {
        std::shared_ptr<ListNode> parentList =
            std::make_shared<ListNode>(__FILE__, __LINE__);
        for (auto subScope : scopeStack) {
          parentList->addChild(
              std::make_shared<ValueNode>(subScope.first, __FILE__, __LINE__));
        }
        node->setCompilerProperty("parentInstances", parentList);
      }
    }

  } else if (node->getNodeType() == AST::Int ||
             node->getNodeType() == AST::Real ||
             node->getNodeType() == AST::String ||
             node->getNodeType() == AST::Switch) {
    domainStack << node;
  } else if (node->getNodeType() == AST::Declaration) {
    // An anonymous reaction declaration.
    domainStack << node;
  }
  return currentDomain;
}

void CodeResolver::setDomainForStack(QList<ASTNode> domainStack,
                                     ASTNode resolvingInstance,
                                     ASTNode domainName,
                                     ScopeStack scopeStack) {
  for (ASTNode relatedNode : domainStack) {
    if (resolvingInstance->getNodeType() == AST::Declaration &&
        static_pointer_cast<DeclarationNode>(resolvingInstance)
                ->getObjectType() == "table") {
      // Table captures this stream
      relatedNode->setCompilerProperty("tableCaptureInstance",
                                       resolvingInstance);
    }

    CodeValidator::setDomainForNode(relatedNode, domainName, scopeStack,
                                    m_tree);
  }
}

std::shared_ptr<DeclarationNode> CodeResolver::createDomainDeclaration(
    QString name) {
  std::shared_ptr<DeclarationNode> newBlock = nullptr;
  newBlock = std::make_shared<DeclarationNode>(
      name.toStdString(), "_domainDefinition", nullptr, __FILE__, __LINE__);
  newBlock->addProperty(std::make_shared<PropertyNode>(
      "domainName",
      std::make_shared<ValueNode>(name.toStdString(), __FILE__, __LINE__),
      __FILE__, __LINE__));
  fillDefaultPropertiesForNode(newBlock, m_tree);
  return newBlock;
}

std::shared_ptr<DeclarationNode> CodeResolver::createSignalDeclaration(
    QString name, int size, ScopeStack scope, ASTNode tree) {
  std::shared_ptr<DeclarationNode> newBlock = nullptr;
  if (size == 0) {  // Is it OK to generate a signal of size 1 in this case??
    newBlock = std::make_shared<DeclarationNode>(name.toStdString(), "signal",
                                                 nullptr, __FILE__, __LINE__);
  } else if (size == 1) {
    newBlock = std::make_shared<DeclarationNode>(name.toStdString(), "signal",
                                                 nullptr, __FILE__, __LINE__);
  } else if (size > 1) {
    std::shared_ptr<ListNode> indexList = std::make_shared<ListNode>(
        std::make_shared<ValueNode>(size, "", -1), __FILE__, __LINE__);
    std::shared_ptr<BundleNode> bundle =
        std::make_shared<BundleNode>(name.toStdString(), indexList, "", -1);
    newBlock =
        std::make_shared<DeclarationNode>(bundle, "signal", nullptr, "", -1);
  }
  Q_ASSERT(newBlock);
  fillDefaultPropertiesForNode(newBlock, tree);

  resolveConstantsInNode(newBlock, scope, tree);
  return newBlock;
}

std::vector<ASTNode> CodeResolver::declareUnknownName(
    std::shared_ptr<BlockNode> block, int size, ScopeStack localScope,
    ASTNode tree) {
  std::vector<ASTNode> declarations;
  std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(
      QString::fromStdString(block->getName()), localScope, tree,
      block->getNamespaceList());
  if (!decl) {  // Not declared, so make declaration
    std::shared_ptr<DeclarationNode> newSignal = createSignalDeclaration(
        QString::fromStdString(block->getName()), size, localScope, m_tree);
    double rate = CodeValidator::getNodeRate(newSignal, ScopeStack(), tree);

    CodeValidator::setNodeRate(block, rate, localScope, tree);
    declarations.push_back(newSignal);
  }
  return declarations;
}

std::vector<ASTNode> CodeResolver::declareUnknownBundle(
    std::shared_ptr<BundleNode> bundle, int size, ScopeStack localScope,
    ASTNode tree) {
  std::vector<ASTNode> declarations;
  std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(
      QString::fromStdString(bundle->getName()), localScope, tree,
      bundle->getNamespaceList());
  if (!block) {  // Not declared, so make declaration
    std::shared_ptr<DeclarationNode> newSignal = createSignalDeclaration(
        QString::fromStdString(bundle->getName()), size, localScope, m_tree);
    double rate = CodeValidator::getNodeRate(newSignal, ScopeStack(), tree);
    CodeValidator::setNodeRate(bundle, rate, localScope, tree);
    declarations.push_back(newSignal);
  }
  return declarations;
}

std::shared_ptr<DeclarationNode> CodeResolver::createConstantDeclaration(
    string name, ASTNode value) {
  std::shared_ptr<DeclarationNode> constant = std::make_shared<DeclarationNode>(
      name, "constant", nullptr, __FILE__, __LINE__);
  std::shared_ptr<PropertyNode> valueProperty =
      std::make_shared<PropertyNode>("value", value, __FILE__, __LINE__);
  constant->addProperty(valueProperty);
  return constant;
}

void CodeResolver::declareIfMissing(string name, ASTNode blocks,
                                    ASTNode value) {
  std::shared_ptr<DeclarationNode> declaration = nullptr;
  if (blocks->getNodeType() == AST::List) {
    ListNode *blockList = static_cast<ListNode *>(blocks.get());
    // First check if block has been declared
    for (ASTNode block : blockList->getChildren()) {
      if (block->getNodeType() == AST::Declaration ||
          block->getNodeType() == AST::BundleDeclaration) {
        std::shared_ptr<DeclarationNode> declaredBlock =
            static_pointer_cast<DeclarationNode>(block);
        if (declaredBlock->getName() == name) {
          declaration = declaredBlock;
          break;
        }
      }
    }
    if (!declaration) {
      declaration = createConstantDeclaration(name, value);
      fillDefaultPropertiesForNode(declaration, m_tree);
      blockList->addChild(declaration);
    } else {
      //            delete value;
    }
  } else {
    //        delete value;
    qDebug() << "CodeResolver::declareIfMissing() blocks is not list";
  }
}

std::vector<ASTNode> CodeResolver::declareUnknownExpressionSymbols(
    std::shared_ptr<ExpressionNode> expr, int size, ScopeStack scopeStack,
    ASTNode tree) {
  std::vector<ASTNode> newDeclarations;
  if (expr->isUnary()) {
    if (expr->getValue()->getNodeType() == AST::Block) {
      std::shared_ptr<BlockNode> name =
          static_pointer_cast<BlockNode>(expr->getValue());
      std::vector<ASTNode> decls =
          declareUnknownName(name, size, scopeStack, tree);
      newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
    } else if (expr->getValue()->getNodeType() == AST::Expression) {
      std::shared_ptr<ExpressionNode> name =
          static_pointer_cast<ExpressionNode>(expr->getValue());
      std::vector<ASTNode> decls =
          declareUnknownExpressionSymbols(name, size, scopeStack, tree);
      newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
    }
  } else {
    if (expr->getLeft()->getNodeType() == AST::Block) {
      std::shared_ptr<BlockNode> name =
          static_pointer_cast<BlockNode>(expr->getLeft());
      std::vector<ASTNode> decls =
          declareUnknownName(name, size, scopeStack, tree);
      newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
    } else if (expr->getLeft()->getNodeType() == AST::Expression) {
      std::shared_ptr<ExpressionNode> inner_expr =
          static_pointer_cast<ExpressionNode>(expr->getLeft());
      std::vector<ASTNode> decls =
          declareUnknownExpressionSymbols(inner_expr, size, scopeStack, tree);
      newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
    }
    if (expr->getRight()->getNodeType() == AST::Block) {
      std::shared_ptr<BlockNode> name =
          static_pointer_cast<BlockNode>(expr->getRight());
      std::vector<ASTNode> decls =
          declareUnknownName(name, size, scopeStack, tree);
      newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
    } else if (expr->getRight()->getNodeType() == AST::Expression) {
      std::shared_ptr<ExpressionNode> inner_expr =
          static_pointer_cast<ExpressionNode>(expr->getRight());
      std::vector<ASTNode> decls =
          declareUnknownExpressionSymbols(inner_expr, size, scopeStack, tree);
      newDeclarations.insert(newDeclarations.end(), decls.begin(), decls.end());
    }
  }
  return newDeclarations;
}

std::vector<ASTNode> CodeResolver::declareUnknownFunctionSymbols(
    std::shared_ptr<FunctionNode> func, ScopeStack scopeStack, ASTNode tree) {
  std::vector<ASTNode> newDeclarations;
  vector<std::shared_ptr<PropertyNode>> properties = func->getProperties();

  for (auto property : properties) {
    ASTNode value = property->getValue();
    if (value->getNodeType() == AST::Block) {
      std::shared_ptr<BlockNode> block = static_pointer_cast<BlockNode>(value);
      std::vector<ASTNode> declarations =
          declareUnknownName(block, 1, scopeStack, tree);
      for (ASTNode declaration : declarations) {
        tree->addChild(declaration);
      }
    } else if (value->getNodeType() == AST::Bundle) {
      // FIXME should we have some heuristics for bundles, or just not allow
      // auto delcarations?
    }
  }

  return newDeclarations;
}

std::shared_ptr<ListNode> CodeResolver::expandNameToList(BlockNode *name,
                                                         int size) {
  std::shared_ptr<ListNode> list = std::make_shared<ListNode>(
      nullptr, name->getFilename().data(), name->getLine());
  for (int i = 0; i < size; i++) {
    std::shared_ptr<ListNode> indexList = std::make_shared<ListNode>(
        std::make_shared<ValueNode>(i, name->getFilename().data(),
                                    name->getLine()),
        name->getFilename().data(), name->getLine());
    std::shared_ptr<BundleNode> bundle = std::make_shared<BundleNode>(
        name->getName(), indexList, name->getFilename().data(),
        name->getLine());
    list->addChild(bundle);
  }
  return list;
}

void CodeResolver::expandNamesToBundles(std::shared_ptr<StreamNode> stream,
                                        ASTNode tree) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();
  //    ASTNode nextStreamMember;
  //    if (right->getNodeType() != AST::Stream) {
  //        nextStreamMember = right;
  //    } else {
  //        nextStreamMember = static_cast<StreamNode *>(right)->getLeft();
  //    }

  if (left->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(left.get());
    ScopeStack scope;
    std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(
        QString::fromStdString(name->getName()), scope, tree);
    int size = 0;
    if (block) {
      if (block->getNodeType() == AST::BundleDeclaration) {
        QList<LangError> errors;
        size = CodeValidator::getBlockDeclaredSize(block, scope, tree, errors);
      } else if (block->getNodeType() == AST::Declaration) {
        size = 1;
      }
    }
    if (size > 1) {
      std::shared_ptr<ListNode> list = expandNameToList(name, size);
      stream->setLeft(list);
    }
  }
  if (right->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(right.get());
    ScopeStack scope;
    std::shared_ptr<DeclarationNode> block =
        CodeValidator::findDeclaration(QString::fromStdString(name->getName()),
                                       scope, tree, name->getNamespaceList());
    int size = 0;
    if (block) {
      if (block->getNodeType() == AST::BundleDeclaration) {
        QList<LangError> errors;
        size = CodeValidator::getBlockDeclaredSize(block, scope, tree, errors);
      } else if (block->getNodeType() == AST::Declaration) {
        size = 1;
      }
    }
    if (size > 1) {
      std::shared_ptr<ListNode> list = expandNameToList(name, size);
      stream->setRight(list);
    }
  } else if (right->getNodeType() == AST::Stream) {
    expandNamesToBundles(static_pointer_cast<StreamNode>(right), tree);
  }
}

std::vector<ASTNode> CodeResolver::declareUnknownStreamSymbols(
    std::shared_ptr<StreamNode> stream, ASTNode previousStreamMember,
    ScopeStack localScope, ASTNode tree) {
  std::vector<ASTNode> newDeclarations;
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();

  ASTNode nextStreamMember;
  if (right->getNodeType() != AST::Stream) {
    nextStreamMember = right;
  } else {
    nextStreamMember = static_pointer_cast<StreamNode>(right)->getLeft();
  }

  if (left->getNodeType() == AST::Block) {
    std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(left);
    QList<LangError> errors;
    int size = -1;
    if (previousStreamMember) {
      size = CodeValidator::getNodeNumOutputs(previousStreamMember, localScope,
                                              m_tree, errors);
    }
    if (size <= 0 && previousStreamMember) {  // Look to the right if can't
                                              // resolve from the left
      // Size for first member should always be 1
      size = CodeValidator::getNodeNumInputs(nextStreamMember, localScope,
                                             m_tree, errors);
    }
    if (size <= 0) {  // None of the elements in the stream have size
      size = 1;
    }
    std::vector<ASTNode> declarations =
        declareUnknownName(name, size, localScope, tree);
    newDeclarations.insert(newDeclarations.end(), declarations.begin(),
                           declarations.end());
  } else if (left->getNodeType() == AST::Expression) {
    int size = 1;  // FIXME implement size detection for expressions
    std::shared_ptr<ExpressionNode> expr =
        static_pointer_cast<ExpressionNode>(left);
    std::vector<ASTNode> declarations =
        declareUnknownExpressionSymbols(expr, size, localScope, tree);
    newDeclarations.insert(newDeclarations.end(), declarations.begin(),
                           declarations.end());
  } else if (left->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        static_pointer_cast<FunctionNode>(left);
    std::vector<ASTNode> declarations =
        declareUnknownFunctionSymbols(func, localScope, tree);
    newDeclarations.insert(newDeclarations.end(), declarations.begin(),
                           declarations.end());
  }

  if (right->getNodeType() == AST::Stream) {
    std::vector<ASTNode> declarations = declareUnknownStreamSymbols(
        static_pointer_cast<StreamNode>(right), left, localScope, tree);
    newDeclarations.insert(newDeclarations.end(), declarations.begin(),
                           declarations.end());
  } else if (right->getNodeType() == AST::Block) {
    std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(right);
    QList<LangError> errors;
    int size = 0;
    if (left->getNodeType() == AST::Function) {
      auto funcDecl = CodeValidator::findDeclaration(
          CodeValidator::streamMemberName(left), localScope, tree);
      if (funcDecl) {
        size = CodeValidator::getTypeNumOutputs(funcDecl, localScope, m_tree,
                                                errors);
      }
    } else {
      size = CodeValidator::getNodeNumOutputs(left, localScope, m_tree, errors);
    }
    if (size <= 0) {  // None of the elements in the stream have size
      size = 1;
    }
    std::vector<ASTNode> declarations =
        declareUnknownName(name, size, localScope, tree);
    newDeclarations.insert(newDeclarations.end(), declarations.begin(),
                           declarations.end());
  } else if (right->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        static_pointer_cast<FunctionNode>(right);
    std::vector<ASTNode> declarations =
        declareUnknownFunctionSymbols(func, localScope, tree);
    newDeclarations.insert(newDeclarations.end(), declarations.begin(),
                           declarations.end());
  }
  return newDeclarations;
}

std::vector<ASTNode> CodeResolver::getModuleStreams(
    std::shared_ptr<DeclarationNode> module) {
  std::vector<ASTNode> streams;

  ASTNode streamsNode = module->getPropertyValue("streams");
  Q_ASSERT(streamsNode);
  if (streamsNode->getNodeType() == AST::Stream) {
    streams.push_back(streamsNode);
  } else if (streamsNode->getNodeType() == AST::List) {
    for (ASTNode node : streamsNode->getChildren()) {
      if (node->getNodeType() == AST::Stream) {
        streams.push_back(node);
      }
    }
  }
  return streams;
}

std::vector<ASTNode> CodeResolver::getModuleBlocks(
    std::shared_ptr<DeclarationNode> module) {
  std::vector<ASTNode> blocks;

  ASTNode blocksNode = module->getPropertyValue("blocks");
  Q_ASSERT(blocksNode);
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
  return blocks;
}

void CodeResolver::declareInternalBlocksForNode(ASTNode node,
                                                ScopeStack subScope) {
  //  {{decl->getName(), internalBlocks->getChildren()}}
  auto findDeclarationWithinFunctionScope = [&](ASTNode portBlock,
                                                ScopeStack innerScope) {
    auto portBlockDecl = CodeValidator::findDeclaration(
        CodeValidator::streamMemberName(portBlock), innerScope, nullptr);
    if (!portBlockDecl) {
      // Check for constants in parent nodes
      auto rootPortBlockDecl = CodeValidator::findDeclaration(
          CodeValidator::streamMemberName(portBlock), subScope, m_tree);
      if (rootPortBlockDecl &&
          rootPortBlockDecl->getObjectType() == "constant") {
        portBlockDecl = rootPortBlockDecl;
      }
    }
    return portBlockDecl;
  };

  if (node->getNodeType() == AST::Declaration) {
    std::shared_ptr<DeclarationNode> decl =
        static_pointer_cast<DeclarationNode>(node);
    if (decl->getObjectType() ==
        "loop") {  // We need to define an internal domain for loops
      ListNode *ports =
          static_cast<ListNode *>(decl->getPropertyValue("ports").get());
      if (!ports) {
        decl->setPropertyValue(
            "ports", std::make_shared<ValueNode>(
                         __FILE__,
                         __LINE__));  // Make a None node to trigger next branch
      }
      if (ports && ports->getNodeType() == AST::None) {
        decl->replacePropertyValue(
            "ports", std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
        ports = static_cast<ListNode *>(decl->getPropertyValue("ports").get());
      }
      ASTNode internalBlocks = decl->getPropertyValue("blocks");
      //            internalBlocks->addChild(createDomainDeclaration(QString::fromStdString("_OutputDomain")));
    }
    if (decl->getObjectType() == "module" ||
        decl->getObjectType() == "reaction" ||
        decl->getObjectType() == "loop") {
      // First insert and resolve input and output domains for main ports. The
      // input port takes the output domain if undefined.

      ASTNode internalBlocks = decl->getPropertyValue("blocks");
      if (!internalBlocks || internalBlocks->getNodeType() !=
                                 AST::List) {  // Turn blocks property into list
        decl->setPropertyValue(
            "blocks",
            std::make_shared<ListNode>(internalBlocks, __FILE__, __LINE__));
        internalBlocks = decl->getPropertyValue("blocks");
      }

      // Check Output port and declare block if not declared
      std::shared_ptr<DeclarationNode> outputPortBlock =
          CodeValidator::getMainOutputPortBlock(decl);
      ASTNode mainPortsDefaultDomain;
      if (outputPortBlock) {
        ASTNode portBlock = outputPortBlock->getPropertyValue("block");
        auto outputDomain = std::make_shared<PortPropertyNode>(
            outputPortBlock->getName(), "domain", __FILE__, __LINE__);
        mainPortsDefaultDomain = outputDomain;
        // First give port a block and its declaration if it doesn't have one.
        if (!portBlock || portBlock->getNodeType() == AST::None) {
          // FIXME  check if there is a block called Output already
          portBlock = std::make_shared<BlockNode>("Output", __FILE__, __LINE__);
          outputPortBlock->setPropertyValue("block", portBlock);
        }

        // Add declaration if not there
        // We need to find declaration only in the internal blocks, as port
        // blocks can only be internal.
        auto portBlockDecl = findDeclarationWithinFunctionScope(
            portBlock, {{decl->getName(), internalBlocks->getChildren()}});

        if (!portBlockDecl) {
          // FIXME this has to check wether block is a bundle and create
          // declaration accordingly
          portBlockDecl = createSignalDeclaration(
              QString::fromStdString(
                  CodeValidator::streamMemberName(portBlock)),
              1, subScope, m_tree);
          internalBlocks->addChild(portBlockDecl);
          fillDefaultPropertiesForNode(portBlockDecl, m_tree);
        }

        // Now check if domain set, if not, set to port domain
        ASTNode domainNode = CodeValidator::getNodeDomain(
            portBlock, {{decl->getName(), internalBlocks->getChildren()}},
            m_tree);
        if (!domainNode || domainNode->getNodeType() == AST::None) {
          if (portBlockDecl) {
            portBlockDecl->setPropertyValue("domain", mainPortsDefaultDomain);
          }
        }
      }
      // Check Input port and declare block if not declared, pass default
      // domain if available from output port or set default mode from input
      // port
      std::shared_ptr<DeclarationNode> inputPortBlock =
          CodeValidator::getMainInputPortBlock(decl);
      if (inputPortBlock) {
        ASTNode portBlock = inputPortBlock->getPropertyValue("block");
        auto inputDomain = std::make_shared<PortPropertyNode>(
            inputPortBlock->getName(), "domain", __FILE__, __LINE__);

        if (!mainPortsDefaultDomain) {  // If not set from output port, set
                                        // here
          mainPortsDefaultDomain = inputDomain;
        }
        // First give port a block and its declaration if it doesn't have one.
        if (!portBlock || portBlock->getNodeType() == AST::None) {
          // FIXME  check if there is a block called Output already
          portBlock = std::make_shared<BlockNode>("Input", __FILE__, __LINE__);
          inputPortBlock->setPropertyValue("block", portBlock);
        }

        // Add declaration if not there
        // Port blocks can only be internal, so look for declaration only in
        // internal blocks

        auto portBlockDecl = findDeclarationWithinFunctionScope(
            portBlock, {{decl->getName(), internalBlocks->getChildren()}});
        if (!portBlockDecl) {
          // FIXME this has to check wether block is a bundle and create
          // declaration accordingly
          portBlockDecl = createSignalDeclaration(
              QString::fromStdString(
                  CodeValidator::streamMemberName(portBlock)),
              1, subScope, m_tree);
          internalBlocks->addChild(portBlockDecl);
          fillDefaultPropertiesForNode(portBlockDecl, m_tree);
        }

        // Now check if domain set, if not, set to port domain
        ASTNode domainNode = CodeValidator::getNodeDomain(
            portBlock, {{decl->getName(), internalBlocks->getChildren()}},
            m_tree);
        if (!domainNode || domainNode->getNodeType() == AST::None) {
          if (portBlockDecl && portBlockDecl->getObjectType() != "constant") {
            portBlockDecl->setPropertyValue("domain", mainPortsDefaultDomain);
          }
        }
      }

      // Find OutputPort node or give a default Block and domain
      auto ports =
          static_pointer_cast<ListNode>(decl->getPropertyValue("ports"));

      internalBlocks = decl->getPropertyValue("blocks");
      // Then go through ports autodeclaring blocks
      if (ports->getNodeType() == AST::List) {
        for (ASTNode port : ports->getChildren()) {
          Q_ASSERT(port->getNodeType() == AST::Declaration);
          auto portDeclaration = static_pointer_cast<DeclarationNode>(port);

          // Properties that we need to auto-declare for
          ASTNode blockPortValue = portDeclaration->getPropertyValue("block");
          auto portDomain = portDeclaration->getPropertyValue("domain");
          if (!portDomain) {
            portDomain = std::make_shared<PortPropertyNode>(
                portDeclaration->getName(), "domain", __FILE__, __LINE__);
          }
          auto portRate = std::make_shared<PortPropertyNode>(
              portDeclaration->getName(), "rate", __FILE__, __LINE__);

          if (blockPortValue) {
            //                        // Now do auto declaration of IO blocks
            //                        if not declared.
            Q_ASSERT(blockPortValue->getNodeType() == AST::Block ||
                     blockPortValue->getNodeType() ==
                         AST::None);  // Catch on debug but fail gracefully on
                                      // release
            if (blockPortValue->getNodeType() == AST::Block) {
              std::shared_ptr<BlockNode> nameNode =
                  static_pointer_cast<BlockNode>(blockPortValue);
              string name = nameNode->getName();

              auto blockDecl = findDeclarationWithinFunctionScope(
                  nameNode, {{decl->getName(), internalBlocks->getChildren()}});
              if (!blockDecl) {  // If block is given but not
                                 // declared, declare
                                 // and assing port domain
                int size = 1;
                blockDecl = createSignalDeclaration(
                    QString::fromStdString(name), size, subScope, m_tree);
                blockDecl->replacePropertyValue(
                    "rate", std::make_shared<ValueNode>(__FILE__, __LINE__));
                internalBlocks->addChild(blockDecl);
                // TODO This default needs to be done per instance
                ASTNode portDefault =
                    portDeclaration->getPropertyValue("default");
                if (portDefault && portDefault->getNodeType() != AST::None) {
                  Q_ASSERT(blockDecl->getPropertyValue("default"));
                  blockDecl->replacePropertyValue("default", portDefault);
                }
              }
              // set its domain to the port domain if not set
              auto declType = CodeValidator::findTypeDeclaration(
                  blockDecl, subScope, m_tree);
              if (declType) {
                auto inheritList = CodeValidator::getInheritedTypes(
                    declType, subScope, m_tree);
                bool inheritsDomainType = false;
                for (auto inherits : inheritList) {
                  auto typeName = inherits->getPropertyValue("typeName");
                  if (typeName->getNodeType() == AST::String) {
                    if (static_pointer_cast<ValueNode>(typeName)
                            ->getStringValue() == "domainMember") {
                      inheritsDomainType = true;
                      break;
                    }
                  }
                }
                if (inheritsDomainType) {
                  ASTNode blockDomain = blockDecl->getDomain();
                  if (!blockDomain || blockDomain->getNodeType() == AST::None) {
                    blockDecl->setPropertyValue("domain", portDomain);
                  }
                  if (blockDecl->getPropertyValue("rate")->getNodeType() ==
                      AST::None) {
                    blockDecl->setPropertyValue("rate", portRate);
                  }
                }
              }
            } else if (blockPortValue->getNodeType() == AST::None) {
              // TODO Make sure block name not being used
              string defaultName = portDeclaration->getName() + "_Block";
              std::shared_ptr<BlockNode> name =
                  std::make_shared<BlockNode>(defaultName, __FILE__, __LINE__);
              portDeclaration->replacePropertyValue("block", name);
              std::shared_ptr<DeclarationNode> newSignal =
                  CodeValidator::findDeclaration(
                      QString::fromStdString(defaultName), ScopeStack(),
                      internalBlocks);
              if (!newSignal) {
                newSignal = createSignalDeclaration(
                    QString::fromStdString(defaultName), 1, subScope, m_tree);
                internalBlocks->addChild(newSignal);
                fillDefaultPropertiesForNode(newSignal, m_tree);
                ASTNode blockDomain = newSignal->getDomain();
                if (!blockDomain || blockDomain->getNodeType() == AST::None) {
                  newSignal->setPropertyValue("domain", portDomain);
                  if (newSignal->getPropertyValue("rate")->getNodeType() ==
                      AST::None) {
                    newSignal->setPropertyValue("rate", portRate);
                  }
                }
              }
            }
          }
        }
      } else if (ports->getNodeType() == AST::None) {
        // If port list is None, then ignore
      } else {
        qDebug() << "ERROR! ports property must be a list or None!";
      }

      // Go through child blocks and autodeclare for internal modules and
      // reactions
      //            ASTNode internalBlocks =
      //            block->getPropertyValue("blocks");
      for (ASTNode node : internalBlocks->getChildren()) {
        declareInternalBlocksForNode(node, subScope);
      }
    }
  }
}

void CodeResolver::resolveStreamSymbols() {
  // FIMXE we need to resolve the streams in the root tree in reverse order as
  // we do for streams within modules.
  for (ASTNode node : m_tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      std::shared_ptr<StreamNode> stream =
          static_pointer_cast<StreamNode>(node);
      std::vector<ASTNode> declarations = declareUnknownStreamSymbols(
          stream, nullptr, ScopeStack(),
          m_tree);  // FIXME Is this already done in expandParallelFunctions?
      for (ASTNode decl : declarations) {
        m_tree->addChild(decl);
      }
      expandNamesToBundles(stream, m_tree);
    } else if (node->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "module" ||
          decl->getObjectType() == "reaction" ||
          decl->getObjectType() == "loop") {
        std::vector<ASTNode> streams = getModuleStreams(decl);
        ScopeStack scopeStack;
        ASTNode blocks = decl->getPropertyValue("blocks");
        if (blocks && blocks->getNodeType() == AST::List) {
          scopeStack.push_back({decl->getName(), blocks->getChildren()});
        }
        auto rit = streams.rbegin();
        while (rit != streams.rend()) {
          const ASTNode streamNode = *rit;
          if (streamNode->getNodeType() == AST::Stream) {
            std::shared_ptr<StreamNode> stream =
                static_pointer_cast<StreamNode>(streamNode);
            std::vector<ASTNode> declarations = declareUnknownStreamSymbols(
                stream, nullptr, scopeStack, m_tree);
            std::shared_ptr<ListNode> blockList =
                static_pointer_cast<ListNode>(decl->getPropertyValue("blocks"));
            Q_ASSERT(blockList && blockList->getNodeType() == AST::List);
            for (ASTNode newDecl : declarations) {
              blockList->addChild(newDecl);
              scopeStack.push_back({decl->getName(), {newDecl}});
            }
          }
          rit++;
        }
      }
    }
  }
}

void CodeResolver::resolveConstants() {
  for (auto override = m_systemConfig.overrides["all"].cbegin();
       override != m_systemConfig.overrides["all"].cend(); ++override) {
    //        qDebug() << override.key();
    std::shared_ptr<DeclarationNode> decl =
        CodeValidator::findDeclaration(override->first, {}, m_tree);
    if (decl) {
      if (decl->getObjectType() == "constant") {
        if (override->second.type() == QVariant::String) {
          decl->replacePropertyValue(
              "value", std::make_shared<ValueNode>(
                           override->second.toString().toStdString(), __FILE__,
                           __LINE__));
        } else if (override->second.type() == QVariant::Int) {
          decl->replacePropertyValue(
              "value", std::make_shared<ValueNode>(override->second.toInt(),
                                                   __FILE__, __LINE__));
        }
      } else {
        qDebug() << "WARNING: Ignoring configuration override '" +
                        QString::fromStdString(override->first) +
                        "'. Not constant.";
      }
    } else {
      qDebug() << "WARNING: Configuration override not found: " +
                      QString::fromStdString(override->first);
    }
  }
  for (ASTNode node : m_tree->getChildren()) {
    resolveConstantsInNode(node, {}, m_tree);
  }
}

std::shared_ptr<ValueNode> CodeResolver::reduceConstExpression(
    std::shared_ptr<ExpressionNode> expr, ScopeStack scope, ASTNode tree) {
  ASTNode left = nullptr, right = nullptr;
  bool isConstant;

  if (!expr->isUnary()) {
    left = expr->getLeft();
  } else {
    left = expr->getValue();
  }

  std::shared_ptr<ValueNode> newValue = resolveConstant(left, scope, tree);
  if (newValue) {
    if (expr->isUnary()) {
      expr->replaceValue(newValue);
    } else {
      expr->replaceLeft(newValue);
    }
    left = newValue;
  }
  if (!expr->isUnary()) {
    right = expr->getRight();
    newValue = resolveConstant(right, scope, tree);
    if (newValue) {
      expr->replaceRight(newValue);
      right = newValue;
    }
    isConstant =
        (left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) &&
        (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real);
  } else {
    isConstant =
        (left->getNodeType() == AST::Int || left->getNodeType() == AST::Real);
  }

  if (isConstant) {
    std::shared_ptr<ValueNode> result = nullptr;
    switch (expr->getExpressionType()) {
      case ExpressionNode::Multiply:
        result = multiply(static_pointer_cast<ValueNode>(left),
                          static_pointer_cast<ValueNode>(right));
        break;
      case ExpressionNode::Divide:
        result = divide(static_pointer_cast<ValueNode>(left),
                        static_pointer_cast<ValueNode>(right));
        break;
      case ExpressionNode::Add:
        result = add(static_pointer_cast<ValueNode>(left),
                     static_pointer_cast<ValueNode>(right));
        break;
      case ExpressionNode::Subtract:
        result = subtract(static_pointer_cast<ValueNode>(left),
                          static_pointer_cast<ValueNode>(right));
        break;
      case ExpressionNode::And:
        result = logicalAnd(static_pointer_cast<ValueNode>(left),
                            static_pointer_cast<ValueNode>(right));
        break;
      case ExpressionNode::Or:
        result = logicalOr(static_pointer_cast<ValueNode>(left),
                           static_pointer_cast<ValueNode>(right));
        break;
      case ExpressionNode::UnaryMinus:
        result = unaryMinus(static_pointer_cast<ValueNode>(left));
        break;
      case ExpressionNode::LogicalNot:
        result = logicalNot(static_pointer_cast<ValueNode>(left));
        break;
      default:
        Q_ASSERT(0 == 1);  // Should never get here
        break;
    }
    if (result) {
      return result;
    }
  }
  return nullptr;
}

std::shared_ptr<ValueNode> CodeResolver::resolveConstant(
    ASTNode value, ScopeStack scope, ASTNode tree, std::string framework) {
  std::shared_ptr<ValueNode> newValue = nullptr;
  if (value->getNodeType() == AST::Expression) {
    std::shared_ptr<ExpressionNode> expr =
        static_pointer_cast<ExpressionNode>(value);
    newValue = reduceConstExpression(expr, scope, tree);
    return newValue;
  } else if (value->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(value.get());
    std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(
        name->getName(), scope, tree, name->getNamespaceList(), framework);
    if (block && block->getNodeType() == AST::Declaration &&
        block->getObjectType() == "constant") {  // Size == 1
      //            string namespaceValue = name->getScopeAt(0);
      ASTNode declarationNamespace = block->getPropertyValue("namespace");
      //            if (namespaceValue.size() == 0 || namespaceValue)
      ASTNode blockValue = block->getPropertyValue("value");
      if (blockValue->getNodeType() == AST::Int ||
          blockValue->getNodeType() == AST::Real ||
          blockValue->getNodeType() == AST::String) {
        return static_pointer_cast<ValueNode>(blockValue);
      }
      newValue = resolveConstant(block->getPropertyValue("value"), scope, tree);
      return newValue;
    }
  } else if (value->getNodeType() == AST::Bundle) {
    // What does this mean??
  } else if (value->getNodeType() == AST::PortProperty) {
    PortPropertyNode *propertyNode =
        static_cast<PortPropertyNode *>(value.get());
    std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(
        QString::fromStdString(propertyNode->getPortName()), scope, tree);
    if (block) {
      ASTNode propertyValue = block->getPropertyValue(propertyNode->getName());
      if (propertyValue) {
        //                || propertyValue->getNodeType() == AST::Block ||
        //                propertyValue->getNodeType() == AST::Bundle
        if (propertyValue->getNodeType() == AST::Int ||
            propertyValue->getNodeType() == AST::Real ||
            propertyValue->getNodeType() == AST::String) {
          return static_pointer_cast<ValueNode>(propertyValue);
        }
      }
    }
  }
  return nullptr;
}

void CodeResolver::resolveConstantsInNode(ASTNode node, ScopeStack scope,
                                          ASTNode tree,
                                          std::string currentFramework) {
  if (node->getNodeType() == AST::Stream) {
    std::shared_ptr<StreamNode> stream = static_pointer_cast<StreamNode>(node);
    resolveConstantsInNode(stream->getLeft(), scope, tree);
    if (stream->getLeft()->getNodeType() == AST::Expression) {
      std::shared_ptr<ExpressionNode> expr =
          static_pointer_cast<ExpressionNode>(stream->getLeft());
      std::shared_ptr<ValueNode> newValue =
          reduceConstExpression(expr, scope, tree);
      if (newValue) {
        stream->setLeft(newValue);
      }
    } else if (stream->getLeft()->getNodeType() == AST::PortProperty) {
      std::shared_ptr<PortPropertyNode> propertyNode =
          static_pointer_cast<PortPropertyNode>(stream->getLeft());
      std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(
          QString::fromStdString(propertyNode->getPortName()), scope, tree);
      if (block) {
        ASTNode property = block->getPropertyValue(propertyNode->getName());
        if (property) {  // First replace if pointing to a name
          if (property->getNodeType() == AST::Block ||
              property->getNodeType() == AST::Bundle) {
            stream->setLeft(property);
          }
          std::shared_ptr<ValueNode> newValue =
              resolveConstant(stream->getLeft(), scope, tree);
          if (newValue) {
            stream->setLeft(newValue);
          }
        }
      }
    }
    resolveConstantsInNode(stream->getRight(), scope, tree);
  } else if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        static_pointer_cast<FunctionNode>(node);
    vector<std::shared_ptr<PropertyNode>> properties = func->getProperties();
    for (auto property : properties) {
      std::shared_ptr<ValueNode> newValue =
          resolveConstant(property->getValue(), scope, tree);
      if (newValue) {
        property->replaceValue(newValue);
      }
    }
  } else if (node->getNodeType() == AST::Declaration) {
    std::shared_ptr<DeclarationNode> decl =
        static_pointer_cast<DeclarationNode>(node);
    vector<std::shared_ptr<PropertyNode>> properties = decl->getProperties();
    std::shared_ptr<ListNode> internalBlocks =
        static_pointer_cast<ListNode>(decl->getPropertyValue("blocks"));
    if ((decl->getObjectType() == "module" ||
         decl->getObjectType() == "reaction" ||
         decl->getObjectType() == "loop") &&
        internalBlocks) {
      if (internalBlocks->getNodeType() == AST::List) {
        auto blocks = internalBlocks->getChildren();
        scope.push_back({decl->getName(), blocks});
      }
    }
    // FIXME This is a hack to protect constants that are context dependent.
    // Should the language mark this?
    //    if (decl->getObjectType() != "type") {

    auto frameworkNode = node->getCompilerProperty("framework");
    std::string frameworkName;
    if (frameworkNode) {
      frameworkName =
          static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    if (frameworkName == "") {
      // Now try to see if domain belongs to a framework

      auto domainId =
          CodeValidator::getNodeDomainName(node, ScopeStack(), tree);

      frameworkName = CodeValidator::getFrameworkForDomain(domainId, tree);
    }

    for (std::shared_ptr<PropertyNode> property : properties) {
      resolveConstantsInNode(property->getValue(), scope, tree, frameworkName);
      std::shared_ptr<ValueNode> newValue =
          resolveConstant(property->getValue(), scope, tree, frameworkName);
      if (!newValue) {
        // try to resolve on global namespace
        newValue = resolveConstant(property->getValue(), scope, tree);
      }
      if (newValue) {
        property->replaceValue(newValue);
      }
    }
    //    }
  } else if (node->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> decl =
        static_pointer_cast<DeclarationNode>(node);
    vector<std::shared_ptr<PropertyNode>> properties = decl->getProperties();
    std::shared_ptr<ListNode> internalBlocks =
        static_pointer_cast<ListNode>(decl->getPropertyValue("blocks"));
    if (internalBlocks) {
      if (internalBlocks->getNodeType() == AST::List) {
        auto blocks = internalBlocks->getChildren();
        scope.push_back({decl->getName(), blocks});
      }
    }

    auto frameworkNode = node->getCompilerProperty("framework");
    std::string frameworkName;
    if (frameworkNode) {
      frameworkName =
          static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }

    for (std::shared_ptr<PropertyNode> property : properties) {
      resolveConstantsInNode(property->getValue(), scope, tree, frameworkName);
      std::shared_ptr<ValueNode> newValue =
          resolveConstant(property->getValue(), scope, tree, frameworkName);
      if (!newValue) {
        // try to resolve on global namespace
        newValue = resolveConstant(property->getValue(), scope, tree);
      }
      if (newValue) {
        property->replaceValue(newValue);
      }
    }
    std::shared_ptr<BundleNode> bundle = decl->getBundle();
    resolveConstantsInNode(bundle->index(), scope, tree);
  } else if (node->getNodeType() == AST::Expression) {
    std::shared_ptr<ExpressionNode> expr =
        static_pointer_cast<ExpressionNode>(node);
    if (expr->isUnary()) {
      resolveConstantsInNode(expr->getValue(), scope, tree);
      if (expr->getValue()->getNodeType() == AST::Expression) {
        std::shared_ptr<ExpressionNode> exprValue =
            static_pointer_cast<ExpressionNode>(expr->getValue());
        std::shared_ptr<ValueNode> newValue =
            reduceConstExpression(exprValue, scope, tree);
        if (newValue) {
          exprValue->replaceValue(newValue);
        }
      }
    } else {
      resolveConstantsInNode(expr->getLeft(), scope, tree);
      resolveConstantsInNode(expr->getRight(), scope, tree);
      if (expr->getLeft()->getNodeType() == AST::Expression) {
        std::shared_ptr<ExpressionNode> exprValue =
            static_pointer_cast<ExpressionNode>(expr->getLeft());
        std::shared_ptr<ValueNode> newValue =
            reduceConstExpression(exprValue, scope, tree);
        if (newValue) {
          expr->replaceLeft(newValue);
        }
      }
      if (expr->getRight()->getNodeType() == AST::Expression) {
        std::shared_ptr<ExpressionNode> exprValue =
            static_pointer_cast<ExpressionNode>(expr->getRight());
        std::shared_ptr<ValueNode> newValue =
            reduceConstExpression(exprValue, scope, tree);
        if (newValue) {
          expr->replaceRight(newValue);
        }
      }
    }
  } else if (node->getNodeType() == AST::List) {
    std::map<ASTNode, ASTNode> replaceMap;
    for (ASTNode element : node->getChildren()) {
      resolveConstantsInNode(element, scope, tree);
      std::shared_ptr<ValueNode> newValue;
      if (element->getNodeType() == AST::Expression) {
        newValue = reduceConstExpression(
            std::static_pointer_cast<ExpressionNode>(element), scope, tree);

      } else {
        newValue = resolveConstant(element, scope, tree);
      }
      if (newValue) {
        replaceMap[element] = newValue;
      }
    }
    std::shared_ptr<ListNode> list = static_pointer_cast<ListNode>(node);
    for (auto &values : replaceMap) {
      list->replaceMember(values.second, values.first);
    }

  } else if (node->getNodeType() == AST::Bundle) {
    std::shared_ptr<BundleNode> bundle = static_pointer_cast<BundleNode>(node);
    std::shared_ptr<ListNode> index = bundle->index();
    resolveConstantsInNode(index, scope, tree);
  }
}

void CodeResolver::propagateDomainsForNode(ASTNode node,
                                           ScopeStack scopeStack) {
  if (node->getNodeType() == AST::Stream) {
    ASTNode contextDomain = mContextDomainStack.back();
    resolveDomainsForStream(static_pointer_cast<StreamNode>(node), scopeStack,
                            contextDomain);
  } else if (node->getNodeType() == AST::Declaration) {
    std::shared_ptr<DeclarationNode> module =
        static_pointer_cast<DeclarationNode>(node);
    if (module->getObjectType() == "module" ||
        module->getObjectType() == "reaction" ||
        module->getObjectType() == "loop") {
      vector<ASTNode> streamsNode = getModuleStreams(module);
      vector<ASTNode>::reverse_iterator streamIt = streamsNode.rbegin();

      vector<ASTNode> blocks = getModuleBlocks(module);
      ASTNode ports = module->getPropertyValue("ports");

      if (ports && ports->getNodeType() ==
                       AST::List) {  // We need to add ports to stack because
                                     // users might need to query their
                                     // properties e.g. Port.domain
        auto portList = ports->getChildren();
        blocks.insert(blocks.begin(), portList.begin(), portList.end());
      }

      scopeStack.push_back({module->getName(), blocks});

      auto contextDomainNode = getModuleContextDomain(module);

      while (streamIt != streamsNode.rend()) {
        const ASTNode streamNode = *streamIt;
        if (streamNode->getNodeType() == AST::Stream) {
          resolveDomainsForStream(static_pointer_cast<StreamNode>(streamNode),
                                  scopeStack, contextDomainNode);
        } else {
          qDebug() << "ERROR: Expecting stream.";
        }
        streamIt++;
      }

      // Now go forward to process anything that was missed
      vector<ASTNode>::iterator streamIt2 = streamsNode.begin();
      while (streamIt2 != streamsNode.end()) {
        const ASTNode streamNode = *streamIt2;
        if (streamNode->getNodeType() == AST::Stream) {
          resolveDomainsForStream(static_pointer_cast<StreamNode>(streamNode),
                                  scopeStack, contextDomainNode);
        } else {
          qDebug() << "ERROR: Expecting stream.";
        }
        streamIt2++;
      }
      //            scopeStack <<
      //            QVector<ASTNode>::fromStdVector(moduleBlocks);
      for (auto block : blocks) {
        propagateDomainsForNode(block, scopeStack);
      }
      //            if (contextDomainDecl) {
      //                setContextDomain(blocks, contextDomainDecl);
      //            }
    }
  }
}

std::shared_ptr<ValueNode> CodeResolver::multiply(
    std::shared_ptr<ValueNode> left, std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() * right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else {  // Automatic casting from int to real
    Q_ASSERT((left->getNodeType() == AST::Real &&
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

std::shared_ptr<ValueNode> CodeResolver::divide(
    std::shared_ptr<ValueNode> left, std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() / right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else {  // Automatic casting from int to real
    Q_ASSERT((left->getNodeType() == AST::Real &&
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

std::shared_ptr<ValueNode> CodeResolver::add(std::shared_ptr<ValueNode> left,
                                             std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() + right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else {  // Automatic casting from int to real
    Q_ASSERT((left->getNodeType() == AST::Real &&
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

std::shared_ptr<ValueNode> CodeResolver::subtract(
    std::shared_ptr<ValueNode> left, std::shared_ptr<ValueNode> right) {
  if (left->getNodeType() == AST::Int && right->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        left->getIntValue() - right->getIntValue(), left->getFilename().data(),
        left->getLine());
  } else {  // Automatic casting from int to real
    Q_ASSERT((left->getNodeType() == AST::Real &&
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

std::shared_ptr<ValueNode> CodeResolver::unaryMinus(
    std::shared_ptr<ValueNode> value) {
  if (value->getNodeType() == AST::Int) {
    return std::make_shared<ValueNode>(
        -value->getIntValue(), value->getFilename().data(), value->getLine());
  } else if (value->getNodeType() == AST::Real) {
    return std::make_shared<ValueNode>(
        -value->getRealValue(), value->getFilename().data(), value->getLine());
  }
  return nullptr;
}

std::shared_ptr<ValueNode> CodeResolver::logicalAnd(
    std::shared_ptr<ValueNode> left, std::shared_ptr<ValueNode> right) {
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

std::shared_ptr<ValueNode> CodeResolver::logicalOr(
    std::shared_ptr<ValueNode> left, std::shared_ptr<ValueNode> right) {
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

std::shared_ptr<ValueNode> CodeResolver::logicalNot(
    std::shared_ptr<ValueNode> value) {
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

std::shared_ptr<StreamNode> CodeResolver::makeStreamFromStack(
    std::vector<ASTNode> &stack) {
  Q_ASSERT(stack.size() > 1);
  ASTNode lastNode = stack.back();
  stack.pop_back();
  std::shared_ptr<StreamNode> newStream = std::make_shared<StreamNode>(
      stack.back(), lastNode, lastNode->getFilename().c_str(),
      lastNode->getLine());
  stack.pop_back();
  while (stack.size() > 0) {
    lastNode = stack.back();
    newStream = std::make_shared<StreamNode>(lastNode, newStream,
                                             lastNode->getFilename().c_str(),
                                             lastNode->getLine());
    stack.pop_back();
  }
  return newStream;
}

std::vector<ASTNode> CodeResolver::sliceStreamByDomain(
    std::shared_ptr<StreamNode> stream, ScopeStack scopeStack) {
  std::vector<ASTNode> streams;
  std::vector<ASTNode> stack;

  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();
  std::string domainName;
  std::string previousDomainName =
      CodeValidator::getNodeDomainName(left, scopeStack, m_tree);
  while (left) {
    domainName = CodeValidator::getNodeDomainName(left, scopeStack, m_tree);
    if (left->getNodeType() == AST::Int || left->getNodeType() == AST::Real ||
        left->getNodeType() == AST::Switch) {
      // If constant, ignore domain and go to next member
      stack.push_back(left);
      if (right->getNodeType() == AST::Stream) {
        //            stream = static_pointer_cast<StreamNode>(right);
        StreamNode *subStream = static_cast<StreamNode *>(right.get());
        left = subStream->getLeft();
        right = subStream->getRight();
      } else {
        left = right;  // Last pass (process right, call it left)
        stack.push_back(right);
        if (stack.size() != 0) {
          streams.push_back(makeStreamFromStack(stack));
        }
        break;
      }
      // Force it to take the domain from the left to avoid domain slicing
      domainName = CodeValidator::getNodeDomainName(left, scopeStack, m_tree);
      if (previousDomainName.size() == 0) {
        previousDomainName = domainName;
      }
    }
    stack.push_back(left);
    if (right->getNodeType() == AST::Stream) {
      //            stream = static_pointer_cast<StreamNode>(right);
      StreamNode *subStream = static_cast<StreamNode *>(right.get());
      left = subStream->getLeft();
      right = subStream->getRight();
    } else {
      stack.push_back(right);
      ASTNode lastNode = nullptr;
      previousDomainName = domainName;
      domainName = CodeValidator::getNodeDomainName(right, scopeStack, m_tree);

      if (domainName != previousDomainName) {
        bool skipSlice = false;
        if (right->getNodeType() == AST::Function) {
          std::shared_ptr<FunctionNode> func =
              static_pointer_cast<FunctionNode>(right);
          std::shared_ptr<DeclarationNode> decl =
              CodeValidator::findDeclaration(
                  QString::fromStdString(func->getName()), scopeStack, m_tree);
          //                        if (decl->getObjectType() == "reaction"
          //                                || decl->getObjectType() ==
          //                                "loop")
          //                                {
          // TODO hack to avoid slice. The right thing should be to have
          // reactions and loops take the right domain...
          skipSlice = true;
          //                        }
        }
        if (stack.size() > 0) {
          if (left->getNodeType() == AST::Function) {
            skipSlice = true;
          } else if (left->getNodeType() == AST::List) {
            bool allFuncs = true;
            for (auto elem : left->getChildren()) {
              if (elem->getNodeType() != AST::Function) {
                allFuncs = false;
                break;
              }
            }
            if (allFuncs) {
              skipSlice = true;
            }
          }
        }

        m_domainChanges.push_back({previousDomainName, domainName});

        // Don't slice by domain, this will be taken care of in the code
        // generation.
        //                if (!skipSlice && stack.size() > 2) {
        //                    lastNode = stack.back();
        //                    stack.pop_back();
        //                    auto oneFromLastNode = stack.back();
        //                    streams.push_back(makeStreamFromStack(stack));
        //                    stack.push_back(oneFromLastNode);
        //                    stack.push_back(lastNode);
        //                    // terminateStackWithBridge(lastNode, streams,
        //                    stack, scopeStack);
        //                }
      }
      if (stack.size() != 0) {
        streams.push_back(makeStreamFromStack(stack));
      }

      left = right = nullptr;  // Mark last pass;
    }
    previousDomainName = domainName;
  }
  return streams;
}

void CodeResolver::sliceDomainsInNode(std::shared_ptr<DeclarationNode> module,
                                      ScopeStack scopeStack) {
  if (module->getObjectType() == "module" ||
      module->getObjectType() == "reaction" ||
      module->getObjectType() ==
          "loop") {  // TODO how to handle different domains within loops and
                     // reactions?
    ASTNode streamsNode = module->getPropertyValue("streams");
    ASTNode blocksNode = module->getPropertyValue("blocks");

    Q_ASSERT(blocksNode);
    if (blocksNode->getNodeType() == AST::None) {
      module->replacePropertyValue(
          "blocks", std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
      blocksNode = module->getPropertyValue("blocks");
    }

    //        if (!streamsNode) {
    //           module->setPropertyValue("streams",
    //           std::make_shared<ListNode>(nullptr, __FILE__, __LINE__));
    //           streamsNode = module->getPropertyValue("streams");
    //        }

    std::shared_ptr<ListNode> newStreamsList =
        std::make_shared<ListNode>(nullptr, __FILE__, __LINE__);
    scopeStack.push_back({module->getName(), CodeValidator::getBlocksInScope(
                                                 module, scopeStack, m_tree)});
    if (streamsNode->getNodeType() == AST::List) {
      for (ASTNode stream : streamsNode->getChildren()) {
        if (stream->getNodeType() == AST::Stream) {
          std::vector<ASTNode> streams = sliceStreamByDomain(
              static_pointer_cast<StreamNode>(stream), scopeStack);
          for (ASTNode streamNode : streams) {
            if (streamNode->getNodeType() == AST::Stream) {
              newStreamsList->addChild(streamNode);
            } else if (streamNode->getNodeType() == AST::Declaration ||
                       streamNode->getNodeType() == AST::BundleDeclaration) {
              blocksNode->addChild(streamNode);
            } else {
              qDebug() << "Stream slicing must result in streams or blocks.";
            }
          }
        }
      }
    } else if (streamsNode->getNodeType() == AST::Stream) {
      std::vector<ASTNode> streams = sliceStreamByDomain(
          static_pointer_cast<StreamNode>(streamsNode), scopeStack);
      for (ASTNode streamNode : streams) {
        if (streamNode->getNodeType() == AST::Stream) {
          newStreamsList->addChild(streamNode);
        } else if (streamNode->getNodeType() == AST::Declaration ||
                   streamNode->getNodeType() == AST::BundleDeclaration) {
          blocksNode->addChild(streamNode);
        } else {
          qDebug() << "Stream slicing must result in streams or blocks.";
        }
      }
    }
    module->replacePropertyValue("streams", newStreamsList);
    for (auto block : blocksNode->getChildren()) {
      if (block->getNodeType() == AST::Declaration) {
        // TODO this is untested and likely not completely working...
        std::shared_ptr<DeclarationNode> decl =
            static_pointer_cast<DeclarationNode>(block);
        scopeStack.push_back({module->getName(), blocksNode->getChildren()});
        sliceDomainsInNode(decl, scopeStack);
      }
    }
  }
}

ASTNode CodeResolver::getModuleContextDomain(
    std::shared_ptr<DeclarationNode> moduleDecl) {
  auto blocks = moduleDecl->getPropertyValue("blocks");
  auto outputPortBlock = CodeValidator::getMainOutputPortBlock(moduleDecl);
  ASTNode contextDomainBlock;
  bool contextDomainSet = false;
  if (outputPortBlock) {
    if (blocks) {
      contextDomainBlock = CodeValidator::getNodeDomain(
          outputPortBlock->getPropertyValue("block"),
          {{moduleDecl->getName(), blocks->getChildren()}}, m_tree);
    }
  }
  auto inputPortBlock = CodeValidator::getMainInputPortBlock(moduleDecl);
  if (inputPortBlock && !contextDomainSet) {
    if (blocks) {
      contextDomainBlock = CodeValidator::getNodeDomain(
          inputPortBlock->getPropertyValue("block"),
          {{moduleDecl->getName(), blocks->getChildren()}}, m_tree);
    }
  }
  return contextDomainBlock;
}

// void CodeResolver::setContextDomain(
//    vector<ASTNode> nodes, std::shared_ptr<DeclarationNode>
//    domainDeclaration)
//    {
//  Q_ASSERT(domainDeclaration);
//  for (ASTNode node : nodes) {
//    if (node->getNodeType() == AST::Declaration ||
//        node->getNodeType() == AST::BundleDeclaration) {
//      std::shared_ptr<DeclarationNode> decl =
//          static_pointer_cast<DeclarationNode>(node);
//      if (decl->getObjectType() == "signal" ||
//          decl->getObjectType() == "switch") {
//        // Check if signal has domain
//        ASTNode nodeDomain =
//            CodeValidator::getNodeDomain(decl, ScopeStack(), m_tree);
//        if (!nodeDomain || nodeDomain->getNodeType() == AST::None) {
//          decl->setDomainString(domainDeclaration->getName());
//          if (CodeValidator::getNodeRate(decl, ScopeStack(), m_tree) < 0) {
//            ASTNode rateValue = domainDeclaration->getPropertyValue("rate");
//            if (rateValue->getNodeType() == AST::Int ||
//                rateValue->getNodeType() == AST::Real) {
//              double rate = static_cast<ValueNode
//              *>(rateValue.get())->toReal();
//              CodeValidator::setNodeRate(decl, rate, ScopeStack(), m_tree);
//            } else if (rateValue->getNodeType() == AST::PortProperty) {
//              decl->replacePropertyValue("rate", rateValue->deepCopy());
//            } else {
//              qDebug() << "Unexpected type for rate in domain declaration: "
//                       <<
//                       QString::fromStdString(domainDeclaration->getName());
//            }
//          }
//        }
//      }
//    } else if (node->getNodeType() == AST::Stream) {
//      // We need to go through setting the domain for functions in stream as
//      // they are the instances
//      //            setContextDomainForStreamNode(node, domainDeclaration);
//    }
//  }
//}

// void CodeResolver::setContextDomainForStreamNode(ASTNode node,
// std::shared_ptr<DeclarationNode> domainDeclaration)
//{
//    if (node->getNodeType() == AST::Stream) {
//        auto stream = static_pointer_cast<StreamNode>(node);
//        setContextDomainForStreamNode(stream->getLeft(), domainDeclaration);
//        setContextDomainForStreamNode(stream->getRight(),
//        domainDeclaration);
//    } else if (node->getNodeType() == AST::Function) {
//        auto func = static_pointer_cast<FunctionNode>(node);
//        auto domainProperty = func->getPropertyValue("domain");
//        if (!domainProperty || domainProperty->getNodeType() ==AST::None) {
//            func->setPropertyValue("domain",
//            make_shared<ValueNode>(domainDeclaration->getName(), __FILE__,
//            __LINE__));
//        }
//    }
//}

// void CodeResolver::populateContextDomains(vector<ASTNode> nodes)
//{
//    for (auto node: nodes) {
//        if (node->getNodeType() == AST::Declaration) {
//            std::shared_ptr<DeclarationNode> decl =
//            std::static_pointer_cast<DeclarationNode>(node); if
//            (decl->getObjectType() == "module" || decl->getObjectType() ==
//            "reaction" || decl->getObjectType() == "loop") {
//                auto blocks = decl->getPropertyValue("blocks");
//                bool hasContextDomain = false;
//                for (auto block: blocks->getChildren()) {
//                    if (block->getNodeType() == AST::Declaration) {
//                        auto internalDecl =
//                        std::static_pointer_cast<DeclarationNode>(block); if
//                        (internalDecl->getName() == "_ContextDomain") {
//                            hasContextDomain = true;
//                            break;
//                        }
//                    }
//                }
//                if (!hasContextDomain) {
//                    // This is currently hard coded to try output first,
//                    then input. This order should somehow be specified
//                    within Stride itself auto outputPortBlock =
//                    CodeValidator::getMainOutputPortBlock(decl); bool
//                    contextDomainSet = false; if (outputPortBlock) {
//                        QVector<ASTNode> scopeStack;
//                        if (blocks) {
//                            scopeStack =
//                            QVector<ASTNode>::fromStdVector(blocks->getChildren());
//                            auto domainBlock =
//                            CodeValidator::getNodeDomain(outputPortBlock->getPropertyValue("block"),
//                            scopeStack, m_tree); if (domainBlock) {
//                                ASTNode propertiesList =
//                                std::make_shared<ListNode>(
//                                            std::make_shared<PropertyNode>("value",
//                                                                           domainBlock,
//                                                                           __FILE__,
//                                                                           __LINE__),
//                                                                           __FILE__,
//                                                                           __LINE__);
//                                blocks->addChild(std::make_shared<DeclarationNode>("_ContextDomain",
//                                "constant", propertiesList, __FILE__,
//                                __LINE__)); contextDomainSet = true;
//                            }
//                        }
//                    }
//                    auto inputPortBlock =
//                    CodeValidator::getMainInputPortBlock(decl); if
//                    (inputPortBlock && !contextDomainSet) {
//                        QVector<ASTNode> scopeStack;
//                        if (blocks) {
//                            scopeStack =
//                            QVector<ASTNode>::fromStdVector(blocks->getChildren());
//                            auto domainBlock =
//                            CodeValidator::getNodeDomain(inputPortBlock->getPropertyValue("block"),
//                            scopeStack, m_tree); if (domainBlock) {
//                                ASTNode propertiesList =
//                                std::make_shared<ListNode>(
//                                            std::make_shared<PropertyNode>("value",
//                                                                           domainBlock,
//                                                                           __FILE__,
//                                                                           __LINE__),
//                                                                           __FILE__,
//                                                                           __LINE__);
//                                blocks->addChild(std::make_shared<DeclarationNode>("_ContextDomain",
//                                "constant", propertiesList, __FILE__,
//                                __LINE__)); contextDomainSet = true;
//                            }
//                        }
//                    }
//                }
//// populateContextDomains(decl->getPropertyValue("blocks")->getChildren()) ;
///// Do it recursively
//            }
//        }
//    }
//}

void CodeResolver::resolveDomainForStreamNode(ASTNode node,
                                              ScopeStack scopeStack) {
  ASTNode domain = nullptr;
  if (node->getNodeType() == AST::Block || node->getNodeType() == AST::Bundle) {
    std::shared_ptr<DeclarationNode> declaration =
        CodeValidator::findDeclaration(CodeValidator::streamMemberName(node),
                                       scopeStack, m_tree,
                                       node->getNamespaceList());
    if (declaration) {
      auto typeDeclaration =
          CodeValidator::findTypeDeclaration(declaration, scopeStack, m_tree);
      if (typeDeclaration &&
          typeDeclaration->getObjectType() == "platformBlock") {
        domain = node->getCompilerProperty("domain");
      } else {
        domain = static_cast<DeclarationNode *>(declaration.get())->getDomain();
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    domain = static_cast<FunctionNode *>(node.get())->getDomain();
  } else if (node->getNodeType() == AST::List) {
    for (ASTNode member : node->getChildren()) {
      resolveDomainForStreamNode(member, scopeStack);
    }
    return;
  } else if (node->getNodeType() == AST::Expression) {
    ExpressionNode *expr = static_cast<ExpressionNode *>(node.get());
    if (expr->isUnary()) {
      resolveDomainForStreamNode(expr->getValue(), scopeStack);
    } else {
      resolveDomainForStreamNode(expr->getLeft(), scopeStack);
      resolveDomainForStreamNode(expr->getRight(), scopeStack);
    }
    return;
  } else if (node->getNodeType() == AST::PortProperty) {
    domain = node->getCompilerProperty("domain");
  }
  if (domain) {
    std::shared_ptr<DeclarationNode> domainDeclaration;
    if (domain->getNodeType() == AST::PortProperty) {
      auto domainNameNode = static_pointer_cast<PortPropertyNode>(domain);
      domain = resolvePortProperty(domainNameNode, scopeStack);
      if (!domain) {
        domain = domainNameNode;
      }
    }
    if (domain->getNodeType() == AST::Block) {  // Resolve domain name
      auto domainNameNode = static_pointer_cast<BlockNode>(domain);
      domainDeclaration = CodeValidator::findDeclaration(
          QString::fromStdString(domainNameNode->getName()), scopeStack,
          m_tree);
    }
    if (domainDeclaration) {
      ASTNode domainValue = domainDeclaration->getPropertyValue("domainName");
      while (domainValue && domainValue->getNodeType() == AST::Block) {
        auto recurseDomain = static_pointer_cast<BlockNode>(domainValue);
        domainDeclaration = CodeValidator::findDeclaration(
            QString::fromStdString(recurseDomain->getName()), scopeStack,
            m_tree);
        domainValue = domainDeclaration->getPropertyValue("name");
      }
      if (domainValue && domainValue->getNodeType() == AST::String) {
        string domainName =
            static_cast<ValueNode *>(domainValue.get())->getStringValue();
        if (node->getNodeType() == AST::Block ||
            node->getNodeType() == AST::Bundle) {
          std::shared_ptr<DeclarationNode> declaration =
              CodeValidator::findDeclaration(
                  CodeValidator::streamMemberName(node), scopeStack, m_tree);
          //                    declaration->setDomainString(domainName);
        }
        //                else if (node->getNodeType() == AST::Function) {
        //                    // FIXME we need to find the domain declaration
        //                    that matches this domain name
        //                    // This is currently assuming the domain name
        //                    and the domain block delcaration name are the
        //                    same
        //                     static_cast<FunctionNode
        //                     *>(node.get())->setDomainString(domainName);
        //                }
      }
    } else {
    }
  }
}

void CodeResolver::remapStreamDomains(std::shared_ptr<StreamNode> stream,
                                      std::map<string, string> domainMap,
                                      ScopeStack scopeStack, ASTNode tree) {
  ASTNode current = stream->getLeft();
  ASTNode next = stream->getRight();

  while (current) {
    auto instance = CodeValidator::getInstance(current, scopeStack, tree);
    if (instance) {
      if (instance->getNodeType() == AST::Expression ||
          instance->getNodeType() == AST::List) {
        for (auto element : instance->getChildren()) {
          auto elementInstance =
              CodeValidator::getInstance(element, scopeStack, tree);
          if (elementInstance->getCompilerProperty("reads")) {
            std::shared_ptr<ListNode> newReads =
                std::make_shared<ListNode>(__FILE__, __LINE__);
            for (auto readDomain :
                 elementInstance->getCompilerProperty("reads")->getChildren()) {
              if (domainMap.find(CodeValidator::getDomainIdentifier(
                      readDomain, scopeStack, m_tree)) != domainMap.end()) {
                std::shared_ptr<PortPropertyNode> mappedDomain =
                    std::make_shared<PortPropertyNode>("OutputPort", "domain",
                                                       __FILE__, __LINE__);
                newReads->addChild(mappedDomain);
                //                                readDomain
              } else {
                newReads->addChild(readDomain);
              }
            }
            elementInstance->setCompilerProperty("reads", newReads);
          }
        }
        if (instance->getCompilerProperty("reads")) {
          std::shared_ptr<ListNode> newReads =
              std::make_shared<ListNode>(__FILE__, __LINE__);
          for (auto readDomain :
               instance->getCompilerProperty("reads")->getChildren()) {
            if (domainMap.find(CodeValidator::getDomainIdentifier(
                    readDomain, scopeStack, m_tree)) != domainMap.end()) {
              std::shared_ptr<PortPropertyNode> mappedDomain =
                  std::make_shared<PortPropertyNode>("OutputPort", "domain",
                                                     __FILE__, __LINE__);
              newReads->addChild(mappedDomain);
              //                                readDomain
            } else {
              newReads->addChild(readDomain);
            }
            instance->setCompilerProperty("reads", newReads);
          }
        }

        if (instance->getCompilerProperty("writes")) {
          std::shared_ptr<ListNode> newWrites =
              std::make_shared<ListNode>(__FILE__, __LINE__);
          for (auto writeDomain :
               instance->getCompilerProperty("writes")->getChildren()) {
            if (domainMap.find(CodeValidator::getDomainIdentifier(
                    writeDomain, scopeStack, m_tree)) != domainMap.end()) {
              std::shared_ptr<PortPropertyNode> mappedDomain =
                  std::make_shared<PortPropertyNode>("OutputPort", "domain",
                                                     __FILE__, __LINE__);
              newWrites->addChild(mappedDomain);
              //                                readDomain
            } else {
              newWrites->addChild(writeDomain);
            }
          }
          instance->setCompilerProperty("writes", newWrites);
        }
      }

      if (!next) {
        current = nullptr;
      } else if (next->getNodeType() == AST::Stream) {
        current = stream->getLeft();
        next = stream->getRight();
      } else {
        current = next;
        next = nullptr;
      }
    }
  }
  //    // Now move internal domains to outerDomains
  //    for (auto inMap : blockInMap) {
  //        if (domainMap.find(inMap.first) != domainMap.end()) {
  //            for (auto b : inMap.second) {
  //                std::shared_ptr<ListNode> newReads =
  //                    std::make_shared<ListNode>(__FILE__, __LINE__);
  //                for (auto readDomain :
  //                     b.externalConnection->getCompilerProperty("reads")
  //                         ->getChildren()) {
  //                    if
  //                    (domainMap.find(CodeValidator::getDomainIdentifier(
  //                            readDomain, scopeStack, m_tree)) !=
  //                        domainMap.end()) {
  //                        std::cout << std::endl;
  //                        std::shared_ptr<PortPropertyNode> mappedDomain =
  //                            std::make_shared<PortPropertyNode>(
  //                                "OutputPort", "domain", __FILE__,
  //                                __LINE__);
  //                        newReads->addChild(mappedDomain);
  //                        //                                readDomain
  //                    } else {
  //                        newReads->addChild(readDomain);
  //                    }
  //                }
  //                b.externalConnection->setCompilerProperty("reads",
  //                                                          newReads);
  //            }
  //            blockInMap[domainMap[inMap.first]].insert(
  //                blockInMap[domainMap[inMap.first]].begin(),
  //                inMap.second.begin(), inMap.second.end());
  //            inMap.second.clear();
  //        }
  //    }
  //    for (auto outMap : blockOutMap) {
  //        if (domainMap.find(outMap.first) != domainMap.end()) {
  //            blockInMap[domainMap[outMap.first]].insert(
  //                blockInMap[domainMap[outMap.first]].begin(),
  //                outMap.second.begin(), outMap.second.end());
  //            outMap.second.clear();
  //        }
  //    }
}

ASTNode CodeResolver::resolvePortProperty(
    std::shared_ptr<PortPropertyNode> portProperty, ScopeStack scopeStack) {
  ASTNode resolved;
  auto decl = CodeValidator::findDeclaration(
      QString::fromStdString(portProperty->getPortName()), scopeStack, m_tree);
  if (decl) {
    resolved = decl->getPropertyValue(portProperty->getName());
    if (resolved && resolved->getNodeType() == AST::PortProperty) {
      auto resolvedPortProperty =
          std::static_pointer_cast<PortPropertyNode>(resolved);
      if (decl->getName() !=
          resolvedPortProperty->getPortName()) {  // To avoid infinite recursion
        resolved = resolvePortProperty(resolvedPortProperty, scopeStack);
      }
    }
  }
  return resolved;
}

void CodeResolver::setInputBlockForFunction(std::shared_ptr<FunctionNode> func,
                                            ScopeStack scopeStack,
                                            ASTNode previous) {
  if (previous) {
    func->setCompilerProperty("inputBlock", previous);
  }
  std::vector<ASTNode> blocks;
  auto funcDecl = CodeValidator::findDeclaration(
      QString::fromStdString(func->getName()), scopeStack, m_tree);
  if (!funcDecl) return;
  auto blocksNode = funcDecl->getPropertyValue("blocks");
  if (blocksNode) {
    if (blocksNode->getNodeType() == AST::List) {
      blocks = blocksNode->getChildren();
    } else if (blocksNode->getNodeType() == AST::Declaration ||
               blocksNode->getNodeType() == AST::BundleDeclaration) {
      blocks.push_back(blocksNode);
    }
  }
  auto streamsNode = funcDecl->getPropertyValue("streams");
  if (streamsNode) {
    for (auto stream : streamsNode->getChildren()) {
      if (stream->getNodeType() == AST::Stream) {
        scopeStack.push_back({funcDecl->getName(), blocks});
        checkStreamConnections(static_pointer_cast<StreamNode>(stream),
                               scopeStack, nullptr);
      }
    }
  }
}

void CodeResolver::setOutputBlockForFunction(std::shared_ptr<FunctionNode> func,
                                             ScopeStack scopeStack,
                                             ASTNode next) {
  if (next) {
    func->setCompilerProperty("outputBlock", next);
  }
  std::vector<ASTNode> blocks;
  auto funcDecl = CodeValidator::findDeclaration(
      QString::fromStdString(func->getName()), scopeStack, m_tree);
  if (!funcDecl) return;
  auto blocksNode = funcDecl->getPropertyValue("blocks");
  if (blocksNode) {
    if (blocksNode->getNodeType() == AST::List) {
      blocks = blocksNode->getChildren();
    } else if (blocksNode->getNodeType() == AST::Declaration ||
               blocksNode->getNodeType() == AST::BundleDeclaration) {
      blocks.push_back(blocksNode);
    }
  }
  auto streamsNode = funcDecl->getPropertyValue("streams");
  if (streamsNode) {
    for (auto stream : streamsNode->getChildren()) {
      if (stream->getNodeType() == AST::Stream) {
        scopeStack.push_back({funcDecl->getName(), blocks});
        checkStreamConnections(static_pointer_cast<StreamNode>(stream),
                               scopeStack, nullptr);
      }
    }
  }
}

void CodeResolver::checkStreamConnections(std::shared_ptr<StreamNode> stream,
                                          ScopeStack scopeStack,
                                          ASTNode previous) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();

  markConnectionForNode(left, scopeStack, previous);

  if (left->getNodeType() == AST::Function) {
    auto func = static_pointer_cast<FunctionNode>(left);
    setInputBlockForFunction(func, scopeStack, previous);
  } else if (left->getNodeType() == AST::List) {
    if (previous) {
      if (previous->getNodeType() == AST::List &&
          previous->getChildren().size() > 0) {
        auto previousChildren = previous->getChildren();
        std::vector<ASTNode>::iterator connection = previousChildren.begin();
        for (auto child : left->getChildren()) {
          if (connection == previousChildren.end()) {
            connection = previousChildren.begin();
          }
          if (child->getNodeType() == AST::Function) {
            auto func = static_pointer_cast<FunctionNode>(child);
            setInputBlockForFunction(func, scopeStack, *connection);
          }
          connection++;
        }
      } else {
        for (auto child : left->getChildren()) {
          if (child->getNodeType() == AST::Function) {
            auto func = static_pointer_cast<FunctionNode>(child);
            setInputBlockForFunction(func, scopeStack, previous);
          }
        }
      }
    }
  } else if (left->getNodeType() == AST::Declaration) {
    left->setCompilerProperty("inputBlock", previous);
    auto decl = static_pointer_cast<DeclarationNode>(left);
    auto blocks = decl->getPropertyValue("blocks");
    auto streams = decl->getPropertyValue("streams");

    std::vector<ASTNode> blocksList;
    if (blocks) {
      for (auto block : blocks->getChildren()) {
        blocksList.push_back(block);
      }
    }
    scopeStack.push_back(std::pair<std::string, std::vector<ASTNode>>(
        std::string(), blocksList));
    if (streams) {
      for (auto stream : streams->getChildren()) {
        if (stream->getNodeType() == AST::Stream) {
          checkStreamConnections(static_pointer_cast<StreamNode>(stream),
                                 scopeStack, nullptr);
        }
      }
    }
    scopeStack.pop_back();
  }

  if (right->getNodeType() == AST::Function) {
    markConnectionForNode(right, scopeStack, left);
    previous = left;
    auto func = static_pointer_cast<FunctionNode>(right);
    setInputBlockForFunction(func, scopeStack, previous);
  } else if (right->getNodeType() == AST::List) {
    previous = left;
    for (auto child : right->getChildren()) {
      if (child->getNodeType() == AST::Function) {
        auto func = static_pointer_cast<FunctionNode>(child);
        setInputBlockForFunction(func, scopeStack, previous);
      }
    }
  } else if (right->getNodeType() == AST::Declaration) {
    auto decl = static_pointer_cast<DeclarationNode>(right);
    auto blocks = decl->getPropertyValue("blocks");
    auto streams = decl->getPropertyValue("streams");
    markConnectionForNode(right, scopeStack, left);
    decl->setCompilerProperty("inputBlock", left);
    std::vector<ASTNode> blocksList;
    if (blocks) {
      for (auto block : blocks->getChildren()) {
        blocksList.push_back(block);
      }
    }
    scopeStack.push_back(std::pair<std::string, std::vector<ASTNode>>(
        std::string(), blocksList));
    if (streams) {
      for (auto stream : streams->getChildren()) {
        if (stream->getNodeType() == AST::Stream) {
          checkStreamConnections(static_pointer_cast<StreamNode>(stream),
                                 scopeStack, nullptr);
        }
      }
    }
    scopeStack.pop_back();
  } else {
    previous = left;
  }

  if (right->getNodeType() == AST::Stream) {
    checkStreamConnections(static_pointer_cast<StreamNode>(right), scopeStack,
                           previous);
    if (left->getNodeType() == AST::Function) {
      auto next = static_pointer_cast<StreamNode>(right)->getLeft();
      if (next) {
        if (left->getNodeType() == AST::List) {
          for (auto child : left->getChildren()) {
            if (child->getNodeType() == AST::Function) {
              auto func = static_pointer_cast<FunctionNode>(child);
              setOutputBlockForFunction(
                  func, scopeStack, next->getCompilerProperty("outputBlock"));
              if (child->getCompilerProperty("outputBlock")) {
                next->setCompilerProperty(
                    "inputBlock", child->getCompilerProperty("outputBlock"));
              }
            }
          }
        } else if (left->getNodeType() == AST::Function) {
          auto func = static_pointer_cast<FunctionNode>(left);
          setOutputBlockForFunction(func, scopeStack, next);
          next->setCompilerProperty("inputBlock", func);
        } else {
          auto func = static_pointer_cast<FunctionNode>(left);
          setOutputBlockForFunction(func, scopeStack, next);
          next->setCompilerProperty("inputBlock", func);
        }
      }
    } else if (left->getNodeType() == AST::List) {
      for (auto child : left->getChildren()) {
        auto next = static_pointer_cast<StreamNode>(right)->getLeft();
        //                CodeValidator::getNodeNumInputs()
        if (child->getNodeType() == AST::Function) {
          //                    if (right->getNodeType() == AST::Function) {
          //                        right->getCompilerProperty("inputBlock");
          //                    }
          auto func = static_pointer_cast<FunctionNode>(child);
          setOutputBlockForFunction(func, scopeStack, next);
          //            if (nextBlock) {
          //                left->setCompilerProperty("outputBlock",
          //                nextBlock);
          //            }
        }
      }
    }
  } else {
    markConnectionForNode(right, scopeStack, previous);
    if (left->getNodeType() == AST::List ||
        left->getNodeType() == AST::Expression) {
      for (auto child : left->getChildren()) {
        ASTNode nextBlock = right;
        if (child->getNodeType() == AST::Function) {
          auto func = static_pointer_cast<FunctionNode>(child);
          setOutputBlockForFunction(func, scopeStack, nextBlock);
        }
      }
    } else if (left->getNodeType() == AST::Function) {
      ASTNode nextBlock = right;
      auto func = static_pointer_cast<FunctionNode>(left);
      setOutputBlockForFunction(func, scopeStack, nextBlock);
    }
  }
}

void CodeResolver::markPreviousReads(ASTNode node, ASTNode previous,
                                     ScopeStack scopeStack) {
  if (previous) {
    std::shared_ptr<ListNode> previousReads;
    ASTNode newReadDomain;
    std::string previousName = CodeValidator::streamMemberName(previous);

    std::string name = CodeValidator::streamMemberName(node);
    std::shared_ptr<DeclarationNode> decl =
        CodeValidator::findDeclaration(name, scopeStack, m_tree);
    if (node->getNodeType() == AST::Declaration) {
      decl = static_pointer_cast<DeclarationNode>(node);
    }
    auto nodeTypeName = decl->getObjectType();
    if (nodeTypeName == "module" || nodeTypeName == "reaction" ||
        nodeTypeName == "loop") {
      // Modules, reactions and loops, the domain that matters is the domain
      // of the output block.
      if (node->getNodeType() == AST::Function) {
        auto func = std::static_pointer_cast<FunctionNode>(node);
        // FIXME get this from the functions input port
        newReadDomain = func->getCompilerProperty("domain");
      }
    } else {
      // platformBlock and platformModule can be ignored as they are tied to
      // a domain. However, we do need to figure out how to support buffer
      // access across domains.
      newReadDomain = CodeValidator::getNodeDomain(node, scopeStack, m_tree);
      // FIXME we need to get previousReads for functions from the function
      // itsefl, not the module decl.

      //                        if (previousDeclReads) {
      //                            for (auto read:
      //                            previousDeclReads->getChildren()) {
      //                                if (read && read->getNodeType() ==
      //                                AST::PortProperty
      //                                        &&
      //                                        newReadDomain->getNodeType()
      //                                        == AST::PortProperty) {
      //                                    std::shared_ptr<PortPropertyNode>
      //                                    newReadDomainNode =
      //                                            std::static_pointer_cast<PortPropertyNode>(newReadDomain);
      //                                    std::shared_ptr<PortPropertyNode>
      //                                    existingReadDomain =
      //                                            std::static_pointer_cast<PortPropertyNode>(read);
      //                                    if (
      //                                    (newReadDomainNode->getName()
      //                                    ==
      //                                    existingReadDomain->getName())
      //                                         &&
      //                                         (newReadDomainNode->getPortName()
      //                                         ==
      //                                         existingReadDomain->getPortName())
      //                                         ) {
      //                                        alreadyInReads = true;
      //                                        break;
      //                                    }
      //                                }
      //                            }
      //                            if (!alreadyInReads) {
      //                                previousDeclReads->addChild(newReadDomain);
      //                            }
      //                        }
    }

    auto appendReadDomain = [](std::shared_ptr<ListNode> previousReads,
                               ASTNode newReadDomain) {
      if (previousReads) {
        bool alreadyInReads = false;
        for (auto read : previousReads->getChildren()) {
          if (read && read->getNodeType() == AST::PortProperty &&
              newReadDomain->getNodeType() == AST::PortProperty) {
            std::shared_ptr<PortPropertyNode> newReadDomainNode =
                std::static_pointer_cast<PortPropertyNode>(newReadDomain);
            std::shared_ptr<PortPropertyNode> existingReadDomain =
                std::static_pointer_cast<PortPropertyNode>(read);
            if ((newReadDomainNode->getName() ==
                 existingReadDomain->getName()) &&
                (newReadDomainNode->getPortName() ==
                 existingReadDomain->getPortName())) {
              alreadyInReads = true;
              break;
            }
          } else {
            // FIXME check duplicate string domain names
          }
        }
        if (!alreadyInReads) {
          previousReads->addChild(newReadDomain);
        }
      }
    };
    // TODO we need to support lists here too
    auto previousInstance =
        CodeValidator::getInstance(previous, scopeStack, m_tree);
    if (previousInstance) {
      // For nodes that are related to a single instance (Block, Functions,
      // etc.) Mark the writes for the declaration, not the instance
      previousReads = static_pointer_cast<ListNode>(
          previousInstance->getCompilerProperty("reads"));
      if (!previousReads) {
        previousInstance->setCompilerProperty(
            "reads", std::make_shared<ListNode>(__FILE__, __LINE__));
        previousReads = static_pointer_cast<ListNode>(
            previousInstance->getCompilerProperty("reads"));
      }
    }

    if (previous->getNodeType() == AST::Block ||
        previous->getNodeType() == AST::Bundle) {
      if (node->getNodeType() == AST::Bundle) {
        auto bundleNode = static_pointer_cast<BundleNode>(node);
        //          FIXME process all types of index configurations
        if (bundleNode->index()) {
          for (auto indexNode : bundleNode->index()->getChildren()) {
            markPreviousReads(node, indexNode, scopeStack);
          }
        }
      }
      //                    Q_ASSERT(previousDecl);

      if (newReadDomain) {
        appendReadDomain(previousReads, newReadDomain);
      }

    } else if (previous->getNodeType() == AST::Expression ||
               previous->getNodeType() == AST::List) {
      newReadDomain = previous->getCompilerProperty("samplingDomain");

      std::function<void(ASTNode, ASTNode)> appendReadDomainForExprList =
          [&](ASTNode exprList, ASTNode newReadDomain) {
            if (exprList->getNodeType() == AST::List ||
                exprList->getNodeType() == AST::Expression) {
              for (auto child : exprList->getChildren()) {
                appendReadDomainForExprList(child, newReadDomain);
              }
            } else {
              auto previousInstance =
                  CodeValidator::getInstance(exprList, scopeStack, m_tree);
              if (previousInstance) {
                // For nodes that are related to a single instance (Block,
                // Functions, etc.) Mark the writes for the declaration, not
                // the instance
                previousReads = static_pointer_cast<ListNode>(
                    previousInstance->getCompilerProperty("reads"));
                appendReadDomain(previousReads, newReadDomain);
              }
            }
          };
      appendReadDomainForExprList(previous, newReadDomain);
    } else if (previous->getNodeType() == AST::PortProperty) {
      if (newReadDomain) {
        appendReadDomain(previousReads, newReadDomain);
      }
    }
  }
}

void CodeResolver::markConnectionForNode(ASTNode node, ScopeStack scopeStack,
                                         ASTNode previous,
                                         unsigned int listIndex) {
  if (node->getNodeType() == AST::Block || node->getNodeType() == AST::Bundle ||
      node->getNodeType() == AST::Function ||
      node->getNodeType() == AST::Declaration) {
    std::string name = CodeValidator::streamMemberName(node);

    if (node->getNodeType() == AST::Bundle) {
      auto bundleNode = static_pointer_cast<BundleNode>(node);
      auto domain = CodeValidator::getNodeDomain(node, scopeStack, m_tree);
      for (auto indexChild : bundleNode->index()->getChildren()) {
        markConnectionForNode(indexChild, scopeStack, nullptr);
        if (indexChild->getNodeType() == AST::Block ||
            indexChild->getNodeType() == AST::Bundle) {
          auto frameworkNode = indexChild->getCompilerProperty("framework");
          std::string frameworkName;
          if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
            frameworkName =
                static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
          }
          auto decl = CodeValidator::findDeclaration(
              CodeValidator::streamMemberName(indexChild), scopeStack, m_tree,
              indexChild->getNamespaceList(), frameworkName);
          if (decl) {
            decl->appendToPropertyValue("reads", domain);
          }
        }
      }
    }
    auto nodeInstance = CodeValidator::getInstance(node, scopeStack, m_tree);
    if (previous && nodeInstance) {
      std::shared_ptr<ListNode> nodeWritesProperties =
          static_pointer_cast<ListNode>(
              nodeInstance->getCompilerProperty("writes"));
      if (!nodeWritesProperties) {
        nodeWritesProperties =
            std::make_shared<ListNode>(nullptr, __FILE__, __LINE__);
        nodeInstance->setCompilerProperty("writes", nodeWritesProperties);
      } else {
        //        qDebug() << "WARNING: node already has write properties.";
      }

      bool alreadyInWrites = false;
      ASTNode newWriteDomain;

      if (node->getNodeType() == AST::Function) {
        newWriteDomain = CodeValidator::getNodeDomain(
            CodeValidator::resolveBlock(node, scopeStack, m_tree, false),
            scopeStack, m_tree);

      } else {
        newWriteDomain =
            CodeValidator::getNodeDomain(previous, scopeStack, m_tree);
      }
      for (auto write : nodeWritesProperties->getChildren()) {
        if (write && write->getNodeType() == AST::PortProperty &&
            newWriteDomain &&
            newWriteDomain->getNodeType() == AST::PortProperty) {
          std::shared_ptr<PortPropertyNode> newReadDomainNode =
              std::static_pointer_cast<PortPropertyNode>(newWriteDomain);
          std::shared_ptr<PortPropertyNode> existingReadDomain =
              std::static_pointer_cast<PortPropertyNode>(write);
          if ((newReadDomainNode->getName() == existingReadDomain->getName()) &&
              (newReadDomainNode->getPortName() ==
               existingReadDomain->getPortName())) {
            alreadyInWrites = true;
            break;
          }
        }
      }
      if (!alreadyInWrites && newWriteDomain) {
        nodeWritesProperties->addChild(newWriteDomain);
      }
    }

    std::shared_ptr<DeclarationNode> decl =
        CodeValidator::findDeclaration(name, scopeStack, m_tree);
    if (node->getNodeType() == AST::Declaration) {
      decl = static_pointer_cast<DeclarationNode>(node);
    }
    if (decl) {
      // Mark trigger connections
      if (decl->getObjectType() == "trigger") {
        if (!decl->getCompilerProperty("triggerSources")) {
          decl->setCompilerProperty(
              "triggerSources", std::make_shared<ListNode>(__FILE__, __LINE__));
        }
        auto sourcesList = decl->getCompilerProperty("triggerSources");
        if (previous) {
          if (previous->getNodeType() == AST::List) {
            sourcesList->addChild(previous->getChildren()[listIndex]);
          } else {
            sourcesList->addChild(previous);
          }
        }
      }
      if (decl->getObjectType() == "iterator") {
        auto doneTrigger = decl->getPropertyValue("done");
        if (doneTrigger) {
          auto doneTriggerDecl = CodeValidator::findDeclaration(
              CodeValidator::streamMemberName(doneTrigger), scopeStack, m_tree);
          if (doneTriggerDecl) {
            auto triggerDests =
                doneTriggerDecl->getCompilerProperty("triggerDestinations");
            if (!triggerDests) {
              doneTriggerDecl->setCompilerProperty(
                  "triggerDestinations",
                  std::make_shared<ListNode>(__FILE__, __LINE__));
              triggerDests =
                  doneTriggerDecl->getCompilerProperty("triggerDestinations");
            }
            auto children = triggerDests->getChildren();
            if (std::find(children.begin(), children.end(), decl) ==
                children.end()) {
              triggerDests->addChild(decl);
            }
          }
        }
      }

      // Check if previous is trigger
      std::shared_ptr<DeclarationNode> previousDecl;
      if (previous) {
        previousDecl = CodeValidator::findDeclaration(
            CodeValidator::streamMemberName(previous), scopeStack, m_tree);
        if (previousDecl && previousDecl->getObjectType() == "trigger") {
          if (!previousDecl->getCompilerProperty("triggerDestinations")) {
            previousDecl->setCompilerProperty(
                "triggerDestinations",
                std::make_shared<ListNode>(__FILE__, __LINE__));
          }
          auto triggerDests =
              previousDecl->getCompilerProperty("triggerDestinations");
          triggerDests->addChild(node);
        }
      }

      // Connect reset property to trigger
      auto resetProp = decl->getPropertyValue("reset");
      if (resetProp && (resetProp->getNodeType() == AST::Block ||
                        resetProp->getNodeType() == AST::Bundle)) {
        // TODO should trigger bundles be allowed?
        auto triggerDecl = CodeValidator::findDeclaration(
            CodeValidator::streamMemberName(resetProp), scopeStack, m_tree);
        if (triggerDecl) {
          if (!triggerDecl->getCompilerProperty("triggerResets")) {
            triggerDecl->setCompilerProperty(
                "triggerResets",
                std::make_shared<ListNode>(__FILE__, __LINE__));
          }
          auto resets = triggerDecl->getCompilerProperty("triggerResets");
          bool notRegistered = true;
          for (auto registeredDecl : resets->getChildren()) {
            // FIXME check namespace too
            if (static_pointer_cast<DeclarationNode>(registeredDecl)
                    ->getName() == decl->getName()) {
              notRegistered = false;
              break;
            }
          }
          if (notRegistered) {
            resets->addChild(decl);
            auto domain =
                CodeValidator::getNodeDomain(node, scopeStack, m_tree);
            decl->appendToPropertyValue("resetDomains", domain);
          }
        }
      }

      if (node->getNodeType() == AST::Function) {
        auto blocks = decl->getPropertyValue("blocks");
        if (blocks) {
          for (auto blockDecl : blocks->getChildren()) {
            if (blockDecl->getNodeType() == AST::Declaration) {
              auto decl = static_pointer_cast<DeclarationNode>(blockDecl);
              if (decl->getObjectType() == "reaction" ||
                  decl->getObjectType() == "loop") {
                auto streamsNode = decl->getPropertyValue("streams");
                auto blocksNode = decl->getPropertyValue("blocks");
                std::vector<ASTNode> innerScope;
                if (blocksNode) {
                  for (auto internalBlock : blocksNode->getChildren()) {
                    innerScope.push_back(internalBlock);
                  }
                }
                if (streamsNode) {
                  scopeStack.push_back({"", innerScope});
                  for (auto stream : streamsNode->getChildren()) {
                    if (stream->getNodeType() == AST::Stream) {
                      checkStreamConnections(
                          static_pointer_cast<StreamNode>(stream), scopeStack);
                    }
                  }
                  scopeStack.pop_back();
                }
              }
            }
          }
        }
      }

      // For the declaration too
      std::shared_ptr<ListNode> readsProperties =
          static_pointer_cast<ListNode>(decl->getCompilerProperty("reads"));
      if (!readsProperties) {
        readsProperties = std::make_shared<ListNode>(
            nullptr, node->getFilename().c_str(), node->getLine());
        decl->setCompilerProperty("reads", readsProperties);
      }
      std::shared_ptr<ListNode> writesProperties =
          static_pointer_cast<ListNode>(decl->getCompilerProperty("writes"));
      if (!writesProperties) {
        writesProperties = std::make_shared<ListNode>(
            nullptr, node->getFilename().c_str(), node->getLine());
        decl->setCompilerProperty("writes", writesProperties);
      }
      if (previous) {
        QString previousName;
        std::shared_ptr<ListNode> previousReads;
        //                    std::shared_ptr<DeclarationNode> previousDecl;
        if (previous->getNodeType() == AST::Block ||
            previous->getNodeType() == AST::Bundle) {
          markPreviousReads(node, previous, scopeStack);
        } else if (previous->getNodeType() == AST::Expression) {
          auto expr = static_pointer_cast<ExpressionNode>(previous);
          for (auto child : expr->getChildren()) {
            markConnectionForNode(child, scopeStack, nullptr);
            markPreviousReads(node, child, scopeStack);
            //                        previousReads =
            //                        static_pointer_cast<ListNode>(expr->getCompilerProperty("reads"));
            //                        Q_ASSERT(previousReads);
          }
          //                    previousReads->addChild(decl->getDomain());
        } else if (previous->getNodeType() == AST::List) {
          // How should lists be handled?
          for (auto child : previous->getChildren()) {
            markConnectionForNode(child, scopeStack, nullptr);
            markPreviousReads(node, child, scopeStack);
          }
        } else if (previous->getNodeType() == AST::Function) {
          // FIXME assumes the output port of the module takes the output port
          // domain
          writesProperties->addChild(decl->getDomain());
        } else if (previous->getNodeType() == AST::Int ||
                   previous->getNodeType() == AST::Real ||
                   previous->getNodeType() == AST::Switch ||
                   previous->getNodeType() == AST::String) {
          // FIXME constant writes should be moved to the init domain related
          // to the signal's declared domain.
          writesProperties->addChild(decl->getDomain());
        }
      }
    }
  } else if (node->getNodeType() == AST::Int ||
             node->getNodeType() == AST::Real ||
             node->getNodeType() == AST::Switch ||
             node->getNodeType() == AST::String) {
    //        // First create read and write compiler properties for the node
    //        if not there std::shared_ptr<ListNode> nodeReadsProperties =
    //        static_pointer_cast<ListNode>(node->getCompilerProperty("reads"));
    //        if (!nodeReadsProperties) {
    //            nodeReadsProperties = std::make_shared<ListNode>(nullptr,
    //            node->getFilename().c_str(), node->getLine());
    //            node->setCompilerProperty("reads", nodeReadsProperties);
    //        }
    // Writes property is created but should be empty for ValueNode
    std::shared_ptr<ListNode> nodeWritesProperties =
        static_pointer_cast<ListNode>(node->getCompilerProperty("writes"));
    if (!nodeWritesProperties) {
      nodeWritesProperties = std::make_shared<ListNode>(
          nullptr, node->getFilename().c_str(), node->getLine());
      node->setCompilerProperty("writes", nodeWritesProperties);
    }

    if (previous) {
      QString previousName;
      std::shared_ptr<ListNode> previousReads;
      if (previous->getNodeType() == AST::Block ||
          previous->getNodeType() == AST::Bundle) {
        markPreviousReads(node, previous, scopeStack);
      } else if (previous->getNodeType() == AST::Expression) {
        auto expr = static_pointer_cast<ExpressionNode>(previous);
        for (auto child : expr->getChildren()) {
          markConnectionForNode(child, scopeStack, nullptr);
          markPreviousReads(node, child, scopeStack);
          //                        previousReads =
          //                        static_pointer_cast<ListNode>(expr->getCompilerProperty("reads"));
          //                        Q_ASSERT(previousReads);
        }
        //                    previousReads->addChild(decl->getDomain());
      } else if (previous->getNodeType() == AST::List) {
        // How should lists be handled?
      }
    }
  } else if (node->getNodeType() == AST::Expression) {
    for (auto child : node->getChildren()) {
      markConnectionForNode(child, scopeStack, previous);
    }
    previous = node;
  } else if (node->getNodeType() == AST::List) {
    QList<LangError> errors;
    int index = 0;
    for (auto child : node->getChildren()) {
      int inputSize =
          CodeValidator::getNodeNumInputs(child, scopeStack, m_tree, errors);
      markConnectionForNode(child, scopeStack, previous, index);
      index += inputSize;
    }
    previous = node;
  }
}

void CodeResolver::storeDeclarationsForNode(ASTNode node, ScopeStack scopeStack,
                                            ASTNode tree) {
  if (node->getNodeType() == AST::Function) {
    auto decl = CodeValidator::findDeclaration(
        CodeValidator::streamMemberName(node), scopeStack, tree);
    std::vector<ASTNode> scopeBlocks;
    auto name = static_pointer_cast<FunctionNode>(node)->getName();
    if (decl) {
      auto blocks = decl->getCompilerProperty("blocks");
      if (blocks) {
        for (auto b : blocks->getChildren()) {
          scopeBlocks.push_back(b);
        }
      }
    }
    scopeStack.push_back({name, scopeBlocks});
  }

  if (node->getNodeType() == AST::Block || node->getNodeType() == AST::Bundle ||
      node->getNodeType() == AST::Function) {
    auto decl = CodeValidator::findDeclaration(
        CodeValidator::streamMemberName(node), scopeStack, tree);
    if (decl && decl->getObjectType() != "platformModule") {
      node->setCompilerProperty("declaration", decl);
    }
  } else if (node->getNodeType() == AST::Expression ||
             node->getNodeType() == AST::List) {
    for (auto child : node->getChildren()) {
      storeDeclarationsForNode(child, scopeStack, tree);
    }
  } else if (node->getNodeType() == AST::Stream) {
    auto stream = static_pointer_cast<StreamNode>(node);
    auto streamNode = stream->getLeft();
    while (streamNode) {
      storeDeclarationsForNode(streamNode, scopeStack, m_tree);
      if (stream) {
        if (stream->getRight()->getNodeType() == AST::Stream) {
          stream = static_pointer_cast<StreamNode>(stream->getRight());
          streamNode = stream->getLeft();
        } else {
          streamNode = stream->getRight();
          stream = nullptr;
        }
      } else {
        streamNode = nullptr;
      }
    }
  } else if (node->getNodeType() == AST::Declaration) {
    // TODO support recursive modules (modules declared in modules)
    auto decl = static_pointer_cast<DeclarationNode>(node);
    if (decl->getObjectType() == "module" ||
        decl->getObjectType() == "reaction" ||
        decl->getObjectType() == "loop") {
      auto internalStreams = decl->getPropertyValue("streams");
      auto internalBlocks = decl->getPropertyValue("blocks");
      vector<ASTNode> blocksList;
      if (internalBlocks) {
        if (internalBlocks->getNodeType() == AST::List) {
          for (auto child : internalBlocks->getChildren()) {
            storeDeclarationsForNode(
                child, {{decl->getName(), internalBlocks->getChildren()}},
                m_tree);
            if (child->getNodeType() == AST::Declaration ||
                child->getNodeType() == AST::BundleDeclaration) {
              blocksList.push_back(static_pointer_cast<DeclarationNode>(child));
            } else {
              qDebug() << "Unexpected node type in internal blocks";
            }
          }
        } else if (internalBlocks->getNodeType() == AST::Declaration ||
                   internalBlocks->getNodeType() == AST::BundleDeclaration) {
          blocksList.push_back(
              static_pointer_cast<DeclarationNode>(internalBlocks));
        }
      }
      ScopeStack stack;
      if (decl->getObjectType() == "reaction" ||
          decl->getObjectType() == "loop") {
        // Loops and reactions have access to their parent scope
        if (scopeStack.size() > 0) {
          stack.push_back(scopeStack.back());
        }
      }
      stack.push_back({decl->getName(), blocksList});
      if (internalStreams) {
        if (internalStreams->getNodeType() == AST::List) {
          for (auto node : internalStreams->getChildren()) {
            storeDeclarationsForNode(node, stack, m_tree);
          }
        }
      }
    }
  } else if (node->getNodeType() == AST::List ||
             node->getNodeType() == AST::Expression) {
    for (auto listNode : node->getChildren()) {
      storeDeclarationsForNode(listNode, scopeStack, m_tree);
    }
  }
  if (node->getNodeType() == AST::Function) {
    scopeStack.pop_back();
  }
}

void CodeResolver::appendParent(std::shared_ptr<DeclarationNode> decl,
                                std::shared_ptr<DeclarationNode> parent) {
  auto parentList = decl->getCompilerProperty("parentDeclarations");
  if (!parentList) {
    decl->setCompilerProperty("parentDeclarations",
                              std::make_shared<ListNode>(__FILE__, __LINE__));
    parentList = decl->getCompilerProperty("parentDeclarations");
  }
  if (parent) {
    parentList->addChild(parent);
  }
  if (decl->getObjectType() == "module" ||
      decl->getObjectType() == "reaction" ||
      decl->getObjectType() ==
          "loop") {  // declaration types that can have sub scopes
    auto internalBlocks = decl->getPropertyValue("blocks");
    if (internalBlocks) {
      for (auto node : internalBlocks->getChildren()) {
        if (node->getNodeType() == AST::Declaration ||
            node->getNodeType() == AST::BundleDeclaration) {
          for (auto grandParent : parentList->getChildren()) {
            if (grandParent->getNodeType() == AST::Declaration ||
                grandParent->getNodeType() == AST::BundleDeclaration) {
              appendParent(static_pointer_cast<DeclarationNode>(node),
                           static_pointer_cast<DeclarationNode>(grandParent));
            }
          }
          appendParent(static_pointer_cast<DeclarationNode>(node), decl);
        }
        Q_ASSERT(node->getNodeType() == AST::Declaration ||
                 node->getNodeType() == AST::BundleDeclaration);
      }
    }
  }
}
