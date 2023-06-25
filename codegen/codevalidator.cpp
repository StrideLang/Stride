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
#include "codeanalysis.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>

#include "stride/parser/astfunctions.h"
#include "stride/parser/astquery.h"
#include "stride/parser/astruntime.h"
#include "stride/parser/astvalidation.h"
#include "codequery.hpp"
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

void CodeValidator::validatePlatform(ASTNode tree,
                                     std::vector<LangError> &errors) {
  for (const ASTNode &node : tree->getChildren()) {
    if (node->getNodeType() == AST::Platform) {
      std::shared_ptr<SystemNode> platformNode =
          std::static_pointer_cast<SystemNode>(node);
    }
  }
  std::vector<std::shared_ptr<SystemNode>> systems =
      ASTQuery::getSystemNodes(tree);
  std::shared_ptr<SystemNode> platformNode = systems.at(0);

  for (size_t i = 1; i < systems.size(); i++) {
    std::cerr << "Ignoring system: " << platformNode->platformName()
              << std::endl;
    LangError error;
    error.type = LangError::SystemRedefinition;
    error.errorTokens.push_back(platformNode->platformName());
    error.filename = platformNode->getFilename();
    error.lineNumber = platformNode->getLine();
    errors.push_back(error);
  }
}

std::vector<StreamNode *> CodeValidator::getStreamsAtLine(ASTNode tree,
                                                          int line) {
  std::vector<StreamNode *> streams;
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      if (node->getLine() == line) {
        streams.push_back(static_cast<StreamNode *>(node.get()));
      }
    }
  }
  return streams;
}

std::vector<LangError> CodeValidator::getErrors() { return m_errors; }

std::vector<std::string> CodeValidator::getPlatformErrors() {
  return m_system->getErrors();
}

void CodeValidator::validate() {
  m_errors.clear();
  if (m_tree) {
    validatePlatform(m_tree, m_errors);
    ASTValidation::validateTypes(m_tree, m_errors, {}, m_tree, {}, "");

    validateBundleIndeces(m_tree, m_errors, {});
    validateBundleSizes(m_tree, m_errors, {});
    validateSymbolUniqueness({{nullptr, m_tree->getChildren()}}, m_errors);
    validateStreamSizes(m_tree, m_errors, {});
    validateRates(m_tree);
    validateConstraints(m_tree);
  }
  sortErrors();
}

void CodeValidator::validateBundleIndeces(ASTNode node,
                                          std::vector<LangError> &errors,
                                          ScopeStack scope) {
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
      errors.push_back(error);
    }
  }
  for (ASTNode child : node->getChildren()) {
    std::vector<ASTNode> blocksInScope =
        CodeAnalysis::getBlocksInScope(child, scope, m_tree);
    auto subScope = scope;
    subScope.push_back({child, blocksInScope});
    validateBundleIndeces(child, errors, subScope);
  }
}

void CodeValidator::validateBundleSizes(ASTNode node,
                                        std::vector<LangError> &errors,
                                        ScopeStack scope) {
  if (node->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> declaration =
        std::static_pointer_cast<DeclarationNode>(node);
    // FIXME this needs to be rewritten looking at the port block size
    int size =
        ASTQuery::getBlockDeclaredSize(declaration, scope, m_tree, &errors);
    int datasize = getBlockDataSize(declaration, scope, &errors);
    if (size != datasize && datasize > 1) {
      LangError error;
      error.type = LangError::BundleSizeMismatch;
      error.lineNumber = node->getLine();
      error.errorTokens.push_back(declaration->getBundle()->getName());
      error.errorTokens.push_back(std::to_string(size));
      error.errorTokens.push_back(std::to_string(datasize));
      errors.push_back(error);
    }

    // TODO : use this pass to store the computed value of constant int?
    //    auto decl = static_pointer_cast<DeclarationNode>(node);
    //    auto subScope = getBlockSubScope(decl);
    //    if (subScope) {
    //      scope.push_back({decl->getName(), subScope->getChildren()});
    //    }
  }

  std::vector<ASTNode> children = node->getChildren();
  for (ASTNode node : children) {
    validateBundleSizes(node, errors, scope);
  }
}

