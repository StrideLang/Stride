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
#include <QMap>
#include <QVariant>

#include "strideparser.h"
#include "porttypes.h"
#include "stridesystem.hpp"
#include "systemconfiguration.hpp"

class CodeValidator
{
public:

    typedef enum {
        NO_OPTIONS = 0x00,
        NO_RATE_VALIDATION = 0x01,
        USE_TESTING = 0x02,
    } Options;

    CodeValidator(QString striderootDir, ASTNode tree = nullptr, Options options = NO_OPTIONS,
                  SystemConfiguration systemConfig = SystemConfiguration());
    ~CodeValidator();

    bool isValid();
    bool platformIsValid();

    QList<LangError> getErrors();
    QStringList getPlatformErrors();

    std::shared_ptr<StrideSystem> getSystem();

    static std::shared_ptr<DeclarationNode> findDeclaration(QString streamMemberName, const QVector<ASTNode> &scopeStack, ASTNode tree,
                                            vector<string> scope = vector<string>(), vector<string> defaultNamespaces = vector<string>());
    static QString streamMemberName(ASTNode  node, QVector<ASTNode > scopeStack, ASTNode tree);
    static PortType resolveBundleType(BundleNode *bundle, QVector<ASTNode > scope, ASTNode tree);
    static PortType resolveNameType(BlockNode *name, QVector<ASTNode > scope, ASTNode tree);
    static PortType resolveNodeOutType(ASTNode node, QVector<ASTNode > scope, ASTNode tree);
    static PortType resolveListType(ListNode *listnode, QVector<ASTNode > scope, ASTNode tree);
    static PortType resolveExpressionType(ExpressionNode *exprnode, QVector<ASTNode > scope, ASTNode tree);
    static PortType resolveRangeType(RangeNode *rangenode, QVector<ASTNode > scope, ASTNode tree);
    static PortType resolvePortPropertyType(PortPropertyNode *portproperty, QVector<ASTNode > scope, ASTNode tree);

    static int evaluateConstInteger(ASTNode node, QVector<ASTNode> scope, ASTNode tree, QList<LangError> &errors);
    static double evaluateConstReal(ASTNode node, QVector<ASTNode > scope, ASTNode tree, QList<LangError> &errors);
    static std::string evaluateConstString(ASTNode node, QVector<ASTNode > scope, ASTNode tree, QList<LangError> &errors);
    static ASTNode getMemberfromBlockBundle(DeclarationNode *block, int index, QList<LangError> &errors);
    static ASTNode getValueFromConstBlock(DeclarationNode *block);
    static ASTNode getMemberFromList(ListNode *node, int index, QList<LangError> &errors);
    static std::shared_ptr<PropertyNode> findPropertyByName(vector<std::shared_ptr<PropertyNode> > properties, QString propertyName);
    static QVector<ASTNode > validTypesForPort(std::shared_ptr<DeclarationNode> typeDeclaration, QString portName, QVector<ASTNode > scope, ASTNode tree);
    static std::shared_ptr<DeclarationNode> findTypeDeclarationByName(string typeName, QVector<ASTNode > scopeStack, ASTNode tree,
                                                      std::vector<std::string> namespaces = std::vector<std::string>());
    static std::shared_ptr<DeclarationNode> findTypeDeclaration(std::shared_ptr<DeclarationNode> block, QVector<ASTNode> scope, ASTNode tree);
    static std::shared_ptr<DeclarationNode> findDomainDeclaration(string domainName, ASTNode tree);

    static QVector<ASTNode> getPortsForTypeBlock(std::shared_ptr<DeclarationNode> block, QVector<ASTNode> scope, ASTNode tree);
    static QVector<ASTNode> getPortsForType(std::string typeName, QVector<ASTNode> scope, ASTNode tree, std::vector<string> namespaces);
    static QVector<ASTNode > getInheritedPorts(std::shared_ptr<DeclarationNode> block, QVector<ASTNode > scope, ASTNode tree);
    static vector<std::shared_ptr<DeclarationNode> > getInheritedTypes(std::shared_ptr<DeclarationNode> typeDeclaration, QVector<ASTNode > scope, ASTNode tree);

    static std::shared_ptr<DeclarationNode> getMainOutputPortBlock(std::shared_ptr<DeclarationNode> moduleBlock);
    static std::shared_ptr<DeclarationNode> getMainInputPortBlock(std::shared_ptr<DeclarationNode> moduleBlock);

    /// Number of parallel streams that a single stream can be broken up into
    static int numParallelStreams(StreamNode *stream, StrideSystem &platform, QVector<ASTNode > &scope, ASTNode tree, QList<LangError> &errors);

    /// Get the number of parallel nodes implicit in node. i.e. into how many parallel streams
    /// can the node be broken up.
    static int getNodeSize(ASTNode node, const QVector<ASTNode> &scopeStack, ASTNode tree);

