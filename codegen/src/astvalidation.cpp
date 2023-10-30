#include "stride/codegen/astvalidation.hpp"
#include "stride/codegen/astquery.hpp"

//#include "stride/parser/strideparser.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_set>

ASTValidation::ASTValidation() {}

std::vector<LangError> ASTValidation::validate(ASTNode tree) {
  std::vector<LangError> errors;
  validateTypes(tree, errors, {}, tree);
  return errors;
}

void ASTValidation::validateTypes(ASTNode node, std::vector<LangError> &errors,
                                  ScopeStack scopeStack, ASTNode tree,
                                  std::vector<std::string> parentNamespace,
                                  std::string currentFramework) {
  if (node->getNodeType() == AST::BundleDeclaration ||
      node->getNodeType() == AST::Declaration) {
    std::shared_ptr<DeclarationNode> decl =
        std::static_pointer_cast<DeclarationNode>(node);
    validateTypesForDeclaration(decl, scopeStack, errors, tree,
                                currentFramework);

    return; // Children have been processed with corect scopes.
  } else if (node->getNodeType() == AST::Stream) {
    // Stream members will be processed with children below
  } else if (node->getNodeType() == AST::List) {
    // Children are checked automatically below
  } else if (node->getNodeType() == AST::Block) {
    BlockNode *block = static_cast<BlockNode *>(node.get());
    std::vector<std::string> namespaces = block->getNamespaceList();
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
      std::string blockName = "";
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
    std::vector<std::string> namespaces = bundle->getNamespaceList();
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
      std::string bundleName = "";
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
      std::string funcName = "";
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
        std::string propertyName = property->getName();
        std::vector<std::string> validPorts =
            ASTQuery::getModulePortNames(declaration);
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
    validateTypes(childNode, errors, scopeStack, tree, {}, currentFramework);
  }
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

void ASTValidation::validateTypesForDeclaration(
    std::shared_ptr<DeclarationNode> decl, ScopeStack scopeStack,
    std::vector<LangError> &errors, ASTNode tree,
    std::string currentFramework) {
  auto blockType = decl->getObjectType();

  auto frameworkNode = decl->getCompilerProperty("framework");
  std::string frameworkName;
  if (frameworkNode) {
    frameworkName =
        std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
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
        ASTNode portValue = port->getValue();

        if (portValue) {
          for (const auto &validType : portTypesList) {
            // Validate constraints
            if (validType->getNodeType() == AST::Declaration) {
              auto constrainDecl =
                  std::static_pointer_cast<DeclarationNode>(validType);

              if (constrainDecl->getObjectType() == "constrainedInt") {
                validateConstrainedInt(constrainDecl, declaration, portName,
                                       portValue, errors);
              }
              if (constrainDecl->getObjectType() == "constrainedReal") {
                validateConstrainedReal(constrainDecl, declaration, portName,
                                        portValue, errors);
              }
              if (constrainDecl->getObjectType() == "constrainedString") {
                validateConstrainedString(constrainDecl, declaration, portName,
                                          portValue, errors);
              }
              if (constrainDecl->getObjectType() == "constrainedList") {
                validateConstrainedList(constrainDecl, declaration, portName,
                                        portValue, errors, scopeStack, tree, {},
                                        currentFramework);
              }
            }
          }

          bool typeIsValid = false;
          std::string failedType;
          std::vector<std::string> validTypeNames = getValidTypeNames(
              portTypesList, scopeStack, tree, {}, currentFramework);
          { // Validate type names
            if (std::find(validTypeNames.begin(), validTypeNames.end(), "") ==
                validTypeNames.end()) {
              if (portValue->getNodeType() == AST::List) {
                typeIsValid = true;
                for (const auto &child : portValue->getChildren()) {
                  auto typeName = getTypeName(child, scopeStack, tree, {},
                                              currentFramework);
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
                auto typeName = getTypeName(portValue, scopeStack, tree, {},
                                            currentFramework);
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
      std::vector<LangError> internalErrors;
      validateTypes(property->getValue(), internalErrors, scopeStack, tree,
                    decl->getNamespaceList(), frameworkName);
    }
  }
}

void ASTValidation::validateConstrainedInt(
    std::shared_ptr<DeclarationNode> constrainDecl,
    std::shared_ptr<DeclarationNode> declaration, std::string portName,
    ASTNode portValue, std::vector<LangError> &errors) {

  if (portValue->getNodeType() == AST::Int) {
    auto codeValue =
        std::static_pointer_cast<ValueNode>(portValue)->getIntValue();
    if (auto maximumNode = constrainDecl->getPropertyValue("maximum")) {
      if (maximumNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(maximumNode)->getIntValue();
        if (codeValue > value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto minimumNode = constrainDecl->getPropertyValue("minimum")) {
      if (minimumNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(minimumNode)->getIntValue();
        if (codeValue < value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto exMaximumNode =
            constrainDecl->getPropertyValue("exclusiveMaximum")) {
      if (exMaximumNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(exMaximumNode)->getIntValue();
        if (codeValue >= value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto exMinimumNode =
            constrainDecl->getPropertyValue("exclusiveMinimum")) {
      if (exMinimumNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(exMinimumNode)->getIntValue();
        if (codeValue <= value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto multipleOfNode = constrainDecl->getPropertyValue("multipleOf")) {
      if (multipleOfNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(multipleOfNode)->getIntValue();
        if (codeValue % value != 0) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
  } else {
    std::cerr << "Failed constraint" << std::endl;
  }
}

void ASTValidation::validateConstrainedReal(
    std::shared_ptr<DeclarationNode> constrainDecl,
    std::shared_ptr<DeclarationNode> declaration, std::string portName,
    ASTNode portValue, std::vector<LangError> &errors) {

  if (portValue->getNodeType() == AST::Int ||
      portValue->getNodeType() == AST::Real) {
    auto codeValue = std::static_pointer_cast<ValueNode>(portValue)->toReal();
    if (auto maximumNode = constrainDecl->getPropertyValue("maximum")) {
      if (maximumNode->getNodeType() == AST::Int ||
          maximumNode->getNodeType() == AST::Real) {
        auto value = std::static_pointer_cast<ValueNode>(maximumNode)->toReal();
        if (codeValue > value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto minimumNode = constrainDecl->getPropertyValue("minimum")) {
      if (minimumNode->getNodeType() == AST::Int ||
          minimumNode->getNodeType() == AST::Real) {
        auto value = std::static_pointer_cast<ValueNode>(minimumNode)->toReal();
        if (codeValue < value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto exMaximumNode =
            constrainDecl->getPropertyValue("exclusiveMaximum")) {
      if (exMaximumNode->getNodeType() == AST::Int ||
          exMaximumNode->getNodeType() == AST::Real) {
        auto value =
            std::static_pointer_cast<ValueNode>(exMaximumNode)->toReal();
        if (codeValue >= value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto exMinimumNode =
            constrainDecl->getPropertyValue("exclusiveMinimum")) {
      if (exMinimumNode->getNodeType() == AST::Int ||
          exMinimumNode->getNodeType() == AST::Real) {
        auto value =
            std::static_pointer_cast<ValueNode>(exMinimumNode)->toReal();
        if (codeValue <= value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
  }
}

void ASTValidation::validateConstrainedString(
    std::shared_ptr<DeclarationNode> constrainDecl,
    std::shared_ptr<DeclarationNode> declaration, std::string portName,
    ASTNode portValue, std::vector<LangError> &errors) {

  if (portValue->getNodeType() == AST::String) {
    auto codeValue =
        std::static_pointer_cast<ValueNode>(portValue)->getStringValue();
    if (auto maximumNode = constrainDecl->getPropertyValue("maxLength")) {
      if (maximumNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(maximumNode)->getIntValue();
        assert(codeValue.size() < INT64_MAX);
        if (codeValue.size() < INT64_MAX) {
          std::cerr << " ERROR: codeValue too large for int64_t" << std::endl;
        }
        if ((int64_t)codeValue.size() > value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto minimumNode = constrainDecl->getPropertyValue("minLength")) {
      if (minimumNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(minimumNode)->getIntValue();
        assert(codeValue.size() < INT64_MAX);
        if (codeValue.size() < INT64_MAX) {
          std::cerr << " ERROR: codeValue too large for int64_t" << std::endl;
        }
        if ((int64_t)codeValue.size() < value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
  }
}

void ASTValidation::validateConstrainedList(
    std::shared_ptr<DeclarationNode> constrainDecl,
    std::shared_ptr<DeclarationNode> declaration, std::string portName,
    ASTNode portValue, std::vector<LangError> &errors, ScopeStack scopeStack,
    ASTNode tree, std::vector<std::string> parentNamespace,
    std::string currentFramework) {

  if (portValue->getNodeType() == AST::List) {
    if (auto maximumNode = constrainDecl->getPropertyValue("maxItems")) {
      if (maximumNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(maximumNode)->getIntValue();
        if (portValue->getChildren().size() > value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto minimumNode = constrainDecl->getPropertyValue("minItems")) {
      if (minimumNode->getNodeType() == AST::Int) {
        auto value =
            std::static_pointer_cast<ValueNode>(minimumNode)->getIntValue();
        if (portValue->getChildren().size() < value) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto uniqueNode = constrainDecl->getPropertyValue("uniqueItems")) {
      if (uniqueNode->getNodeType() == AST::Switch &&
          std::static_pointer_cast<ValueNode>(uniqueNode)->getSwitchValue() ==
              true) {
        bool repeated = false;
        std::unordered_set<std::string> previous;
        for (const auto &node : portValue->getChildren()) {
          auto newString = AST::toText(node, 0, 0, false);
          if (previous.find(newString) != previous.end()) {
            repeated = true;
            break;
          }
          previous.insert(newString);
        }

        if (repeated) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      }
    }
    if (auto allowedNode = constrainDecl->getPropertyValue("allowed")) {
      if (allowedNode->getNodeType() == AST::List) {
        bool repeated = false;
        std::unordered_set<std::string> previous;
        for (const auto &node : portValue->getChildren()) {
          auto newString = AST::toText(node, 0, 0, false);
          if (previous.find(newString) != previous.end()) {
            repeated = true;
            break;
          }
          previous.insert(newString);
        }
        std::vector<ASTNode> failed;
        for (const auto &node : portValue->getChildren()) {
          if (node->getNodeType() == AST::Declaration) {
            auto constraintDecl =
                std::static_pointer_cast<DeclarationNode>(node);
            if (constraintDecl->getObjectType() == "anyOf") {
            }

          } else if (node->getNodeType() == AST::Block) {
            auto blockName =
                std::static_pointer_cast<BlockNode>(node)->getName();
            if (getTypeName(node, scopeStack, tree, parentNamespace,
                            currentFramework) != blockName) {
              LangError error;
              error.lineNumber = portValue->getLine();
              error.filename = portValue->getFilename();
              error.type = LangError::ConstraintFail;
              error.errorTokens.push_back(declaration->getObjectType());
              error.errorTokens.push_back(portName);
              error.errorTokens.push_back(blockName);
              error.errorTokens.push_back(AST::toText(portValue));
              errors.push_back(error);
            }
          }
        }

        if (repeated) {
          LangError error;
          error.lineNumber = portValue->getLine();
          error.filename = portValue->getFilename();
          error.type = LangError::ConstraintFail;
          error.errorTokens.push_back(declaration->getObjectType());
          error.errorTokens.push_back(portName);
          error.errorTokens.push_back(AST::toText(constrainDecl));
          error.errorTokens.push_back(AST::toText(portValue));
          errors.push_back(error);
        }
      } else {
        std::cerr << "ERROR expecting list for allowed in constrainedList"
                  << std::endl;
      }
    }
  }
}

std::string ASTValidation::getTypeName(ASTNode child, ScopeStack scopeStack,
                                       ASTNode tree,
                                       std::vector<std::string> parentNamespace,
                                       std::string currentFramework) {
  if (child->getNodeType() == AST::Declaration ||
      child->getNodeType() == AST::BundleDeclaration) {
    auto typeDeclaration = ASTQuery::findTypeDeclaration(
        std::static_pointer_cast<DeclarationNode>(child), scopeStack, tree,
        currentFramework);
    if (typeDeclaration) {
      return typeDeclaration->getName();
    }
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
}

std::vector<std::string> ASTValidation::getValidTypeNames(
    std::vector<ASTNode> portTypesList, ScopeStack scopeStack, ASTNode tree,
    std::vector<std::string> parentNamespace, std::string currentFramework) {
  std::vector<std::string> validTypeNames;
  for (const ASTNode &validType : portTypesList) {
    if (validType->getNodeType() == AST::String) {
      std::string typeCode =
          static_cast<ValueNode *>(validType.get())->getStringValue();
      validTypeNames.push_back(typeCode);
    } else if (validType->getNodeType() == AST::Block) {
      auto blockNode = std::static_pointer_cast<BlockNode>(validType);
      std::shared_ptr<DeclarationNode> declaration =
          ASTQuery::findDeclarationByName(blockNode->getName(), scopeStack,
                                          tree);
      if (declaration) {
        std::string validTypeName = declaration->getName();
        validTypeNames.push_back(validTypeName);
      }
    } else if (validType->getNodeType() == AST::Declaration) {
      auto constrainDecl = std::static_pointer_cast<DeclarationNode>(validType);

      if (constrainDecl->getObjectType() == "constrainedInt") {
        validTypeNames.push_back("_IntLiteral");
      }
      if (constrainDecl->getObjectType() == "constrainedReal") {
        validTypeNames.push_back("_IntLiteral");
        validTypeNames.push_back("_RealLiteral");
      }
      if (constrainDecl->getObjectType() == "constrainedString") {
        validTypeNames.push_back("_StringLiteral");
      }
      if (constrainDecl->getObjectType() == "constrainedList") {
        validTypeNames.push_back(""); // FIXME define list type
      }
    }
  }
  return validTypeNames;
}
