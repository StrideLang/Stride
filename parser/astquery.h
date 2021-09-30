#ifndef ASTQUERY_H
#define ASTQUERY_H

#include "declarationnode.h"

#include <vector>

typedef std::vector<std::pair<ASTNode, std::vector<ASTNode>>> ScopeStack;

class ASTQuery {
public:
  // Find declarations
  static std::shared_ptr<DeclarationNode>
  findDeclarationWithType(std::string objectName, std::string type,
                          const ScopeStack &scopeStack, ASTNode tree,
                          std::vector<std::string> namespaces = {},
                          std::string platform = std::string());

  static std::shared_ptr<DeclarationNode>
  findDeclaration(std::string objectName, const ScopeStack &scopeStack,
                  ASTNode tree, std::vector<std::string> namespaces = {},
                  std::string platform = std::string());

  static std::vector<std::shared_ptr<DeclarationNode>>
  findAllDeclarations(std::string objectName, const ScopeStack &scopeStack,
                      ASTNode tree, std::vector<std::string> namespaces = {},
                      std::string currentFramework = std::string());

  static std::shared_ptr<DeclarationNode>
  findTypeDeclarationByName(std::string typeName, ScopeStack scope,
                            ASTNode tree,
                            std::vector<std::string> namespaces = {},
                            std::string currentFramework = "");

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

private:
  static bool namespaceMatch(std::vector<std::string> scopeList,
                             std::shared_ptr<DeclarationNode> decl,
                             std::string currentFramework = "");
};

#endif // ASTQUERY_H
