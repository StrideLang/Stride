#include "astvalidation.h"
#include "astquery.h"

#include "strideparser.h"

#include <algorithm>
#include <iostream>

ASTValidation::ASTValidation() {}

std::vector<LangError>
ASTValidation::validateTypes(ASTNode node, ScopeStack scopeStack, ASTNode tree,
                             std::vector<std::string> parentNamespace,
                             std::string currentFramework) {
  std::vector<LangError> errors;
  if (node->getNodeType() == AST::BundleDeclaration ||
      node->getNodeType() == AST::Declaration) {
    std::shared_ptr<DeclarationNode> decl =
        std::static_pointer_cast<DeclarationNode>(node);
    errors =
        validateTypesForDeclaration(decl, scopeStack, tree, currentFramework);
    return errors;
  } else if (node->getNodeType() == AST::Stream) {
    // Stream members will be processed with children below
  } else if (node->getNodeType() == AST::List) {
    // Children are checked automatically below
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *block = static_cast<BlockNode *>(node.get());
    vector<string> namespaces = block->getNamespaceList();
    namespaces.insert(namespaces.begin(), parentNamespace.begin(),
                      parentNamespace.end());
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(block->getName(), scopeStack, tree,
                                        namespaces, currentFramework);
    if (!declaration) {
      LangError error;
      error.type = LangError::UndeclaredSymbol;
      error.lineNumber = node->getLine();
      error.filename = node->getFilename();
      //            error.errorTokens.push_back(block->getName());
      //            error.errorTokens.push_back(block->getNamespace());
      string blockName = "";
      if (block->getScopeLevels()) {
        for (unsigned int i = 0; i < block->getScopeLevels(); i++) {
          blockName += block->getScopeAt(i);
          blockName += "::";
        }
      }
      blockName += block->getName();
      error.errorTokens.push_back(blockName);
      errors.push_back(error);
    }

  } else if (node->getNodeType() == AST::Bundle) {
    BundleNode *bundle = static_cast<BundleNode *>(node.get());
    vector<string> namespaces = bundle->getNamespaceList();
    namespaces.insert(namespaces.begin(), parentNamespace.begin(),
                      parentNamespace.end());
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(bundle->getName(), scopeStack, tree,
                                        namespaces);
    if (!declaration) {
      LangError error;
      error.type = LangError::UndeclaredSymbol;
      error.lineNumber = node->getLine();
      error.filename = node->getFilename();
      //            error.errorTokens.push_back(bundle->getName());
      //            error.errorTokens.push_back(bundle->getNamespace());
      string bundleName = "";
      if (bundle->getScopeLevels()) {
        for (unsigned int i = 0; i < bundle->getScopeLevels(); i++) {
          bundleName += bundle->getScopeAt(i);
          bundleName += "::";
        }
      }
      bundleName += bundle->getName();
      error.errorTokens.push_back(bundleName);
      errors.push_back(error);
    }
  } else if (node->getNodeType() == AST::Function) {
    FunctionNode *func = static_cast<FunctionNode *>(node.get());
    std::shared_ptr<DeclarationNode> declaration =
        ASTQuery::findDeclarationByName(func->getName(), scopeStack, tree,
                                        func->getNamespaceList());
    if (!declaration) {
      LangError error;
      error.type = LangError::UndeclaredSymbol;
      error.lineNumber = node->getLine();
      error.filename = node->getFilename();
      //            error.errorTokens.push_back(func->getName());
      //            error.errorTokens.push_back(func->getNamespace());
      string funcName = "";
      if (func->getScopeLevels()) {
        for (unsigned int i = 0; i < func->getScopeLevels(); i++) {
          funcName += func->getScopeAt(i);
          funcName += "::";
        }
      }
      funcName += func->getName();
      error.errorTokens.push_back(funcName);
      errors.push_back(error);
    } else {
      for (std::shared_ptr<PropertyNode> property : func->getProperties()) {
        string propertyName = property->getName();
        vector<string> validPorts = ASTQuery::getModulePortNames(declaration);
        // TODO "domain" port is being allowed forcefully. Should this be
        // specified in a different way?
        if (/*propertyName.at(0) != '_' && propertyName != "domain" &&*/
            std::find(validPorts.begin(), validPorts.end(), propertyName) ==
            validPorts.end()) {
          LangError error;
          error.type = LangError::InvalidPort;
          error.lineNumber = func->getLine();
          error.errorTokens.push_back(func->getName());
          error.errorTokens.push_back(propertyName);
          error.filename = func->getFilename();
          errors.push_back(error);
        }
      }
    }
  }

  for (const ASTNode &childNode : node->getChildren()) {
    //    auto frameworkNode = childNode->getCompilerProperty("framework");
    //    std::string frameworkName;
    //    if (frameworkNode) {
    //      frameworkName =
    //          static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    //    }
    auto newErrors =
        validateTypes(childNode, scopeStack, tree, {}, currentFramework);
    errors.insert(errors.end(), newErrors.begin(), newErrors.end());
  }
  return errors;
}

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

