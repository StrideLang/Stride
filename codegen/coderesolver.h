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

#ifndef CODERESOLVER_H
#define CODERESOLVER_H

#include <QSharedPointer>
#include <QVector>

#include "stridesystem.hpp"

#include "ast.h"
#include "blocknode.h"
#include "bundlenode.h"
#include "declarationnode.h"
#include "expressionnode.h"
#include "propertynode.h"
#include "rangenode.h"
#include "streamnode.h"
#include "systemconfiguration.hpp"
#include "valuenode.h"

typedef std::vector<std::pair<ASTNode, std::vector<ASTNode>>> ScopeStack;

class CodeResolver {
public:
  CodeResolver(ASTNode tree, QString striderootDir,
               SystemConfiguration systemConfig = SystemConfiguration());
  ~CodeResolver();

  void process();

  std::shared_ptr<StrideSystem> getSystem() { return m_system; }

  //  bool platformIsValid() {
  //      return m_system->getErrors().size() == 0;
  //  }

  std::vector<std::pair<std::string, std::string>> m_domainChanges;

  static void fillDefaultPropertiesForNode(ASTNode node,
                                           std::vector<ASTNode> tree);
  static void insertBuiltinObjectsForNode(ASTNode node,
                                          map<string, vector<ASTNode>> &objects,
                                          ASTNode tree,
                                          string currentFramework = "");
  static std::shared_ptr<DeclarationNode>
  createSignalDeclaration(QString name, int size, ScopeStack scope,
                          ASTNode tree);

  static void
  insertDependentTypes(std::shared_ptr<DeclarationNode> typeDeclaration,
                       map<string, vector<ASTNode>> &objects, ASTNode tree);
  static std::shared_ptr<ValueNode>
  reduceConstExpression(std::shared_ptr<ExpressionNode> expr, ScopeStack scope,
                        ASTNode tree);
  static std::shared_ptr<ValueNode> resolveConstant(ASTNode value,
                                                    ScopeStack scope,
                                                    ASTNode tree,
                                                    string framework = "");
  static void resolveConstantsInNode(ASTNode node, ScopeStack scope,
                                     ASTNode tree,
                                     string currentFramework = "");

private:
  // Main processing functions
  void processSystem();
  void insertBuiltinObjects();
  void fillDefaultProperties();
  void enableTesting();
  void declareModuleInternalBlocks();
  void resolveStreamSymbols();
  void processDeclarations();
  void expandParallel();
  void processAnoymousDeclarations();
  void resolveConstants();
  void processDomains();
  void resolveRates();
  void analyzeConnections();
  void storeDeclarations();
  void analyzeParents();

  void printTree();

  // Sub functions
  void resolveStreamRatesReverse(std::shared_ptr<StreamNode> stream);
  void resolveStreamRates(std::shared_ptr<StreamNode> stream);
  void expandParallelStream(std::shared_ptr<StreamNode> stream,
                            ScopeStack scopeStack, ASTNode tree);

  void expandStreamToSizes(std::shared_ptr<StreamNode> stream,
                           QVector<int> &size, int previousOutSize,
                           ScopeStack scopeStack);
  ASTNode expandFunctionFromProperties(std::shared_ptr<FunctionNode> func,
                                       ScopeStack scope, ASTNode tree);

  void analyzeChildConnections(ASTNode node,
                               ScopeStack scopeStack = ScopeStack());

  void resolveDomainsForStream(std::shared_ptr<StreamNode> stream,
                               ScopeStack scopeStack,
                               ASTNode contextDomainNode);
  ASTNode processDomainsForNode(ASTNode node, ScopeStack scopeStack,
                                QList<ASTNode> &domainStack);
  void setDomainForStack(QList<ASTNode> domainStack, ASTNode resolvingInstance,
                         ASTNode domainName, ScopeStack scopeStack);
  std::shared_ptr<DeclarationNode> createDomainDeclaration(QString name);
  std::vector<ASTNode> declareUnknownName(std::shared_ptr<BlockNode> block,
                                          int size, ScopeStack localScope,
                                          ASTNode tree);
  std::vector<ASTNode> declareUnknownBundle(std::shared_ptr<BundleNode> name,
                                            int size, ScopeStack localScope,
                                            ASTNode tree);
  std::shared_ptr<DeclarationNode> createConstantDeclaration(string name,
                                                             ASTNode value);
  void declareIfMissing(string name, ASTNode blockList, ASTNode value);
  //  std::shared_ptr<DeclarationNode>
  //  createSignalBridge(string bridgeName, string originalName,
  //                     ASTNode defaultValue, ASTNode inDomain, ASTNode
  //                     outDomain, const string filename, int line, int size =
  //                     1, string type = "signal");

