#ifndef ASTVALIDATION_H
#define ASTVALIDATION_H

#include "ast.h"
#include "declarationnode.h"
#include "streamnode.h"

#include <functional>

class ASTValidation {
public:
  ASTValidation();

  // Verifies that types are declared for all nodes, and that the properties and
  // property types are allowed by the type declaration
  static std::vector<LangError>
  validateTypes(ASTNode node, ScopeStack scopeStack = ScopeStack(),
                ASTNode tree = nullptr,
                std::vector<std::string> parentNamespace = {},
                std::string currentFramework = "");

  // Validate property types
  static bool isValidStringProperty(std::shared_ptr<DeclarationNode> decl,
                                    std::string propertyName,
                                    bool optional = false, bool verbose = true);
  static bool isValidSwitchProperty(std::shared_ptr<DeclarationNode> decl,
                                    std::string propertyName,
                                    bool optional = false, bool verbose = true);
  static bool isValidIntProperty(std::shared_ptr<DeclarationNode> decl,
                                 std::string propertyName,
                                 bool optional = false, bool verbose = true);
  static bool isValidRealProperty(std::shared_ptr<DeclarationNode> decl,
                                  std::string propertyName,
                                  bool optional = false, bool verbose = true);
  static bool isValidNumberProperty(std::shared_ptr<DeclarationNode> decl,
                                    std::string propertyName,
                                    bool optional = false, bool verbose = true);
  static bool isValidBlockProperty(std::shared_ptr<DeclarationNode> decl,
                                   std::string propertyName,
                                   bool optional = false, bool verbose = true);
  static bool isValidNoneProperty(std::shared_ptr<DeclarationNode> decl,
                                  std::string propertyName,
                                  bool optional = false, bool verbose = true);
  static bool isValidDeclarationProperty(std::shared_ptr<DeclarationNode> decl,
                                         std::string propertyName,
                                         bool optional = false,
                                         bool verbose = true);

  static bool isValidOr(std::function<bool(std::shared_ptr<DeclarationNode>,
                                           std::string, bool, bool)>
                            func1,
                        std::function<bool(std::shared_ptr<DeclarationNode>,
                                           std::string, bool, bool)>
                            func2,
                        std::shared_ptr<DeclarationNode> decl,
                        std::string propertyName, bool optional = false,
                        bool verbose = true);

  static bool isValidListProperty(
      std::shared_ptr<DeclarationNode> decl, std::string propertyName,
      std::function<bool(ASTNode)> validateElement =
          [](ASTNode) { return true; },
      bool optional = false, bool verbose = true);

protected:
  static std::vector<LangError>
  validateTypesForDeclaration(std::shared_ptr<DeclarationNode> decl,
                              ScopeStack scopeStack, ASTNode tree,
                              std::string currentFramework);
};

#endif // ASTVALIDATION_H
