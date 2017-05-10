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
    CodeResolver(StrideSystem * system, AST *tree);
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
    void resolveRates();
    void processDomains();
    void analyzeConnections();

    // Sub functions
    void resolveStreamRates(StreamNode *stream);
    void expandParallelStream(StreamNode *stream, QVector<AST *> scopeStack, AST *tree);

    void expandStreamToSizes(StreamNode *stream, QVector<int> &size, QVector<AST *> scopeStack);
    AST *expandFunctionFromProperties(FunctionNode *func, QVector<AST *> scope, AST *tree);
    void fillDefaultPropertiesForNode(AST *node);

    void insertBuiltinObjectsForNode(AST *node, map<string, vector<AST *> > &objects);

    void resolveDomainsForStream(const StreamNode *func, QVector<AST *> scopeStack, QString contextDomain = "");
    string processDomainsForNode(AST *node, QVector<AST *> scopeStack, QList<AST *> &domainStack);
    void setDomainForStack(QList<AST *> domainStack, string domainName,  QVector<AST *> scopeStack);
    DeclarationNode *createDomainDeclaration(QString name);
    DeclarationNode *createSignalDeclaration(QString name, int size = 1);
    std::vector<AST *> declareUnknownName(BlockNode *block, int size, QVector<AST *> localScope, AST *tree);
    std::vector<AST *> declareUnknownBundle(BundleNode *name, int size, QVector<AST *> localScope, AST *tree);
    DeclarationNode *createConstantDeclaration(string name, AST *value);
    void declareIfMissing(string name, AST *blockList, AST *value);
    DeclarationNode *createSignalBridge(string name, AST *defaultValue, AST *inDomain, AST * outDomain, const string filename, int line, int size = 1);

    std::vector<AST *> declareUnknownExpressionSymbols(ExpressionNode *expr, int size, QVector<AST *> scopeStack, AST * tree);
    std::vector<AST *> declareUnknownFunctionSymbols(FunctionNode *func, QVector<AST *> scopeStack, AST * tree);
    ListNode *expandNameToList(BlockNode *name, int size);
    void expandNamesToBundles(StreamNode *stream, AST *tree);
    std::vector<AST *> declareUnknownStreamSymbols(const StreamNode *stream, AST *previousStreamMember, QVector<AST *> localScope, AST *tree);
    std::vector<const AST *> getModuleStreams(DeclarationNode *module);

    ValueNode *reduceConstExpression(ExpressionNode *expr, QVector<AST *> scope, AST *tree);
    ValueNode *resolveConstant(AST *value, QVector<AST *> scope);
    void resolveConstantsInNode(AST *node, QVector<AST *> scope);
    void resolveDomainForStreamNode(AST *node, QVector<AST *> scope);

    void checkStreamConnections(const StreamNode *stream, QVector<AST *> scopeStack, bool start = true);
    void markConnectionForNode(AST *node, QVector<AST *> scopeStack, bool start);

    // Operators
    ValueNode *multiply(ValueNode *left, ValueNode *right);
    ValueNode *divide(ValueNode *left, ValueNode *right);
    ValueNode *add(ValueNode *left, ValueNode *right);
    ValueNode *subtract(ValueNode *left, ValueNode *right);
    ValueNode *unaryMinus(ValueNode *value);
    ValueNode *logicalAnd(ValueNode *left, ValueNode *right);
    ValueNode *logicalOr(ValueNode *left, ValueNode *right);
    ValueNode *logicalNot(ValueNode *left);

    AST * makeConnector(AST * node, string connectorName, int size, const QVector<AST *> &scopeStack);
    QVector<AST *> sliceStreamByDomain(StreamNode *stream, QVector<AST *> scopeStack);
    QVector<AST *> processExpression(ExpressionNode *expr, QVector<AST *> scopeStack, AST *outDOmain);

    StrideSystem * m_system;
    AST *m_tree;
    int m_connectorCounter;
};

#endif // CODERESOLVER_H
