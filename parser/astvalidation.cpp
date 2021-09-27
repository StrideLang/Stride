#include "astvalidation.h"

#include <iostream>

ASTValidation::ASTValidation() {}

bool ASTValidation::isValidStringProperty(std::shared_ptr<DeclarationNode> decl,
                                          std::string propertyName,
                                          bool optional, bool verbose) {
  auto node = decl->getPropertyValue(propertyName);
  if (!node && optional) {
    return true;
  } else if (!node || node->getNodeType() != AST::String) {
    if (verbose) {
      std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                << std::endl;
    }
    return false;
  }
  return true;
}

bool ASTValidation::isValidSwitchProperty(std::shared_ptr<DeclarationNode> decl,
                                          std::string propertyName,
                                          bool optional, bool verbose) {
  auto node = decl->getPropertyValue(propertyName);
  if (!node && optional) {
    return true;
  } else if (!node || node->getNodeType() != AST::Switch) {
    if (verbose) {
      std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                << std::endl;
    }
    return false;
  }
  return true;
}

bool ASTValidation::isValidIntProperty(std::shared_ptr<DeclarationNode> decl,
                                       std::string propertyName, bool optional,
                                       bool verbose) {
  auto node = decl->getPropertyValue(propertyName);
  if (!node && optional) {
    return true;
  } else if (!node || node->getNodeType() != AST::Int) {
    if (verbose) {
      std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                << std::endl;
    }
    return false;
  }
  return true;
}

bool ASTValidation::isValidRealProperty(std::shared_ptr<DeclarationNode> decl,
                                        std::string propertyName, bool optional,
                                        bool verbose) {
  auto node = decl->getPropertyValue(propertyName);
  if (!node && optional) {
    return true;
  } else if (!node || node->getNodeType() != AST::Real) {
    if (verbose) {
      std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                << std::endl;
    }
    return false;
  }
  return true;
}

bool ASTValidation::isValidNumberProperty(std::shared_ptr<DeclarationNode> decl,
                                          std::string propertyName,
                                          bool optional, bool verbose) {
  auto isNumber = isValidIntProperty(decl, propertyName, optional, false) ||
                  isValidRealProperty(decl, propertyName, optional, false);
  if (verbose && !isNumber) {
    std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
              << std::endl;
  }
  return isNumber;
}

bool ASTValidation::isValidBlockProperty(std::shared_ptr<DeclarationNode> decl,
                                         std::string propertyName,
                                         bool optional, bool verbose) {
  auto node = decl->getPropertyValue(propertyName);
  if (!node && optional) {
    return true;
  } else if (!node || node->getNodeType() != AST::Block) {
    if (verbose) {
      std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                << std::endl;
    }
    return false;
  }
  return true;
}

bool ASTValidation::isValidNoneProperty(std::shared_ptr<DeclarationNode> decl,
                                        std::string propertyName, bool optional,
                                        bool verbose) {
  auto node = decl->getPropertyValue(propertyName);
  if (!node && optional) {
    return true;
  } else if (!node || node->getNodeType() != AST::None) {
    if (verbose) {
      std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                << std::endl;
    }
    return false;
  }
  return true;
}

bool ASTValidation::isValidDeclarationProperty(
    std::shared_ptr<DeclarationNode> decl, std::string propertyName,
    bool optional, bool verbose) {
  auto node = decl->getPropertyValue(propertyName);
  if (!node && optional) {
    return true;
  } else if (!node || node->getNodeType() != AST::Declaration) {
    if (verbose) {
      std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                << std::endl;
    }
    return false;
  } /*else {
    auto decl = std::static_pointer_cast<DeclarationNode>(node);
    if (objectTypes.size() == 0) {
      return true;
    }
    if (std::find(objectTypes.begin(), objectTypes.end(), decl->getObjectType())
        == objectTypes.end()) {
      if (verbose) {
        std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
  << std::endl;
      }
    }
    return false;
  }*/
  return true;
}

bool ASTValidation::isValidOr(
    std::function<bool(std::shared_ptr<DeclarationNode>, std::string, bool,
                       bool)>
        func1,
    std::function<bool(std::shared_ptr<DeclarationNode>, std::string, bool,
                       bool)>
        func2,
    std::shared_ptr<DeclarationNode> decl, std::string propertyName,
    bool optional, bool verbose) {
  bool valid1 = func1(decl, propertyName, true, false);
  bool valid2 = func2(decl, propertyName, true, false);
  return valid1 || valid2;
}

bool ASTValidation::isValidListProperty(
    std::shared_ptr<DeclarationNode> decl, std::string propertyName,
    std::function<bool(ASTNode)> validateElement, bool optional, bool verbose) {
  auto node = decl->getPropertyValue(propertyName);
  if (!node && optional) {
    return true;
  } else if (!node || node->getNodeType() != AST::List) {
    if (verbose) {
      std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                << std::endl;
    }
    return false;
  }
  if (!optional) {
    for (auto busName : node->getChildren()) {
      if (!validateElement(busName)) {
        if (verbose) {
          std::cerr << __FILE__ << ":" << __LINE__ << " Failed " << propertyName
                    << std::endl;
        }
        return false;
      }
    }
  }
  return true;
}