    static std::vector<std::string> getModulePropertyNames(std::shared_ptr<DeclarationNode> blockDeclaration);
    static int getFunctionDataSize(std::shared_ptr<FunctionNode> func, QVector<ASTNode > scope, ASTNode  tree, QList<LangError> &errors);
    static int getNodeNumOutputs(ASTNode node, const QVector<ASTNode> &scope, ASTNode tree, QList<LangError> &errors);    bool validateNodeDeclaration(QString name, QVector<ASTNode > scopeStack, ASTNode tree);
    static int getNodeNumInputs(ASTNode node, const QVector<ASTNode > &scope, ASTNode tree, QList<LangError> &errors);
    static int getTypeNumOutputs(std::shared_ptr<DeclarationNode> blockDeclaration, const QVector<ASTNode > &scope, ASTNode tree, QList<LangError> &errors);
    static int getTypeNumInputs(std::shared_ptr<DeclarationNode> blockDeclaration, const QVector<ASTNode > &scope, ASTNode tree, QList<LangError> &errors);

    static int getBlockDeclaredSize(std::shared_ptr<DeclarationNode> block, QVector<ASTNode > scope, ASTNode tree, QList<LangError> &errors);

    static int getLargestPropertySize(vector<std::shared_ptr<PropertyNode >> &properties, QVector<ASTNode > scope, ASTNode tree, QList<LangError> &errors);

    static ASTNode getBlockSubScope(std::shared_ptr<DeclarationNode> block);
    static int getBundleSize(BundleNode *bundle, QVector<ASTNode> scope, ASTNode tree, QList<LangError> &errors);

    static QString getPortTypeName(PortType type);

    static ASTNode getNodeDomain(ASTNode node, QVector<ASTNode > scopeStack, ASTNode tree);
    static std::string getNodeDomainName(ASTNode node, QVector<ASTNode > scopeStack, ASTNode tree);
    static std::string getDomainNodeString(ASTNode node);
    static QVector<ASTNode > getBlocksInScope(ASTNode root, QVector<ASTNode > scopeStack, ASTNode tree);

    static std::vector<std::string> getUsedDomains(ASTNode tree);
    static std::string getFrameworkForDomain(std::string domainName, ASTNode tree);

    static double findRateInProperties(vector<std::shared_ptr<PropertyNode>> properties, QVector<ASTNode > scope, ASTNode tree);
    static double getNodeRate(ASTNode node,  QVector<ASTNode> scope = QVector<ASTNode >(), ASTNode tree = nullptr);
    static void setNodeRate(ASTNode node, double rate,  QVector<ASTNode> scope = QVector<ASTNode >(), ASTNode tree = nullptr);

    static double getDefaultForTypeAsDouble(string type, string port, QVector<ASTNode > scope, ASTNode tree, std::vector<string> namespaces);
    static ASTNode getDefaultPortValueForType(string type, string portName, QVector<ASTNode > scope, ASTNode tree, std::vector<string> namespaces);

    static bool namespaceMatch(std::vector<string> scopeList, std::shared_ptr<DeclarationNode> decl);

    static vector<StreamNode *> getStreamsAtLine(ASTNode tree, int line);

    ASTNode getTree() const;
    void setTree(const ASTNode &tree);

private:

    void validateTree(QString platformRootDir, ASTNode tree);
    void validate();
    QVector<std::shared_ptr<SystemNode> > getPlatformNodes();

    void validatePlatform(ASTNode node, QVector<ASTNode > scopeStack);
    void validateTypes(ASTNode node, QVector<ASTNode > scopeStack, vector<string> parentNamespace = vector<string>());
    void validateStreamMembers(StreamNode *node, QVector<ASTNode > scopeStack);
    void validateBundleIndeces(ASTNode node, QVector<ASTNode > scope);
    void validateBundleSizes(ASTNode node, QVector<ASTNode > scope);
    void validateSymbolUniqueness(QVector<ASTNode > scope);
    void validateListTypeConsistency(ASTNode node, QVector<ASTNode > scope);
    void validateStreamSizes(ASTNode tree, QVector<ASTNode > scope);
    void validateRates(ASTNode tree);

    void sortErrors();

    void validateStreamInputSize(StreamNode *stream, QVector<ASTNode > scope, QList<LangError> &errors);
    void validateNodeRate(ASTNode node, ASTNode tree);

    int getBlockDataSize(std::shared_ptr<DeclarationNode> block, QVector<ASTNode > scope, QList<LangError> &errors);

    QString getNodeText(ASTNode node);

    std::shared_ptr<StrideSystem> m_system;
    ASTNode m_tree;
    QList<LangError> m_errors;
    Options m_options;
    SystemConfiguration m_systemConfig;
};

#endif // CODEGEN_H
