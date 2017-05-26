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

#ifndef CODEGEN_H
#define CODEGEN_H

#include <QString>
#include <QVector>

#include "strideparser.h"
#include "porttypes.h"
#include "stridesystem.hpp"

class CodeValidator
{
public:

    typedef enum {
        NO_OPTIONS = 0x00,
        NO_RATE_VALIDATION = 0x01,
        USE_TESTING = 0x02,
    } Options;

    CodeValidator(QString striderootDir, AST * tree = nullptr, Options options = NO_OPTIONS);
    ~CodeValidator();

    bool isValid();
    bool platformIsValid();

    QList<LangError> getErrors();
    QStringList getPlatformErrors();

    StrideSystem *getSystem();

    static DeclarationNode *findDeclaration(QString streamMemberName, QVector<AST *> scopeStack, AST *tree,
                                            vector<string> scope = vector<string>(), vector<string> defaultNamespaces = vector<string>());
    static QString streamMemberName(AST * node, QVector<AST *> scopeStack, AST *tree);
    static PortType resolveBundleType(BundleNode *bundle, QVector<AST *> scope, AST *tree);
    static PortType resolveNameType(BlockNode *name, QVector<AST *> scope, AST *tree);
    static PortType resolveNodeOutType(AST *node, QVector<AST *> scope, AST *tree);
    static PortType resolveListType(ListNode *listnode, QVector<AST *> scope, AST *tree);
    static PortType resolveExpressionType(ExpressionNode *exprnode, QVector<AST *> scope, AST *tree);
    static PortType resolveRangeType(RangeNode *rangenode, QVector<AST *> scope, AST *tree);

    static int evaluateConstInteger(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors);
    static double evaluateConstReal(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors);
    static std::string evaluateConstString(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors);
    static AST *getMemberfromBlockBundle(DeclarationNode *block, int index, QList<LangError> &errors);
    static AST *getValueFromConstBlock(DeclarationNode *block);
    static AST *getMemberFromList(ListNode *node, int index, QList<LangError> &errors);
    static PropertyNode *findPropertyByName(vector<PropertyNode *> properties, QString propertyName);
    static QVector<AST *> validTypesForPort(DeclarationNode *typeDeclaration, QString portName, QVector<AST *> scope, AST *tree);
    static DeclarationNode *findTypeDeclarationByName(QString typeName, QVector<AST *> scopeStack, AST *tree,
                                                      QList<LangError> &errors,
                                                      std::vector<std::string> namespaces = std::vector<std::string>());
    static DeclarationNode *findTypeDeclaration(DeclarationNode *block, QVector<AST *> scope, AST *tree, QList<LangError> &errors);
    static DeclarationNode *findDomainDeclaration(string domainName, AST *tree);

    static QVector<AST *> getPortsForTypeBlock(DeclarationNode *block, QVector<AST *> scope, AST *tree);
    static QVector<AST *> getPortsForType(QString typeName, QVector<AST *> scope, AST *tree);
    static QVector<AST *> getInheritedPorts(DeclarationNode *block, QVector<AST *> scope, AST *tree);
    static QStringList getInheritedTypeNames(DeclarationNode *block, QVector<AST *> scope, AST *tree);

    static DeclarationNode *getMainOutputPortBlock(DeclarationNode *moduleBlock);
    static DeclarationNode *getMainInputPortBlock(DeclarationNode *moduleBlock);

    /// Number of parallel streams that a single stream can be broken up into
    static int numParallelStreams(StreamNode *stream, StrideSystem &platform, QVector<AST *> &scope, AST *tree, QList<LangError> &errors);

    /// Get the number of parallel nodes implicit in node. i.e. into how many parallel streams
    /// can the node be broken up.
    static int getNodeSize(AST *node, AST *tree);

    static int getFunctionDataSize(FunctionNode *func, QVector<AST *> scope, AST * tree, QList<LangError> &errors);
    static int getNodeNumOutputs(AST *node, const QVector<AST *> &scope, AST *tree, QList<LangError> &errors);    bool validateNodeDeclaration(QString name, QVector<AST *> scopeStack, AST *tree);
    static int getNodeNumInputs(AST *node, const QVector<AST *> &scope, AST *tree, QList<LangError> &errors);
    static int getTypeNumOutputs(DeclarationNode *blockDeclaration, const QVector<AST *> &scope, AST *tree, QList<LangError> &errors);
    static int getTypeNumInputs(DeclarationNode *blockDeclaration, const QVector<AST *> &scope, AST *tree, QList<LangError> &errors);

    static int getBlockDeclaredSize(DeclarationNode *block, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    static int getLargestPropertySize(vector<PropertyNode *> &properties, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    static AST *getBlockSubScope(DeclarationNode *block);
    static int getBundleSize(BundleNode *bundle, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    static QString getPortTypeName(PortType type);

    static AST* getNodeDomain(AST *node, QVector<AST *> scopeStack, AST *tree);
    static std::string getNodeDomainName(AST *node, QVector<AST *> scopeStack, AST *tree);
    static std::string getDomainNodeString(AST *node);
    static QVector<AST *> getBlocksInScope(AST *root, QVector<AST *> scopeStack, AST *tree);

    static std::vector<std::string> getUsedDomains(AST *tree);
    static std::string getFrameworkForDomain(std::string domainName, AST *tree);

    static double findRateInProperties(vector<PropertyNode *> properties, QVector<AST *> scope, AST *tree);
    static double getNodeRate(AST *node,  QVector<AST *> scope = QVector<AST *>(), AST *tree = nullptr);
    static void setNodeRate(AST *node, double rate,  QVector<AST *> scope = QVector<AST *>(), AST *tree = nullptr);

    static double getDefaultForTypeAsDouble(QString type, QString port, QVector<AST *> scope, AST *tree);
    static AST *getDefaultPortValueForType(QString type, QString portName, QVector<AST *> scope, AST *tree);

    static bool scopesMatch(QStringList scopeList, AST *node);
    static bool scopesMatch(AST *node1, AST *node2);
    static bool nodeInScope(std::vector<string> scopeList, AST *node);

    static vector<StreamNode *> getStreamsAtLine(AST *tree, int line);

private:

    void validateTree(QString platformRootDir, AST * tree);
    void validate();
    QVector<SystemNode *> getPlatformNodes();

    void validatePlatform(AST *node, QVector<AST *> scopeStack);
    void validateTypes(AST *node, QVector<AST *> scopeStack, vector<string> parentNamespace = vector<string>());
    void validateStreamMembers(StreamNode *node, QVector<AST *> scopeStack);
    void validateBundleIndeces(AST *node, QVector<AST *> scope);
    void validateBundleSizes(AST *node, QVector<AST *> scope);
    void validateSymbolUniqueness(AST *node, QVector<AST *> scope);
    void validateListTypeConsistency(AST *node, QVector<AST *> scope);
    void validateStreamSizes(AST *tree, QVector<AST *> scope);
    void validateRates(AST *tree);

    void sortErrors();

    void validateStreamInputSize(StreamNode *stream, QVector<AST *> scope, QList<LangError> &errors);
    void validateNodeRate(AST *node, AST *tree);

    int getBlockDataSize(DeclarationNode *block, QVector<AST *> scope, QList<LangError> &errors);

    QString getNodeText(AST *node);

    StrideSystem * m_system;
    AST *m_tree;
    QList<LangError> m_errors;
    Options m_options;
};

#endif // CODEGEN_H