  std::vector<ASTNode>
  declareUnknownExpressionSymbols(std::shared_ptr<ExpressionNode> expr,
                                  int size, ScopeStack scopeStack,
                                  ASTNode tree);
  std::vector<ASTNode>
  declareUnknownFunctionSymbols(std::shared_ptr<FunctionNode> func,
                                ScopeStack scopeStack, ASTNode tree);
  std::shared_ptr<ListNode> expandNameToList(BlockNode *name, int size);
  void expandNamesToBundles(std::shared_ptr<StreamNode> stream, ASTNode tree);
  std::vector<ASTNode>
  declareUnknownStreamSymbols(std::shared_ptr<StreamNode> stream,
                              ASTNode previousStreamMember,
                              ScopeStack localScope, ASTNode tree);
  std::vector<ASTNode>
  getModuleStreams(std::shared_ptr<DeclarationNode> module);
  std::vector<ASTNode> getModuleBlocks(std::shared_ptr<DeclarationNode> module);

  void declareInternalBlocksForNode(ASTNode node, ScopeStack scope);

  void propagateDomainsForNode(ASTNode node, ScopeStack scopeStack);
  void resolveDomainForStreamNode(ASTNode node, ScopeStack scope);

  void remapStreamDomains(std::shared_ptr<StreamNode> stream,
                          std::map<std::string, std::string> domainMap,
                          ScopeStack scopeStack, ASTNode tree);

  ASTNode resolvePortProperty(std::shared_ptr<PortPropertyNode> portProperty,
                              ScopeStack scopeStack);

  void setInputBlockForFunction(std::shared_ptr<FunctionNode> func,
                                ScopeStack scopeStack, ASTNode previous);
  void setOutputBlockForFunction(std::shared_ptr<FunctionNode> func,
                                 ScopeStack scopeStack, ASTNode previous);
  void checkStreamConnections(std::shared_ptr<StreamNode> stream,
                              ScopeStack scopeStack,
                              ASTNode previous = nullptr);

  void markPreviousReads(ASTNode node, ASTNode previous, ScopeStack scopeStack);
  void markConnectionForNode(ASTNode node, ScopeStack scopeStack,
                             ASTNode previous = nullptr,
                             unsigned int listIndex = 0);

  void storeDeclarationsForNode(ASTNode node, ScopeStack scopeStack,
                                ASTNode tree);

  void appendParent(std::shared_ptr<DeclarationNode> decl,
                    std::shared_ptr<DeclarationNode> parent);

  // Operators
  static std::shared_ptr<ValueNode> multiply(std::shared_ptr<ValueNode> left,
                                             std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> divide(std::shared_ptr<ValueNode> left,
                                           std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> add(std::shared_ptr<ValueNode> left,
                                        std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> subtract(std::shared_ptr<ValueNode> left,
                                             std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode>
  unaryMinus(std::shared_ptr<ValueNode> value);
  static std::shared_ptr<ValueNode>
  logicalAnd(std::shared_ptr<ValueNode> left, std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> logicalOr(std::shared_ptr<ValueNode> left,
                                              std::shared_ptr<ValueNode> right);
  static std::shared_ptr<ValueNode> logicalNot(std::shared_ptr<ValueNode> left);

  //    ASTNode makeConnector(ASTNode node, string connectorName, int size,
  //    const ScopeStack &scopeStack); void terminateStackWithBridge(ASTNode
  //    node, ScopeStack &streams, ScopeStack &stack, ScopeStack &scopeStack);
  std::shared_ptr<StreamNode> makeStreamFromStack(std::vector<ASTNode> &stack);
  std::vector<ASTNode> sliceStreamByDomain(std::shared_ptr<StreamNode> stream,
                                           ScopeStack scopeStack);
  void sliceDomainsInNode(std::shared_ptr<DeclarationNode> stream,
                          ScopeStack scopeStack);
  //  std::vector<ASTNode> processExpression(std::shared_ptr<ExpressionNode>
  //  expr,
  //                                         ScopeStack scopeStack,
  //                                         ASTNode outDomain);

  ASTNode getModuleContextDomain(std::shared_ptr<DeclarationNode> moduleDecl);
  //    void setContextDomain(vector<ASTNode> nodes,
  //    std::shared_ptr<DeclarationNode> domainDeclaration); void
  //    setContextDomainForStreamNode(ASTNode node,
  //    std::shared_ptr<DeclarationNode> domainDeclaration);

  //    void populateContextDomains(vector<ASTNode> nodes);

  std::vector<ASTNode> mContextDomainStack;

  std::shared_ptr<StrideSystem> m_system;
  SystemConfiguration m_systemConfig;

  ASTNode m_tree;
  int m_connectorCounter;
  //    std::vector<std::vector<string>> m_bridgeAliases; //< 1: bridge signal
  //    2: original name 3: domain
};

#endif // CODERESOLVER_H
