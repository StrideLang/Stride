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


typedef std::vector<std::pair<string, std::vector<ASTNode> > > ScopeStack;

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

    ASTNode getTree() const;
    void setTree(const ASTNode &tree);

    // Static functions -----------------------------
    static std::shared_ptr<DeclarationNode> findDeclaration(std::string streamMemberName, const ScopeStack &scopeStack, ASTNode tree,
                                            vector<string> scope = vector<string>(), vector<string> defaultNamespaces = vector<string>());
    static std::shared_ptr<DeclarationNode> findDeclaration(QString streamMemberName, const ScopeStack &scopeStack, ASTNode tree,
                                            vector<string> scope = vector<string>(), vector<string> defaultNamespaces = vector<string>());
    static std::shared_ptr<DeclarationNode> getDeclaration(ASTNode node); // Retrieve stored declaration

    static std::string streamMemberName(ASTNode  node);
    static PortType resolveBundleType(BundleNode *bundle, ScopeStack scopeStack, ASTNode tree);
    static PortType resolveNameType(BlockNode *name, ScopeStack scopeStack, ASTNode tree);
    static PortType resolveNodeOutType(ASTNode node, ScopeStack scopeStack, ASTNode tree);
    static PortType resolveListType(ListNode *listnode, ScopeStack scopeStack, ASTNode tree);
    static PortType resolveExpressionType(ExpressionNode *exprnode, ScopeStack scopeStack, ASTNode tree);
    static PortType resolveRangeType(RangeNode *rangenode, ScopeStack scopeStack, ASTNode tree);
    static PortType resolvePortPropertyType(PortPropertyNode *portproperty, ScopeStack scopeStack, ASTNode tree);
    static shared_ptr<DeclarationNode> resolveBlock(ASTNode node, bool downStream = true);

    static ASTNode resolveDomain(ASTNode node, ScopeStack scopeStack, ASTNode tree, bool downStream = true);
    static double resolveRate(ASTNode node, ScopeStack scopeStack, ASTNode tree, bool downStream = true);

    static int evaluateConstInteger(ASTNode node, ScopeStack scope, ASTNode tree, QList<LangError> &errors);
    static double evaluateConstReal(ASTNode node, ScopeStack scope, ASTNode tree, QList<LangError> &errors);
    static std::string evaluateConstString(ASTNode node, ScopeStack scope, ASTNode tree, QList<LangError> &errors);
    static ASTNode getMemberfromBlockBundle(DeclarationNode *block, int index, QList<LangError> &errors);
    static ASTNode getValueFromConstBlock(DeclarationNode *block);
    static ASTNode getMemberFromList(ListNode *node, int index, QList<LangError> &errors);
    static std::shared_ptr<PropertyNode> findPropertyByName(vector<std::shared_ptr<PropertyNode> > properties, QString propertyName);
    static QVector<ASTNode > validTypesForPort(std::shared_ptr<DeclarationNode> typeDeclaration, QString portName, ScopeStack scope, ASTNode tree);
    static std::shared_ptr<DeclarationNode> findTypeDeclarationByName(string typeName, ScopeStack scope, ASTNode tree,
                                                      std::vector<std::string> namespaces = std::vector<std::string>());
    static std::shared_ptr<DeclarationNode> findTypeDeclaration(std::shared_ptr<DeclarationNode> block, ScopeStack scope, ASTNode tree);
    static std::shared_ptr<DeclarationNode> findDomainDeclaration(string domainName, ASTNode tree);
    static std::string getDomainIdentifier(ASTNode domain, ScopeStack scopeStack, ASTNode tree);

    static QVector<ASTNode> getPortsForTypeBlock(std::shared_ptr<DeclarationNode> block, ScopeStack scope, ASTNode tree);
    static QVector<ASTNode> getPortsForType(std::string typeName, ScopeStack scope, ASTNode tree, std::vector<string> namespaces);
    static QVector<ASTNode > getInheritedPorts(std::shared_ptr<DeclarationNode> block, ScopeStack scope, ASTNode tree);
    static vector<std::shared_ptr<DeclarationNode> > getInheritedTypes(std::shared_ptr<DeclarationNode> typeDeclaration, ScopeStack scope, ASTNode tree);

    static std::shared_ptr<DeclarationNode> getMainOutputPortBlock(std::shared_ptr<DeclarationNode> moduleBlock);
    static std::shared_ptr<DeclarationNode> getMainInputPortBlock(std::shared_ptr<DeclarationNode> moduleBlock);
    static std::shared_ptr<DeclarationNode> getPort(std::shared_ptr<DeclarationNode> moduleBlock, std::string name);

    /// Number of parallel streams that a single stream can be broken up into
    static int numParallelStreams(StreamNode *stream, StrideSystem &platform, const ScopeStack &scope, ASTNode tree, QList<LangError> &errors);

    /// Get the number of parallel nodes implicit in node. i.e. into how many parallel streams
    /// can the node be broken up.
    static int getNodeSize(ASTNode node, const ScopeStack &scopeStack, ASTNode tree);

    static std::vector<std::string> getModulePropertyNames(std::shared_ptr<DeclarationNode> blockDeclaration);
    static int getFunctionDataSize(std::shared_ptr<FunctionNode> func, ScopeStack scope, ASTNode  tree, QList<LangError> &errors);
    static int getFunctionNumInstances(std::shared_ptr<FunctionNode> func, ScopeStack scope, ASTNode  tree);
    static int getNodeNumOutputs(ASTNode node, const ScopeStack &scope, ASTNode tree, QList<LangError> &errors);    bool validFDomainateNodeDeclaration(QString name, QVector<ASTNode > scopeStack, ASTNode tree);
    static int getNodeNumInputs(ASTNode node, ScopeStack scope, ASTNode tree, QList<LangError> &errors);

    // A value of -1 means undefined. -2 means set from port property.
    // FIXME determine the size set by port properties to provide an accurate size.
    static int getTypeNumOutputs(std::shared_ptr<DeclarationNode> blockDeclaration, const ScopeStack &scope, ASTNode tree, QList<LangError> &errors);
    static int getTypeNumInputs(std::shared_ptr<DeclarationNode> blockDeclaration, const ScopeStack &scope, ASTNode tree, QList<LangError> &errors);

    static int getBlockDeclaredSize(std::shared_ptr<DeclarationNode> block, ScopeStack scope, ASTNode tree, QList<LangError> &errors);

    static int getLargestPropertySize(vector<std::shared_ptr<PropertyNode >> &properties, ScopeStack scope, ASTNode tree, QList<LangError> &errors);

    // Remove this function. getModuleBlocks() is better.
    static ASTNode getBlockSubScope(std::shared_ptr<DeclarationNode> block);
    static int getBundleSize(BundleNode *bundle, ScopeStack scope, ASTNode tree, QList<LangError> &errors);

    static QString getPortTypeName(PortType type);

    static ASTNode getNodeDomain(ASTNode node, ScopeStack scopeStack, ASTNode tree);
    static std::string getNodeDomainName(ASTNode node, ScopeStack scopeStack, ASTNode tree);
    static std::string getDomainNodeString(ASTNode node); // Deprecated. Don't use.
    static QVector<ASTNode > getBlocksInScope(ASTNode root, ScopeStack scopeStack, ASTNode tree);

    static std::vector<std::string> getUsedDomains(ASTNode tree);
    static std::string getFrameworkForDomain(std::string domainName, ASTNode tree);

    static double resolveRateToFloat(ASTNode rateNode, ScopeStack scope, ASTNode tree);
    static double getNodeRate(ASTNode node, ScopeStack scope = {}, ASTNode tree = nullptr);
    static void setNodeRate(ASTNode node, double rate,  ScopeStack scope = {}, ASTNode tree = nullptr);

    static double getDefaultForTypeAsDouble(string type, string port, ScopeStack scope, ASTNode tree, std::vector<string> namespaces);
    static ASTNode getDefaultPortValueForType(string type, string portName, ScopeStack scope, ASTNode tree, std::vector<string> namespaces);

    // Get instance. For blocks the declaration is the instance, for modules it is the function node in the stream
    // Many properties need to be stored in the instance.
    static ASTNode getInstance(ASTNode block, ScopeStack scopeStack, ASTNode tree);
    static bool namespaceMatch(std::vector<string> scopeList, std::shared_ptr<DeclarationNode> decl);

    static vector<StreamNode *> getStreamsAtLine(ASTNode tree, int line);

    static double getDomainDefaultRate(std::shared_ptr<DeclarationNode> domainDecl);

