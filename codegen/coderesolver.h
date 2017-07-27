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


#include <QVector>
#include <QSharedPointer>

#include "stridesystem.hpp"

#include "ast.h"
#include "streamnode.h"
#include "bundlenode.h"
#include "expressionnode.h"
#include "propertynode.h"
#include "declarationnode.h"
#include "blocknode.h"
#include "rangenode.h"
#include "valuenode.h"

class CodeResolver
{
public:
    CodeResolver(StrideSystem * system, ASTNode tree);
    ~CodeResolver();

    void preProcess();

private:
    // Main processing functions
    void processSystem();
    void insertBuiltinObjects();
    void fillDefaultProperties();
    void declareModuleInternalBlocks();
    void resolveStreamSymbols();
    void expandParallel();
    void resolveConstants();
    void processResets();
    void resolveRates();
    void processDomains();
    void analyzeConnections();

    // Sub functions
    void resolveStreamRatesReverse(std::shared_ptr<StreamNode> stream);
    void resolveStreamRates(std::shared_ptr<StreamNode> stream);
    void expandParallelStream(std::shared_ptr<StreamNode> stream, QVector<ASTNode> scopeStack, ASTNode tree);

    void expandStreamToSizes(std::shared_ptr<StreamNode> stream, QVector<int> &size, int previousOutSize, QVector<ASTNode > scopeStack);
    ASTNode expandFunctionFromProperties(std::shared_ptr<FunctionNode> func, QVector<ASTNode > scope, ASTNode tree);
    void fillDefaultPropertiesForNode(ASTNode node);

    void insertBuiltinObjectsForNode(ASTNode node, map<string, vector<ASTNode> > &objects);

    void resolveDomainsForStream(std::shared_ptr<StreamNode> func, QVector<ASTNode > scopeStack, QString contextDomain = "");
    string processDomainsForNode(ASTNode node, QVector<ASTNode > scopeStack, QList<ASTNode > &domainStack);
    void setDomainForStack(QList<ASTNode > domainStack, string domainName,  QVector<ASTNode > scopeStack);
    std::shared_ptr<DeclarationNode> createDomainDeclaration(QString name);
    std::shared_ptr<DeclarationNode> createSignalDeclaration(QString name, int size = 1);
    std::vector<ASTNode> declareUnknownName(std::shared_ptr<BlockNode> block, int size, QVector<ASTNode> localScope, ASTNode tree);
    std::vector<ASTNode> declareUnknownBundle(std::shared_ptr<BundleNode> name, int size, QVector<ASTNode > localScope, ASTNode tree);
    std::shared_ptr<DeclarationNode> createConstantDeclaration(string name, ASTNode value);
    void declareIfMissing(string name, ASTNode blockList, ASTNode value);
    std::shared_ptr<DeclarationNode> createSignalBridge(string bridgeName, string originalName, ASTNode defaultValue, ASTNode inDomain, ASTNode  outDomain, const string filename, int line, int size = 1);

    std::vector<ASTNode > declareUnknownExpressionSymbols(std::shared_ptr<ExpressionNode> expr, int size, QVector<ASTNode > scopeStack, ASTNode  tree);
    std::vector<ASTNode > declareUnknownFunctionSymbols(std::shared_ptr<FunctionNode> func, QVector<ASTNode > scopeStack, ASTNode  tree);
    std::shared_ptr<ListNode> expandNameToList(BlockNode *name, int size);
    void expandNamesToBundles(std::shared_ptr<StreamNode> stream, ASTNode tree);
    std::vector<ASTNode > declareUnknownStreamSymbols(std::shared_ptr<StreamNode> stream, ASTNode previousStreamMember, QVector<ASTNode > localScope, ASTNode tree);
    std::vector<ASTNode > getModuleStreams(std::shared_ptr<DeclarationNode> module);
    std::vector<ASTNode > getModuleBlocks(std::shared_ptr<DeclarationNode> module);
    ASTNode getSignalReset(std::shared_ptr<DeclarationNode> signal);

    void declareInternalBlocksForNode(ASTNode node);

    std::shared_ptr<ValueNode> reduceConstExpression(std::shared_ptr<ExpressionNode> expr, QVector<ASTNode > scope, ASTNode tree);
    std::shared_ptr<ValueNode> resolveConstant(ASTNode value, QVector<ASTNode > scope);
    void resolveConstantsInNode(ASTNode node, QVector<ASTNode > scope);
    void processResetForNode(ASTNode thisScope, ASTNode streamScope, ASTNode upperScope);
    void propagateDomainsForNode(ASTNode node, QVector<ASTNode > scopeStack);
    void resolveDomainForStreamNode(ASTNode node, QVector<ASTNode > scope);

    void checkStreamConnections(std::shared_ptr<StreamNode> stream, QVector<ASTNode > scopeStack, bool start = true);
    void markConnectionForNode(ASTNode node, QVector<ASTNode > scopeStack, bool start);

    // Operators
    std::shared_ptr<ValueNode>  multiply(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right);
    std::shared_ptr<ValueNode>  divide(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right);
    std::shared_ptr<ValueNode>  add(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right);
    std::shared_ptr<ValueNode>  subtract(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right);
    std::shared_ptr<ValueNode>  unaryMinus(std::shared_ptr<ValueNode>  value);
    std::shared_ptr<ValueNode>  logicalAnd(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right);
    std::shared_ptr<ValueNode>  logicalOr(std::shared_ptr<ValueNode>  left, std::shared_ptr<ValueNode>  right);
    std::shared_ptr<ValueNode>  logicalNot(std::shared_ptr<ValueNode>  left);

    ASTNode  makeConnector(ASTNode  node, string connectorName, int size, const QVector<ASTNode > &scopeStack);
    QVector<ASTNode > sliceStreamByDomain(std::shared_ptr<StreamNode> stream, QVector<ASTNode > scopeStack);
    void sliceDomainsInNode(std::shared_ptr<DeclarationNode> stream, QVector<ASTNode > scopeStack);
    QVector<ASTNode > processExpression(std::shared_ptr<ExpressionNode> expr, QVector<ASTNode > scopeStack, ASTNode outDOmain);

    StrideSystem * m_system;
    ASTNode m_tree;
    int m_connectorCounter;
    std::vector<std::vector<string>> m_bridgeAliases; //< 1: bridge signal 2: original name 3: domain
};

#endif // CODERESOLVER_H