void CodeValidator::validateSymbolUniqueness(ScopeStack scope,
                                             std::vector<LangError> &errors) {
  auto singleScope = scope.back();
  while (singleScope.second.size() > 0) {
    auto node = singleScope.second.front();
    singleScope.second.erase(singleScope.second.begin());
    for (ASTNode sibling : singleScope.second) {
      std::string nodeName, siblingName;
      if (sibling->getNodeType() == AST::Declaration ||
          sibling->getNodeType() == AST::BundleDeclaration) {
        siblingName = static_cast<DeclarationNode *>(sibling.get())->getName();
      }
      if (node->getNodeType() == AST::Declaration ||
          node->getNodeType() == AST::BundleDeclaration) {
        nodeName = static_cast<DeclarationNode *>(node.get())->getName();
      }
      if (nodeName.size() > 0 && nodeName == siblingName) {
        // Check if framework matches
        auto nodeFrameworkNode = node->getCompilerProperty("framework");
        auto siblingFrameworkNode = sibling->getCompilerProperty("framework");
        if (nodeFrameworkNode &&
            nodeFrameworkNode->getNodeType() == AST::String &&
            siblingFrameworkNode &&
            siblingFrameworkNode->getNodeType() == AST::String) {
          auto nodeFramework =
              std::static_pointer_cast<ValueNode>(nodeFrameworkNode)
                  ->getStringValue();
          auto siblingFramework =
              std::static_pointer_cast<ValueNode>(siblingFrameworkNode)
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
                  std::static_pointer_cast<ValueNode>(nodeAt)->getIntValue();
              auto siblingAtInt =
                  std::static_pointer_cast<ValueNode>(siblingAt)->getIntValue();
              if (nodeAtInt == siblingAtInt) {
                atMatches = true;
              }
            } else if (nodeAt->getNodeType() == AST::String &&
                       siblingAt->getNodeType() == AST::String) {
              auto nodeAtString =
                  std::static_pointer_cast<ValueNode>(nodeAt)->getStringValue();
              auto siblingAtString =
                  std::static_pointer_cast<ValueNode>(siblingAt)
                      ->getStringValue();
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
            error.errorTokens.push_back(nodeName);
            error.errorTokens.push_back(node->getFilename());
            error.errorTokens.push_back(std::to_string(node->getLine()));
            errors.push_back(error);
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
          validateSymbolUniqueness(scope, errors);
          scope.pop_back();
        }
      }
    }
  }
}

