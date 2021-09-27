#ifndef ASTVALIDATION_H
#define ASTVALIDATION_H

#include "ast.h"
#include "declarationnode.h"

#include <functional>

class ASTValidation {
public:
  ASTValidation();

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
};

#endif // ASTVALIDATION_H
