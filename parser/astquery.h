#ifndef ASTQUERY_H
#define ASTQUERY_H

#include "strideparser.h"

#include <vector>

typedef std::vector<std::pair<ASTNode, std::vector<ASTNode>>> ScopeStack;

class ASTQuery {
public:
  static std::vector<std::shared_ptr<SystemNode>> getSystemNodes(ASTNode tree);

  static std::vector<std::shared_ptr<ImportNode>> getImportNodes(ASTNode tree);

  // Find declarations
  static std::shared_ptr<DeclarationNode>
  findDeclarationWithType(std::string objectName, std::string type,
                          const ScopeStack &scopeStack, ASTNode tree,
                          std::vector<std::string> namespaces = {},
                          std::string framework = std::string());

  static std::shared_ptr<DeclarationNode>
  findDeclarationByName(std::string objectName, const ScopeStack &scopeStack,
                        ASTNode tree, std::vector<std::string> namespaces = {},
                        std::string framework = std::string());

  static std::vector<std::shared_ptr<DeclarationNode>>
  findAllDeclarations(std::string objectName, const ScopeStack &scopeStack,
                      ASTNode tree, std::vector<std::string> namespaces = {},
                      std::string currentFramework = std::string());

  static std::shared_ptr<DeclarationNode>
  findTypeDeclaration(std::shared_ptr<DeclarationNode> block, ScopeStack scope,
                      ASTNode tree, std::string currentFramework = "");

  static std::shared_ptr<DeclarationNode>
  findTypeDeclarationByName(std::string typeName, ScopeStack scope,
                            ASTNode tree,
                            std::vector<std::string> namespaces = {},
                            std::string currentFramework = "");

  // Get name for node. Names are found in different places for different types
  // of nodes
  static std::string getNodeName(ASTNode node);

  ///// Size queries
  /// Get the number of parallel nodes implicit in node. i.e. into how many
  /// parallel streams can the node be broken up.
  static int getNodeSize(ASTNode node, const ScopeStack &scopeStack,
                         ASTNode tree);

  static int getBundleSize(BundleNode *bundle, ScopeStack scope, ASTNode tree,
                           std::vector<LangError> *errors = nullptr);

  static int getBlockDeclaredSize(std::shared_ptr<DeclarationNode> block,
                                  ScopeStack scope, ASTNode tree,
                                  std::vector<LangError> *errors = nullptr);

  static int
  getLargestPropertySize(std::vector<std::shared_ptr<PropertyNode>> &properties,
                         ScopeStack scope, ASTNode tree,
                         std::vector<LangError> *errors = nullptr);
  // Inheritance information
  static std::vector<std::shared_ptr<DeclarationNode>>
  getInheritedTypes(std::shared_ptr<DeclarationNode> block, ScopeStack scope,
                    ASTNode tree);

  static std::vector<ASTNode>
  getInheritedPorts(std::shared_ptr<DeclarationNode> block, ScopeStack scope,
                    ASTNode tree);

  // Type information
  static std::vector<ASTNode>
  getPortsForType(std::string typeName, ScopeStack scope, ASTNode tree,
                  std::vector<std::string> namespaces = {},
                  std::string framework = "");

  static std::vector<ASTNode>
  getPortsForTypeBlock(std::shared_ptr<DeclarationNode> block, ScopeStack scope,
                       ASTNode tree);

  static std::vector<ASTNode>
  getValidTypesForPort(std::shared_ptr<DeclarationNode> typeDeclaration,
                       std::string portName, ScopeStack scope, ASTNode tree);

  // Module Query
  static std::vector<ASTNode>
  getModuleBlocks(std::shared_ptr<DeclarationNode> moduleDecl);

  static std::shared_ptr<DeclarationNode>
  getModuleMainOutputPortBlock(std::shared_ptr<DeclarationNode> moduleDecl);

  static std::shared_ptr<DeclarationNode>
  getModuleMainInputPortBlock(std::shared_ptr<DeclarationNode> moduleDecl);

  static std::shared_ptr<DeclarationNode>
  getModulePort(std::shared_ptr<DeclarationNode> moduleDecl, std::string name);

  static std::vector<std::string>
  getModulePortNames(std::shared_ptr<DeclarationNode> blockDeclaration);

  // Bundle Declarations
  static ASTNode
  getMemberfromBlockBundleConst(std::shared_ptr<DeclarationNode> blockDecl,
                                int index, ASTNode tree, ScopeStack scopeStack,
                                std::vector<LangError> *errors);

  // Const Declarations
  static ASTNode getValueFromConstBlock(DeclarationNode *block);

  // List nodes
  static ASTNode getMemberFromList(ListNode *node, int index,
                                   std::vector<LangError> *errors);

  // Property Nodes
  static std::shared_ptr<PropertyNode>
  findPropertyByName(std::vector<std::shared_ptr<PropertyNode>> properties,
                     std::string propertyName);

private:
  static bool namespaceMatch(std::vector<std::string> scopeList,
                             std::shared_ptr<DeclarationNode> decl,
                             std::string currentFramework = "");
};

#endif // ASTQUERY_H
