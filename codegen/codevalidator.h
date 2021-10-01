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

#include <QMap>
#include <QString>
#include <QVariant>
#include <QVector>

#include "strideparser.h"

class StrideSystem;

typedef std::vector<std::pair<ASTNode, std::vector<ASTNode>>> ScopeStack;

class CodeValidator {
public:
  typedef enum { NO_OPTIONS = 0x00, NO_RATE_VALIDATION = 0x01 } Options;

  CodeValidator(ASTNode tree = nullptr, Options options = NO_OPTIONS);
  ~CodeValidator();

  bool isValid();
  bool platformIsValid();

  QList<LangError> getErrors();
  QStringList getPlatformErrors();

  ASTNode getTree() const;
  void setTree(const ASTNode &tree);

  // Static functions -----------------------------

  static std::shared_ptr<DeclarationNode>
  getDeclaration(ASTNode node); // Retrieve stored declaration

  // Resolve types
  static std::string resolveBundleType(BundleNode *bundle,
                                       ScopeStack scopeStack, ASTNode tree);
  static std::string resolveBlockType(BlockNode *name, ScopeStack scopeStack,
                                      ASTNode tree);
  static std::string resolveNodeOutType(ASTNode node, ScopeStack scopeStack,
                                        ASTNode tree);
  static std::string resolveListType(ListNode *listnode, ScopeStack scopeStack,
                                     ASTNode tree);
  static std::string resolveExpressionType(ExpressionNode *exprnode,
                                           ScopeStack scopeStack, ASTNode tree);
  static std::string resolveRangeType(RangeNode *rangenode,
                                      ScopeStack scopeStack, ASTNode tree);
  static std::string resolvePortPropertyType(PortPropertyNode *portproperty,
                                             ScopeStack scopeStack,
                                             ASTNode tree);

  static shared_ptr<DeclarationNode>
  resolveConnectionBlock(ASTNode node, ScopeStack scopeStack, ASTNode tree,
                         bool downStream = true);

  static ASTNode
  getMatchedOuterInstance(std::shared_ptr<FunctionNode> functionNode,
                          std::shared_ptr<DeclarationNode> blockDecl,
                          std::shared_ptr<DeclarationNode> funcDecl,
                          ScopeStack scopeStack, ASTNode tree);

  static int resolveSizePortProperty(std::string targetPortName,
                                     ScopeStack scopeStack,
                                     std::shared_ptr<DeclarationNode> decl,
                                     std::shared_ptr<FunctionNode> func,
                                     ASTNode tree);

  static double resolveRatePortProperty(std::string targetPortName,
                                        ScopeStack scopeStack,
                                        std::shared_ptr<DeclarationNode> decl,
                                        std::shared_ptr<FunctionNode> func,
                                        ASTNode tree);

  static ASTNode resolveDomain(ASTNode node, ScopeStack scopeStack,
                               ASTNode tree, bool downStream = true);
  static double resolveRate(ASTNode node, ScopeStack scopeStack, ASTNode tree,
                            bool downStream = true);

  static int evaluateConstInteger(ASTNode node, ScopeStack scope, ASTNode tree,
                                  QList<LangError> &errors);
  static double evaluateConstReal(ASTNode node, ScopeStack scope, ASTNode tree,
                                  QList<LangError> &errors);
  static std::string evaluateConstString(ASTNode node, ScopeStack scope,
                                         ASTNode tree, string currentFramework,
                                         QList<LangError> &errors);
  static ASTNode
  getMemberfromBlockBundleConst(std::shared_ptr<DeclarationNode> blockDecl,
                                int index, ASTNode tree, ScopeStack scopeStack,
                                QList<LangError> &errors);
  static ASTNode getValueFromConstBlock(DeclarationNode *block);
  static ASTNode getMemberFromList(ListNode *node, int index,
                                   QList<LangError> &errors);
  static std::shared_ptr<PropertyNode>
  findPropertyByName(vector<std::shared_ptr<PropertyNode>> properties,
                     QString propertyName);

  static std::shared_ptr<DeclarationNode>
  findDomainDeclaration(string domainName, string framework, ASTNode tree);
  static std::shared_ptr<DeclarationNode> findDomainDeclaration(string domainId,
                                                                ASTNode tree);

  static std::shared_ptr<DeclarationNode>
  findDataTypeDeclaration(std::string dataTypeName, ASTNode tree);

  static std::string
  getDataTypeForDeclaration(std::shared_ptr<DeclarationNode> decl,
                            ASTNode tree);

  /// Number of parallel streams that a single stream can be broken up into
  static int numParallelStreams(StreamNode *stream, StrideSystem &platform,
                                const ScopeStack &scope, ASTNode tree,
                                QList<LangError> &errors);

  /// Get the number of parallel nodes implicit in node. i.e. into how many
  /// parallel streams can the node be broken up.
  static int getNodeSize(ASTNode node, const ScopeStack &scopeStack,
                         ASTNode tree);

  static int getFunctionDataSize(std::shared_ptr<FunctionNode> func,
                                 ScopeStack scope, ASTNode tree,
                                 QList<LangError> &errors);
  static int getFunctionNumInstances(std::shared_ptr<FunctionNode> func,
                                     ScopeStack scope, ASTNode tree);
  static int getNodeNumOutputs(ASTNode node, const ScopeStack &scope,
                               ASTNode tree, QList<LangError> &errors);
  static int getNodeNumInputs(ASTNode node, ScopeStack scope, ASTNode tree,
                              QList<LangError> &errors);