std::vector<LangError> ASTValidation::validateTypesForDeclaration(
    std::shared_ptr<DeclarationNode> decl, ScopeStack scopeStack, ASTNode tree,
    string currentFramework) {
  std::vector<LangError> errors;
  auto blockType = decl->getObjectType();

  auto frameworkNode = decl->getCompilerProperty("framework");
  std::string frameworkName;
  if (frameworkNode) {
    frameworkName =
        static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
  }
  std::shared_ptr<DeclarationNode> declaration =
      ASTQuery::findTypeDeclaration(decl, scopeStack, tree, frameworkName);
  if (!declaration) { // Check if node type exists
    LangError error;  // Not a valid type, then error
    error.type = LangError::UnknownType;
    error.lineNumber = decl->getLine();
    error.errorTokens.push_back(decl->getObjectType());
    error.filename = decl->getFilename();
    errors.push_back(error);
  } else {
    // Validate port names and types
    std::vector<std::shared_ptr<PropertyNode>> ports = decl->getProperties();
    ASTNode blocksNode = decl->getPropertyValue("blocks");
    std::vector<ASTNode> blocks;
    if (blocksNode) {
      blocks = blocksNode->getChildren();
    }
    scopeStack.push_back({decl, blocks});
    for (const auto &port : ports) {
      std::string portName = port->getName();
      // Check if portname is valid
      std::vector<ASTNode> portTypesList = ASTQuery::getValidTypesForPort(
          declaration, portName, scopeStack, tree);
      if (portTypesList.size() == 0) {
        LangError error;
        error.type = LangError::InvalidPort;
        error.lineNumber = port->getLine();
        error.errorTokens.push_back(blockType);
        error.errorTokens.push_back(portName);
        error.filename = port->getFilename();
        errors.push_back(error);
      } else {
        // Then check type passed to port is valid
        bool typeIsValid = false;
        ASTNode portValue = port->getValue();

        auto getTypeName = [&](ASTNode child) {
          if (child->getNodeType() == AST::Declaration ||
              child->getNodeType() == AST::BundleDeclaration) {
            auto typeDeclaration = ASTQuery::findTypeDeclaration(
                std::static_pointer_cast<DeclarationNode>(child), scopeStack,
                tree, currentFramework);
            return typeDeclaration->getName();
          } else if (child->getNodeType() == AST::Block ||
                     child->getNodeType() == AST::Bundle) {
            auto decl = ASTQuery::findDeclarationByName(
                ASTQuery::getNodeName(child), scopeStack, tree,
                child->getNamespaceList(), currentFramework);
            if (decl) {
              auto typeDeclaration = ASTQuery::findTypeDeclaration(
                  decl, scopeStack, tree, currentFramework);
              return typeDeclaration->getName();
            }
            return std::string();
          } else if (child->getNodeType() == AST::String) {
            return std::string("_StringLiteral");
          } else if (child->getNodeType() == AST::Int) {
            return std::string("_IntLiteral");
          } else if (child->getNodeType() == AST::Real) {
            return std::string("_RealLiteral");
          } else if (child->getNodeType() == AST::Switch) {
            return std::string("_SwitchLiteral");
          } else if (child->getNodeType() == AST::PortProperty) {
            return std::string("_PortProperty");
          } else if (child->getNodeType() == AST::None) {
            return std::string("_NoneLiteral");
          }
          return std::string();
        };

        if (portValue) {
          std::vector<std::string> validTypeNames;

          for (const ASTNode &validType : portTypesList) {
            if (validType->getNodeType() == AST::String) {
              std::string typeCode =
                  static_cast<ValueNode *>(validType.get())->getStringValue();
              validTypeNames.push_back(typeCode);
            } else if (validType->getNodeType() == AST::Block) {
              auto blockNode = static_pointer_cast<BlockNode>(validType);
              std::shared_ptr<DeclarationNode> declaration =
                  ASTQuery::findDeclarationByName(blockNode->getName(),
                                                  scopeStack, tree);
              //                                    ASTNode typeNameValue =
              //                                    declaration->getPropertyValue("typeName");
              //                                    Q_ASSERT(typeNameValue->getNodeType()
              //                                    == AST::String);
              //                                    Q_ASSERT(declaration);
              if (declaration) {
                string validTypeName = declaration->getName();
                validTypeNames.push_back(validTypeName);
              }
            }
          }
          std::string failedType;
          if (std::find(validTypeNames.begin(), validTypeNames.end(), "") ==
              validTypeNames.end()) {
            if (portValue->getNodeType() == AST::List) {
              typeIsValid = true;
              for (const auto &child : portValue->getChildren()) {
                auto typeName = getTypeName(child);
                bool thisTypeIsValid = false;
                for (const auto &validTypeName : validTypeNames) {
                  if (validTypeName == typeName) {
                    thisTypeIsValid = true;
                    break;
                  }
                }
                if (!thisTypeIsValid) {
                  failedType += typeName + " ";
                }
                typeIsValid &= thisTypeIsValid;
              }
            } else {
              auto typeName = getTypeName(portValue);
              for (const auto &validTypeName : validTypeNames) {
                if (validTypeName == typeName || validTypeName == "") {
                  typeIsValid = true;
                  break;
                }
                failedType = typeName;
              }
            }

          } else { // Catch all for old behavior
            typeIsValid = true;
          }
          if (!typeIsValid) {
            LangError error;
            error.type = LangError::InvalidPortType;
            error.lineNumber = port->getLine();
            error.errorTokens.push_back(blockType);
            error.errorTokens.push_back(portName);
            error.errorTokens.push_back(failedType);
            //                                error.errorTokens.push_back(typeNames[0]);
            //                                //FIXME mark which entry fails
            std::string validTypesList;
            for (const auto &type : validTypeNames) {
              validTypesList += type + ",";
            }
            if (validTypesList.size() > 0) {
              validTypesList =
                  validTypesList.substr(0, validTypesList.size() - 1);
            }
            error.errorTokens.push_back(validTypesList);
            error.filename = port->getFilename();
            errors.push_back(error);
          }
        }
      }
    }
  }
  std::vector<ASTNode> subScope = ASTQuery::getModuleBlocks(decl);
  if (decl->getPropertyValue("ports")) {
    for (const ASTNode &port : decl->getPropertyValue("ports")->getChildren()) {
      subScope.push_back(port);
    }
  }
  scopeStack.push_back({decl, subScope});

  frameworkNode = decl->getCompilerProperty("framework");
  if (frameworkNode) {
    frameworkName =
        std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
  }
  // For BundleDeclarations in particular, we need to ignore the bundle when
  // declaring types. The inner bundle has no scope set, and trying to find
  // it will fail if the declaration is scoped....
  for (const auto &property : decl->getProperties()) {
    if (property->getName() != "constraints") {
      // Ignore constraints streams as they play by different rules
      auto newErrors = validateTypes(property->getValue(), scopeStack, tree,
                                     decl->getNamespaceList(), frameworkName);
      errors.insert(errors.begin(), newErrors.begin(), newErrors.end());
    }
  }
  return errors;
}
