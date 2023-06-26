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

#include "stride/parser/ast.h"
#include "stride/parser/blocknode.h"
#include "stride/parser/bundlenode.h"
#include "stride/parser/declarationnode.h"
#include "stride/parser/expressionnode.h"
#include "stride/parser/propertynode.h"
#include "stride/parser/rangenode.h"
#include "stride/parser/streamnode.h"
#include "stride/parser/valuenode.h"

#include "stridesystem.hpp"

#include "systemconfiguration.hpp"

// Resolves all code and creates a StrideSystem
class CodeResolver {
public:
  CodeResolver(ASTNode tree, std::string striderootDir,
               SystemConfiguration systemConfig = SystemConfiguration());
  ~CodeResolver();

  void process();

  std::shared_ptr<StrideSystem> getSystem() { return m_system; }

  static std::shared_ptr<DeclarationNode>
  createSignalDeclaration(std::string name, int size, ScopeStack scope,
                          ASTNode tree);

private:
  // Main processing functions
  void processSystem();
  void enableTesting();
  void declareModuleInternalBlocks();
  void resolveStreamSymbols();
  void processDeclarations();
  void expandParallel();
  void resolveConstants();
  void processDomains();
  void resolveRates();
  void analyzeConnections();
  void resolveTypeCasting();
  void storeDeclarations();
  void analyzeParents();

  void printTree();

  // Sub functions
  void resolveStreamRatesReverse(std::shared_ptr<StreamNode> stream);
  void resolveStreamRates(std::shared_ptr<StreamNode> stream);
  void expandParallelStream(std::shared_ptr<StreamNode> stream,
                            ScopeStack scopeStack, ASTNode tree);

  void expandStreamToSizes(std::shared_ptr<StreamNode> stream,
                           std::vector<int> &size, int previousOutSize,
                           ScopeStack scopeStack);
  ASTNode expandFunctionFromProperties(std::shared_ptr<FunctionNode> func,
                                       ScopeStack scope, ASTNode tree);

  void analyzeChildConnections(ASTNode node,
                               ScopeStack scopeStack = ScopeStack());

  void resolveDomainsForStream(std::shared_ptr<StreamNode> stream,
                               ScopeStack scopeStack,
                               ASTNode contextDomainNode);

  void setDomainForNode(ASTNode node, ASTNode domain, ScopeStack scopeStack,
                        ASTNode tree, bool force = false);
  ASTNode processDomainsForNode(ASTNode node, ScopeStack scopeStack,
                                std::vector<ASTNode> &domainStack);
  void setDomainForStack(std::vector<ASTNode> domainStack,
                         ASTNode resolvingInstance, ASTNode domainName,
                         ScopeStack scopeStack);

  std::vector<ASTNode> declareUnknownName(std::shared_ptr<BlockNode> block,
                                          int size, ScopeStack localScope,
                                          ASTNode tree);
  std::vector<ASTNode> declareUnknownBundle(std::shared_ptr<BundleNode> name,
                                            int size, ScopeStack localScope,
                                            ASTNode tree);
  std::shared_ptr<DeclarationNode> createConstantDeclaration(std::string name,
                                                             ASTNode value);
  void declareIfMissing(std::string name, ASTNode blockList, ASTNode value);

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

  void declareInternalBlocksForNode(ASTNode node, ScopeStack scope);

  void propagateDomainsForNode(ASTNode node, ScopeStack scopeStack);
  void resolveDomainForStreamNode(ASTNode node, ScopeStack scope);

  void remapStreamDomains(std::shared_ptr<StreamNode> stream,
                          std::map<std::string, std::string> domainMap,
                          ScopeStack scopeStack, ASTNode tree);

  ASTNode resolvePortProperty(std::shared_ptr<PortPropertyNode> portProperty,
                              ScopeStack scopeStack);

  void setNodeRate(ASTNode node, double rate, ScopeStack scope, ASTNode tree);

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

  void resolveTypeCastForNode(ASTNode node, ScopeStack scopeStack,
                              ASTNode tree);
  void resolveTypeCastForStream(std::shared_ptr<StreamNode> stream,
                                ScopeStack scopeStack, ASTNode tree);
  void resolveTypeCastForDeclaration(std::shared_ptr<DeclarationNode> decl,
                                     ScopeStack scopeStack, ASTNode tree);

  void appendParent(std::shared_ptr<DeclarationNode> decl,
                    std::shared_ptr<DeclarationNode> parent);

  std::shared_ptr<StreamNode> makeStreamFromStack(std::vector<ASTNode> &stack);
  std::vector<ASTNode> sliceStreamByDomain(std::shared_ptr<StreamNode> stream,
                                           ScopeStack scopeStack);
  void sliceDomainsInNode(std::shared_ptr<DeclarationNode> stream,
                          ScopeStack scopeStack);

  ASTNode getModuleContextDomain(std::shared_ptr<DeclarationNode> moduleDecl);

  std::vector<ASTNode> mContextDomainStack;

  std::shared_ptr<StrideSystem> m_system;
  SystemConfiguration m_systemConfig;

  ASTNode m_tree;
};

#endif // CODERESOLVER_H