  // A value of -1 means undefined. -2 means set from port property.
  // FIXME determine the size set by port properties to provide an accurate
  // size.
  static int
  getTypeNumOutputs(std::shared_ptr<DeclarationNode> blockDeclaration,
                    const ScopeStack &scope, ASTNode tree,
                    QList<LangError> &errors);
  static int getTypeNumInputs(std::shared_ptr<DeclarationNode> blockDeclaration,
                              const ScopeStack &scope, ASTNode tree,
                              QList<LangError> &errors);

  static int getBlockDeclaredSize(std::shared_ptr<DeclarationNode> block,
                                  ScopeStack scope, ASTNode tree,
                                  QList<LangError> &errors);

  static int
  getLargestPropertySize(vector<std::shared_ptr<PropertyNode>> &properties,
                         ScopeStack scope, ASTNode tree,
                         QList<LangError> &errors);

  static int getBundleSize(BundleNode *bundle, ScopeStack scope, ASTNode tree,
                           QList<LangError> &errors);

  static ASTNode getNodeDomain(ASTNode node, ScopeStack scopeStack,
                               ASTNode tree);
  static std::string getNodeDomainName(ASTNode node, ScopeStack scopeStack,
                                       ASTNode tree);
  static std::string getDomainIdentifier(ASTNode domain, ScopeStack scopeStack,
                                         ASTNode tree);
  static void setDomainForNode(ASTNode node, ASTNode domain,
                               ScopeStack scopeStack, ASTNode tree,
                               bool force = false);

  static std::vector<ASTNode>
  getBlocksInScope(ASTNode root, ScopeStack scopeStack, ASTNode tree);

  static std::vector<std::string> getUsedDomains(ASTNode tree);
  static std::string getFrameworkForDomain(std::string domainName,
                                           ASTNode tree);
  static std::vector<std::string> getUsedFrameworks(ASTNode tree);

  static double resolveRateToFloat(ASTNode rateNode, ScopeStack scope,
                                   ASTNode tree);
  static double getNodeRate(ASTNode node, ScopeStack scope = {},
                            ASTNode tree = nullptr);
  static void setNodeRate(ASTNode node, double rate, ScopeStack scope = {},
                          ASTNode tree = nullptr);

  static double getDefaultForTypeAsDouble(string type, string port,
                                          ScopeStack scope, ASTNode tree,
                                          std::vector<string> namespaces);
  static ASTNode getDefaultPortValueForType(string type, string portName,
                                            ScopeStack scope, ASTNode tree,
                                            std::vector<string> namespaces);

  // Get instance. For blocks the declaration is the instance, for modules it is
  // the function node in the stream Many properties need to be stored in the
  // instance.
  static ASTNode getInstance(ASTNode block, ScopeStack scopeStack,
                             ASTNode tree);
  static bool namespaceMatch(std::vector<string> scopeList,
                             std::shared_ptr<DeclarationNode> decl,
                             string currentFramework = "");

  static vector<StreamNode *> getStreamsAtLine(ASTNode tree, int line);

  static double
  getDomainDefaultRate(std::shared_ptr<DeclarationNode> domainDecl);

  static std::vector<std::shared_ptr<SystemNode>> getSystemNodes(ASTNode tree);

  static std::vector<std::shared_ptr<ImportNode>> getImportNodes(ASTNode tree);

  static std::vector<ASTNode> loadAllInDirectory(std::string path);

  static std::vector<string> listAvailableSystems(std::string strideroot);

private:
  void validateTree(ASTNode tree);
  void validate();

  void validatePlatform(ASTNode node, ScopeStack scopeStack);

  void validateBundleIndeces(ASTNode node, ScopeStack scope);
  void validateBundleSizes(ASTNode node, ScopeStack scope);
  void validateSymbolUniqueness(ScopeStack scope);
  void validateStreamSizes(ASTNode tree, ScopeStack scope);
  void validateRates(ASTNode tree);
  void validateConstraints(ASTNode tree);

  void sortErrors();

  void validateStreamInputSize(StreamNode *stream, ScopeStack scope,
                               QList<LangError> &errors);
  void validateNodeRate(ASTNode node, ASTNode tree);

  void validateConstraints(std::shared_ptr<StreamNode> stream,
                           ScopeStack scopeStack, ASTNode tree);
  void validateFunctionConstraints(std::shared_ptr<FunctionNode> function,
                                   ScopeStack scopeStack, ASTNode tree);

  void validateConstraintStream(std::shared_ptr<StreamNode> stream,
                                std::shared_ptr<FunctionNode> function,
                                std::shared_ptr<DeclarationNode> declaration,
                                ScopeStack scopeStack, ASTNode tree);

  std::vector<ASTNode>
  resolveConstraintNode(ASTNode node, std::vector<ASTNode> previous,
                        std::shared_ptr<FunctionNode> function,
                        std::shared_ptr<DeclarationNode> declaration,
                        ScopeStack scopeStack, ASTNode tree);

  int getBlockDataSize(std::shared_ptr<DeclarationNode> declaration,
                       ScopeStack scope, QList<LangError> &errors);

  QString getNodeText(ASTNode node);

  std::shared_ptr<StrideSystem> m_system;
  ASTNode m_tree;
  QList<LangError> m_errors;
  Options m_options;
};

#endif // CODEGEN_H
