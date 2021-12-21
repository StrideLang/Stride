#ifndef ASTFUNCTIONS_H
#define ASTFUNCTIONS_H

#include "ast.h"
#include "declarationnode.h"
#include "expressionnode.h"
#include "streamnode.h"
#include "valuenode.h"

#include <map>
#include <vector>

// The ASTFunctions, ASTQuery and ASTRuntime should provide a minimal layer
// dealing with syntax constructs and minimal semantic information
// (with the large exception of 'types')

class ASTFunctions {
public:
  static ASTNode parseFile(const char *fileName,
                           const char *sourceFilename = nullptr);
  static std::vector<LangError> getParseErrors();

  static std::vector<ASTNode> loadAllInDirectory(std::string path);

  // Insert properties from inherited types if not present
  static bool resolveInheritance(std::shared_ptr<DeclarationNode> decl,
                                 ASTNode tree);

  // Insert system and library objects used in tree from import trees.
  // externalNodes provides library and import nodes. The key is the namespace
  // they are imported into
  static void insertRequiredObjects( //**
      ASTNode tree, std::map<std::string, std::vector<ASTNode>> externalNodes);
  static void insertRequiredObjectsForNode( //**
      ASTNode node, std::map<std::string, std::vector<ASTNode>> &objects,
      ASTNode tree, std::string currentFramework = "");

  // Insert system and library types required in declaration
  // externalNodes provides library and import nodes. The key is the namespace
  // they are imported into
  static void insertDependentTypes(
      std::shared_ptr<DeclarationNode> typeDeclaration,
      std::map<std::string, std::vector<ASTNode>> &externalNodes, ASTNode tree);

  // Fill default properties for all declarations from type declarations
  // The tree must contain all required declarations (type, module, reaction,
  // etc.) that define the default properties
  static void fillDefaultProperties(ASTNode tree);

  // The scopeNodes vector must contain any additional declarations needed to
  // fill the node's default properties
  static void fillDefaultPropertiesForNode(ASTNode node,
                                           std::vector<ASTNode> scopeNodes);

  // Process Anonymous declarations inside streams
  // extracting definitions and a replacing with name
  static void processAnoymousDeclarations(ASTNode tree);

  // Recursively resolve constants
  static void resolveConstantsInNode(ASTNode node, ScopeStack scope,
                                     ASTNode tree);

  // Reduce 'value' to a single constant ValueNode if possible
  static std::shared_ptr<ValueNode> resolveConstant(ASTNode value,
                                                    ScopeStack scope,
                                                    ASTNode tree,
                                                    std::string framework = "");

  static std::shared_ptr<ValueNode>
  reduceConstExpression(std::shared_ptr<ExpressionNode> expr, ScopeStack scope,
                        ASTNode tree);

  static int evaluateConstInteger(ASTNode node, ScopeStack scope, ASTNode tree,
                                  std::vector<LangError> *errors);
  static double evaluateConstReal(ASTNode node, ScopeStack scope, ASTNode tree,
                                  std::vector<LangError> *errors);
  static std::string evaluateConstString(ASTNode node, ScopeStack scope,
                                         ASTNode tree,
                                         std::string currentFramework,
                                         std::vector<LangError> *errors);
  // Type defaults
  static double
  getDefaultForTypeAsDouble(std::string type, std::string port,
                            ScopeStack scope, ASTNode tree,
                            std::vector<std::string> namespaces,
                            std::vector<LangError> *errors = nullptr);
  static ASTNode
  getDefaultPortValueForType(std::string type, std::string portName,
                             ScopeStack scope, ASTNode tree,
                             std::vector<std::string> namespaces);

protected:
  // Anonymous declaration helpre functions
  static std::vector<ASTNode>
  processAnonDeclsForScope(const std::vector<ASTNode> scopeTree);
  // Look for declarations in a stream, extract them and replace them with a
  // block
  static std::vector<std::shared_ptr<DeclarationNode>>
  extractStreamDeclarations(std::shared_ptr<StreamNode> stream);
};

#endif // ASTFUNCTIONS_H
