#ifndef ASTFUNCTIONS_H
#define ASTFUNCTIONS_H

#include "ast.h"
#include "declarationnode.h"

#include <map>
#include <vector>

class ASTFunctions {
public:
  static ASTNode parseFile(const char *fileName,
                           const char *sourceFilename = nullptr);
  static std::vector<LangError> getParseErrors();

  static void
  insertBuiltinObjects(ASTNode tree,
                       std::map<std::string, std::vector<ASTNode>> importTrees);
  static void
  insertDependentTypes(std::shared_ptr<DeclarationNode> typeDeclaration,
                       std::map<std::string, std::vector<ASTNode>> &objects,
                       ASTNode tree);

  static bool resolveInherits(std::shared_ptr<DeclarationNode> decl,
                              ASTNode tree);

  static void insertBuiltinObjectsForNode(
      ASTNode node, std::map<std::string, std::vector<ASTNode>> &objects,
      ASTNode tree, std::string currentFramework = "");
};

#endif // ASTFUNCTIONS_H