void CodeValidator::validateStreamSizes(ASTNode tree,
                                        std::vector<LangError> &errors,
                                        ScopeStack scope) {
  for (ASTNode node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      StreamNode *stream = static_cast<StreamNode *>(node.get());
      validateStreamInputSize(stream, errors, scope);
    } else if (node->getNodeType() == AST::Declaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
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
              validateStreamInputSize(stream.get(), errors, subScope);
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
  for (const ASTNode &node : tree->getChildren()) {
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
      stream = std::static_pointer_cast<StreamNode>(stream->getRight());
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
  auto declaration = ASTQuery::findDeclarationByName(
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
    if (constraintsNode->getNodeType() == AST::Stream) {

      validateConstraintStream(
          std::static_pointer_cast<StreamNode>(constraintsNode), function,
          declaration, scopeStack, tree);
    } else if (constraintsNode->getNodeType() == AST::List) {
      for (const auto &constraint : constraintsNode->getChildren()) {
        if (constraint->getNodeType() == AST::Stream) {
          validateConstraintStream(
              std::static_pointer_cast<StreamNode>(constraint), function,
              declaration, scopeStack, tree);
        } else {
          std::cerr
              << "WARNING: Unexpected type in constraint, expecting stream"
              << std::endl;
        }
      }
    } else {
      std::cerr << __FILE__ << ":" << __LINE__
                << " constraints must be streams or list of streams"
                << std::endl;
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
        stream = std::static_pointer_cast<StreamNode>(stream->getRight());
        node = stream->getLeft();
      } else {
        stream = nullptr;
      }
    }
  }
}

std::vector<ASTNode> CodeValidator::resolveConstraintNode(
    ASTNode node, std::vector<ASTNode> previous,
    std::shared_ptr<FunctionNode> function,
    std::shared_ptr<DeclarationNode> declaration, ScopeStack scopeStack,
    ASTNode tree) {
  if (node->getNodeType() == AST::PortProperty) {
    auto pp = std::static_pointer_cast<PortPropertyNode>(node);
    if (pp->getPortName() == "size") {
      return {std::make_shared<ValueNode>(
          (int64_t)CodeAnalysis::evaluateSizePortProperty(
              pp->getName(), scopeStack, declaration, function, tree),
          __FILE__, __LINE__)};
    }
    if (pp->getPortName() == "rate") {
      return {std::make_shared<ValueNode>(
          CodeAnalysis::evaluateRatePortProperty(pp->getName(), scopeStack,
                                                 declaration, function, tree),
          __FILE__, __LINE__)};
    } else {
      return {std::make_shared<AST>()};
    }

  } else if (node->getNodeType() == AST::Function) {
    // FIXME the output blocks must be determined from the function.
    // Currently providing only a switch block for output as the only
    // implemented functions are
    StrideRuntimeStatus status;
    std::vector<ASTNode> output;
    output.emplace_back(std::make_shared<ValueNode>(true, __FILE__, __LINE__));
    auto functionInstance = std::static_pointer_cast<FunctionNode>(node);
    ASTRuntime::resolveFunction(functionInstance, previous, output, status);
    if (!status.ok) {
      auto &err = status.err;
      err.filename = functionInstance->getFilename();
      err.lineNumber = functionInstance->getLine();
      err.errorTokens[0] = functionInstance->getName();
      m_errors.push_back(err);
    }
    return output;
  } else if (node->getNodeType() == AST::Block) {

  } else if (node->getNodeType() == AST::List) {
    std::vector<ASTNode> resolvedList;
    size_t i = 0;
    for (const auto &elem : node->getChildren()) {
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
                                            std::vector<LangError> &errors,
                                            ScopeStack scope) {
  ASTNode left = stream->getLeft();
  ASTNode right = stream->getRight();

  int leftOutSize =
      CodeAnalysis::getNodeNumOutputs(left, scope, m_tree, &errors);
  int rightInSize =
      CodeAnalysis::getNodeNumInputs(right, scope, m_tree, &errors);

  auto leftDecl = ASTQuery::findDeclarationByName(ASTQuery::getNodeName(left),
                                                  scope, m_tree);
  std::shared_ptr<DeclarationNode> rightDecl;
  if (right->getNodeType() == AST::Stream) {
    auto nextStreamMember =
        std::static_pointer_cast<StreamNode>(right)->getLeft();
    rightDecl = ASTQuery::findDeclarationByName(
        ASTQuery::getNodeName(nextStreamMember), scope, m_tree);
  } else {
    rightDecl = ASTQuery::findDeclarationByName(ASTQuery::getNodeName(right),
                                                scope, m_tree);
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
        error.errorTokens.push_back(std::to_string(leftOutSize));
        error.errorTokens.push_back(ASTQuery::getNodeName(left));
        error.errorTokens.push_back(std::to_string(rightInSize));
        error.filename = left->getFilename();
        errors.push_back(error);
      }
    }
  }

  if (right->getNodeType() == AST::Stream) {
    validateStreamInputSize(static_cast<StreamNode *>(right.get()), errors,
                            scope);
  }
}

int CodeValidator::getBlockDataSize(
    std::shared_ptr<DeclarationNode> declaration, ScopeStack scope,
    std::vector<LangError> *errors) {
  std::vector<std::shared_ptr<PropertyNode>> ports =
      declaration->getProperties();
  if (ports.size() == 0) {
    return 0;
  }
  int size = CodeAnalysis::getNodeNumOutputs(ports.at(0)->getValue(), scope,
                                             m_tree, errors);
  for (std::shared_ptr<PropertyNode> port : ports) {
    ASTNode value = port->getValue();
    int newSize = CodeAnalysis::getNodeNumOutputs(value, scope, m_tree, errors);
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
