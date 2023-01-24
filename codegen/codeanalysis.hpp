#ifndef CODEANALYSIS_HPP
#define CODEANALYSIS_HPP

#include "strideparser.h"
#include "stridesystem.hpp"

class CodeAnalysis {
public:
  enum { SIZE_UNKNOWN = -1, SIZE_PORT_PROPERTY = -2 };
  // Global tree analysis
  static std::vector<std::string> getUsedDomains(ASTNode tree);
  static std::vector<std::string> getUsedFrameworks(ASTNode tree);

  // Get instance. For blocks the declaration is the instance, for modules it is
  // the function node in the stream Many properties need to be stored in the
  // instance.
  static ASTNode getInstance(ASTNode block, ScopeStack scopeStack,
                             ASTNode tree);

  // Retrieve stored declaration in "declaration" compiler property
  static std::shared_ptr<DeclarationNode> getDeclaration(ASTNode node);

  // Domain queries
  static std::shared_ptr<DeclarationNode>
  findDomainDeclaration(std::string domainName, std::string framework,
                        ASTNode tree);

  static std::shared_ptr<DeclarationNode>
  findDomainDeclaration(std::string domainId, ASTNode tree);

  static ASTNode getNodeDomain(ASTNode node, ScopeStack scopeStack,
                               ASTNode tree);

  static std::string getNodeDomainName(ASTNode node, ScopeStack scopeStack,
                                       ASTNode tree);

  static std::string getDomainIdentifier(ASTNode domain, ScopeStack scopeStack,
                                         ASTNode tree);

  static ASTNode resolveDomain(ASTNode node, ScopeStack scopeStack,
                               ASTNode tree, bool downStream = true);

  // Rate queries
  static double getNodeRate(ASTNode node, ScopeStack scope = ScopeStack(),
                            ASTNode tree = nullptr);

  static double resolveRate(ASTNode node, ScopeStack scopeStack, ASTNode tree,
                            bool downStream = true);

  static double resolveRateToFloat(ASTNode rateNode, ScopeStack scope,
                                   ASTNode tree,
                                   std::vector<LangError> *errors);
  static double
  getDomainDefaultRate(std::shared_ptr<DeclarationNode> domainDecl);

  // Framework queries
  static std::string getFrameworkForDomain(std::string domainName,
                                           ASTNode tree);
  // Stream analysis

  /// Number of parallel streams that a single stream can be broken up into
  static int numParallelStreams(StreamNode *stream, StrideSystem &platform,
                                const ScopeStack &scope, ASTNode tree,
                                std::vector<LangError> *errors = nullptr);

  static std::shared_ptr<DeclarationNode>
  resolveConnectionBlock(ASTNode node, ScopeStack scopeStack, ASTNode tree,
                         bool downStream = true);

  //
  static ASTNode
  getMatchedOuterInstance(std::shared_ptr<FunctionNode> functionNode,
                          std::shared_ptr<DeclarationNode> blockDecl,
                          std::shared_ptr<DeclarationNode> funcDecl,
                          ScopeStack scopeStack, ASTNode tree);

  static std::vector<ASTNode>
  getBlocksInScope(ASTNode root, ScopeStack scopeStack, ASTNode tree);
  // Evaluate value of port property (size and rate)
  static int evaluateSizePortProperty(std::string targetPortName,
                                      ScopeStack scopeStack,
                                      std::shared_ptr<DeclarationNode> decl,
                                      std::shared_ptr<FunctionNode> func,
                                      ASTNode tree);

  static double evaluateRatePortProperty(std::string targetPortName,
                                         ScopeStack scopeStack,
                                         std::shared_ptr<DeclarationNode> decl,
                                         std::shared_ptr<FunctionNode> func,
                                         ASTNode tree);

  static std::vector<std::shared_ptr<PortPropertyNode>>
  getUsedPortProperties(std::shared_ptr<DeclarationNode> funcDecl);

  static std::vector<std::shared_ptr<PortPropertyNode>>
  getUsedPortPropertiesInNode(ASTNode node);

  // //

  // Input and output size
  static int getNodeNumOutputs(ASTNode node, const ScopeStack &scope,
                               ASTNode tree,
                               std::vector<LangError> *errors = nullptr);
  static int getNodeNumInputs(ASTNode node, ScopeStack scope, ASTNode tree,
                              std::vector<LangError> *errors = nullptr);

  // A value of -1 means undefined. -2 means set from port property.
  // FIXME determine the size set by port properties to provide an accurate
  // size.
  static int
  getTypeNumOutputs(std::shared_ptr<DeclarationNode> blockDeclaration,
                    const ScopeStack &scope, ASTNode tree,
                    std::vector<LangError> *errors = nullptr);

  static int getTypeNumInputs(std::shared_ptr<DeclarationNode> blockDeclaration,
                              const ScopeStack &scope, ASTNode tree,
                              std::vector<LangError> *errors = nullptr);

  static int getFunctionDataSize(std::shared_ptr<FunctionNode> func,
                                 ScopeStack scope, ASTNode tree,
                                 std::vector<LangError> *errors = nullptr);

  static int getFunctionNumInstances(std::shared_ptr<FunctionNode> func,
                                     ScopeStack scope, ASTNode tree,
                                     std::vector<LangError> *errors = nullptr);

  static std::shared_ptr<DeclarationNode>
  findDataTypeDeclaration(std::string dataTypeName, ASTNode tree);

  static std::string
  getDataTypeForDeclaration(std::shared_ptr<DeclarationNode> decl,
                            ASTNode tree);
};

#endif // CODEANALYSIS_HPP
