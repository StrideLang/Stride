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
                          std::string platform = std::string());

  static std::shared_ptr<DeclarationNode>
  findDeclarationByName(std::string objectName, const ScopeStack &scopeStack,
                        ASTNode tree, std::vector<std::string> namespaces = {},
                        std::string platform = std::string());

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

private:
  static bool namespaceMatch(std::vector<std::string> scopeList,
                             std::shared_ptr<DeclarationNode> decl,
                             std::string currentFramework = "");
};

#endif // ASTQUERY_H