private:

    void validateTree(QString platformRootDir, ASTNode tree);
    void validate();
    QVector<std::shared_ptr<SystemNode> > getPlatformNodes();

    void validatePlatform(ASTNode node, ScopeStack scopeStack);
    void validateTypes(ASTNode node, ScopeStack scopeStack, vector<string> parentNamespace = vector<string>());
    void validateStreamMembers(StreamNode *node, ScopeStack scopeStack);
    void validateBundleIndeces(ASTNode node, ScopeStack scope);
    void validateBundleSizes(ASTNode node, ScopeStack scope);
    void validateSymbolUniqueness(ScopeStack scope);
    void validateListTypeConsistency(ASTNode node, ScopeStack scope);
    void validateStreamSizes(ASTNode tree, ScopeStack scope);
    void validateRates(ASTNode tree);

    void sortErrors();

    void validateStreamInputSize(StreamNode *stream, ScopeStack scope, QList<LangError> &errors);
    void validateNodeRate(ASTNode node, ASTNode tree);

    int getBlockDataSize(std::shared_ptr<DeclarationNode> block, ScopeStack scope, QList<LangError> &errors);

    QString getNodeText(ASTNode node);

    std::shared_ptr<StrideSystem> m_system;
    ASTNode m_tree;
    QList<LangError> m_errors;
    Options m_options;
    SystemConfiguration m_systemConfig;
};

#endif // CODEGEN_H
