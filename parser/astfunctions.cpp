#include "astfunctions.h"
#include "astquery.h"
#include "astruntime.h"

#include "blocknode.h"
#include "expressionnode.h"
#include "functionnode.h"
#include "portpropertynode.h"
#include "streamnode.h"
#include "valuenode.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

extern AST *parse(const char *fileName, const char *sourceFilename);
extern std::vector<LangError> getErrors();

ASTNode ASTFunctions::parseFile(const char *fileName,
                                const char *sourceFilename) {
  return std::shared_ptr<AST>(parse(fileName, sourceFilename));
}

std::vector<LangError> ASTFunctions::getParseErrors() { return getErrors(); }

void ASTFunctions::insertRequiredObjects(
    ASTNode tree, std::map<std::string, std::vector<ASTNode>> externalNodes) {
  auto children = tree->getChildren();
  for (ASTNode object : children) {
    insertRequiredObjectsForNode(object, externalNodes, tree);
  }
}

void ASTFunctions::insertDependentTypes(
    std::shared_ptr<DeclarationNode> typeDeclaration,
    std::map<std::string, std::vector<ASTNode>> &externalNodes, ASTNode tree) {
  std::vector<std::shared_ptr<DeclarationNode>> blockList;
  //    std::shared_ptr<DeclarationNode> existingDecl =
  //    CodeValidator::findTypeDeclaration(typeDeclaration, ScopeStack(),
  //    m_tree);
  for (auto it = externalNodes.begin(); it != externalNodes.end(); it++) {
    // To avoid redundant checking here we should mark nodes that have already
    // been processed
    auto inheritedTypes = ASTQuery::getInheritedTypes(
        typeDeclaration, {{nullptr, it->second}}, tree);

    for (auto inheritedType : inheritedTypes) {
      auto children = tree->getChildren();
      if (std::find(children.begin(), children.end(), inheritedType) !=
          children.end()) {
        // nothing
      } else {
        tree->addChild(inheritedType);
        auto findPos =
            std::find(it->second.begin(), it->second.end(), inheritedType);
        if (findPos != it->second.end()) {
          it->second.erase(findPos);
        }
      }
      insertDependentTypes(inheritedType, externalNodes, tree);
    }
  }
}

void ASTFunctions::insertRequiredObjectsForNode(
    ASTNode node, std::map<std::string, std::vector<ASTNode>> &objects,
    ASTNode tree, std::string currentFramework) {
  std::vector<std::shared_ptr<DeclarationNode>> blockList;
  if (node->getNodeType() == AST::List) {
    for (ASTNode child : node->getChildren()) {
      insertRequiredObjectsForNode(child, objects, tree, currentFramework);
    }
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *stream = static_cast<StreamNode *>(node.get());
    insertRequiredObjectsForNode(stream->getLeft(), objects, tree);
    insertRequiredObjectsForNode(stream->getRight(), objects, tree);
  } else if (node->getNodeType() == AST::Expression) {
    ExpressionNode *expr = static_cast<ExpressionNode *>(node.get());
    if (expr->isUnary()) {
      insertRequiredObjectsForNode(expr->getValue(), objects, tree);
    } else {
      insertRequiredObjectsForNode(expr->getLeft(), objects, tree);
      insertRequiredObjectsForNode(expr->getRight(), objects, tree);
    }
  } else if (node->getNodeType() == AST::Function) {
    FunctionNode *func = static_cast<FunctionNode *>(node.get());
    for (auto it = objects.begin(); it != objects.end(); it++) {
      //        auto existingDecl = ASTQuery::findDeclaration(
      //            QString::fromStdString(func->getName()), {}, tree,
      //            {it->first}, currentFramework);
      // for now, insert all declarations from all frameworks.
      // FIXME check outer domain and framework to only import needed modules

      std::vector<std::shared_ptr<DeclarationNode>> alldecls =
          ASTQuery::findAllDeclarations(
              func->getName(), {{nullptr, it->second}}, nullptr,
              func->getNamespaceList(), currentFramework);
      for (const auto &decl : alldecls) {
        if (std::find(blockList.begin(), blockList.end(), decl) ==
            blockList.end()) {
          blockList.push_back(decl);
        }
      }
    }
    // Look for declarations of blocks present in function properties
    for (auto property : func->getProperties()) {
      insertRequiredObjectsForNode(property->getValue(), objects, tree);
    }
    for (std::shared_ptr<DeclarationNode> usedBlock : blockList) {
      // Add declarations to tree if not there
      auto usedBlockFramework = usedBlock->getCompilerProperty("framework");
      std::string fw;

      if (usedBlockFramework &&
          usedBlockFramework->getNodeType() == AST::String) {
        fw = std::static_pointer_cast<ValueNode>(usedBlockFramework)
                 ->getStringValue();
      }

      // Declarations don't have a namespace, but they can be imported into one
      // This is written into the namespaceTree property
      auto namespaceTreeNode = usedBlock->getCompilerProperty("namespaceTree");
      std::vector<std::string> namespaceTree;
      if (namespaceTreeNode) {
        for (auto node : namespaceTreeNode->getChildren()) {
          namespaceTree.push_back(
              std::static_pointer_cast<ValueNode>(node)->getStringValue());
        }
      }
      if (namespaceTree.size() > 0 && namespaceTree.at(0) == fw) {
        namespaceTree.erase(namespaceTree.begin());
      }

      if (!ASTQuery::findDeclaration(usedBlock->getName(), ScopeStack(), tree,
                                     namespaceTree, fw)) {
        tree->addChild(usedBlock);
      }
      insertRequiredObjectsForNode(usedBlock, objects, tree, fw);
    }
  } else if (node->getNodeType() == AST::Declaration ||
             node->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> declaration =
        std::static_pointer_cast<DeclarationNode>(node);
    //    vector<ASTNode> namespaceObjects;
    if (declaration->getScopeLevels() > 0) {
      //      namespaceObjects = objects[declaration->getScopeAt(
      //          0)]; // TODO need to implement for namespaces that are more
      //          than one
      // level deep...
    }
    //    auto typeDecl = CodeValidator::findTypeDeclarationByName(
    //        declaration->getObjectType(), {}, tree);
    //    if (!typeDecl) {

    auto frameworkNode = node->getCompilerProperty("framework");
    std::string framework;
    if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
      framework =
          std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    for (auto it = objects.begin(); it != objects.end(); it++) {
      auto existingTypeDecl = ASTQuery::findTypeDeclarationByName(
          declaration->getObjectType(), {}, tree, {it->first}, framework);
      //      for (auto objectTree : it->second) {
      // FIXME get namespaces for declaration, not objects to insert
      auto typeDecl = ASTQuery::findTypeDeclarationByName(
          declaration->getObjectType(), {{nullptr, it->second}}, nullptr, {},
          framework);
      if (!typeDecl) { // try root namespace
        typeDecl = ASTQuery::findTypeDeclarationByName(
            declaration->getObjectType(), {{nullptr, it->second}}, nullptr, {});
      }
      if (typeDecl && !existingTypeDecl) {
        tree->addChild(typeDecl);
        // FIXME instead of removing we must make sure that objects are not
        // inserted in tree if already there
        auto position =
            std::find(it->second.begin(), it->second.end(), typeDecl);
        if (position != it->second.end()) {
          it->second.erase(position);
        }
        insertRequiredObjectsForNode(typeDecl, objects, tree);
        insertDependentTypes(typeDecl, objects, tree);
      }
      //      }
    }
    if (declaration /*&& !ASTQuery::findDeclaration(
                  QString::fromStdString(name->getName()), {},
                  tree, name->getNamespaceList())*/) {
      //                declaration->setRootScope(it->first);
      if (std::find(blockList.begin(), blockList.end(), declaration) ==
          blockList.end()) {
        blockList.push_back(declaration);
        // Insert needed objects for things in module properties
        for (std::shared_ptr<PropertyNode> property :
             declaration->getProperties()) {
          if (property->getName() == "constraints") {
            continue;
            // Constraints play by their own rules.
          }
          insertRequiredObjectsForNode(property->getValue(), objects, tree,
                                       framework);
        }
        // Process index for bundle declarations
        if (node->getNodeType() == AST::BundleDeclaration) {
          insertRequiredObjectsForNode(declaration->getBundle()->index(),
                                       objects, tree, framework);
        }
      }
    }
    //  }
    //    }
  } else if (node->getNodeType() == AST::Block) {
    //    QList<std::shared_ptr<DeclarationNode>> blockList;
    BlockNode *name = static_cast<BlockNode *>(node.get());

    auto declaration = ASTQuery::findDeclaration(
        name->getName(), {}, tree, name->getNamespaceList(), currentFramework);

    if (!declaration) {
      for (auto it = objects.begin(); it != objects.end(); it++) {
        auto newDeclarations = ASTQuery::findAllDeclarations(
            name->getName(), {{nullptr, it->second}}, nullptr,
            name->getNamespaceList(), currentFramework);

        for (auto newDecl : newDeclarations) {
          if (std::find(blockList.begin(), blockList.end(), declaration) ==
              blockList.end()) {
            blockList.push_back(newDecl);
            auto existingPos =
                std::find(it->second.begin(), it->second.end(), newDecl);
            if (existingPos != it->second.end()) {
              it->second.erase(existingPos);
            }
          }
        }
      }
      for (auto usedBlock : blockList) {
        auto frameworkNode = usedBlock->getCompilerProperty("framework");
        std::string framework;
        if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
          framework = std::static_pointer_cast<ValueNode>(frameworkNode)
                          ->getStringValue();
        }
        auto namespaceList = usedBlock->getNamespaceList();
        //        if (namespaceList.size() > 0 && namespaceList[0] == framework)
        //        {
        //          namespaceList.erase(namespaceList.begin());
        //        }
        auto existingDeclaration = ASTQuery::findDeclaration(
            usedBlock->getName(), {}, tree, namespaceList, framework);
        if (!existingDeclaration) {
          tree->addChild(usedBlock);
        }
        insertRequiredObjectsForNode(usedBlock, objects, tree);
      }
    }
  } else if (node->getNodeType() == AST::Bundle) {
    std::vector<std::shared_ptr<DeclarationNode>> blockList;
    auto bundle = std::static_pointer_cast<BundleNode>(node);
    auto declaration =
        ASTQuery::findDeclaration(bundle->getName(), {}, tree,
                                  bundle->getNamespaceList(), currentFramework);
    if (!declaration) {
      for (auto it = objects.begin(); it != objects.end(); it++) {
        auto newDeclarations = ASTQuery::findAllDeclarations(
            bundle->getName(), {{nullptr, it->second}}, nullptr,
            bundle->getNamespaceList(), currentFramework);
        for (auto newDecl : newDeclarations) {
          if (std::find(blockList.begin(), blockList.end(), declaration) ==
              blockList.end()) {
            blockList.push_back(newDecl);
            auto existingPos =
                std::find(it->second.begin(), it->second.end(), newDecl);
            if (existingPos != it->second.end()) {
              it->second.erase(existingPos);
            }
          }
        }
      }
      for (auto usedBlock : blockList) {
        auto frameworkNode = usedBlock->getCompilerProperty("framework");
        std::string framework;
        if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
          framework = std::static_pointer_cast<ValueNode>(frameworkNode)
                          ->getStringValue();
        }
        auto existingDeclaration =
            ASTQuery::findDeclaration(usedBlock->getName(), {}, tree,
                                      usedBlock->getNamespaceList(), framework);
        if (!existingDeclaration) {
          tree->addChild(usedBlock);
        }
        insertRequiredObjectsForNode(usedBlock, objects, tree);
      }
    }
  } else if (node->getNodeType() == AST::Property) {
    std::shared_ptr<PropertyNode> prop =
        std::static_pointer_cast<PropertyNode>(node);
    insertRequiredObjectsForNode(prop->getValue(), objects, tree);
  }
}

void ASTFunctions::fillDefaultProperties(ASTNode tree) {
  std::vector<ASTNode> nodes = tree->getChildren();
  for (const auto &node : nodes) {
    ASTFunctions::fillDefaultPropertiesForNode(node, tree->getChildren());
  }
}

void ASTFunctions::fillDefaultPropertiesForNode(
    ASTNode node, std::vector<ASTNode> scopeNodes) {
  if (node->getNodeType() == AST::Declaration ||
      node->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> destBlock =
        std::static_pointer_cast<DeclarationNode>(node);
    std::vector<std::shared_ptr<PropertyNode>> blockProperties =
        destBlock->getProperties();
    auto frameworkNode = node->getCompilerProperty("framework");
    std::string frameworkName;
    if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
      frameworkName =
          std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    auto typeProperties = ASTQuery::getPortsForType(
        destBlock->getObjectType(), {{nullptr, scopeNodes}}, nullptr,
        destBlock->getNamespaceList(), frameworkName);
    if (typeProperties.size() == 0) {
      std::cerr
          << "ERROR: fillDefaultPropertiesForNode() No type definition for "
          << destBlock->getObjectType() << std::endl;
      return;
    }
    for (const auto &property : blockProperties) {
      fillDefaultPropertiesForNode(property->getValue(), scopeNodes);
    }

    for (const ASTNode &propertyListMember : typeProperties) {
      assert(propertyListMember->getNodeType() == AST::Declaration);
      DeclarationNode *portDescription =
          static_cast<DeclarationNode *>(propertyListMember.get());
      ASTNode propName = portDescription->getPropertyValue("name");
      assert(propName->getNodeType() == AST::String);
      std::string propertyName =
          static_cast<ValueNode *>(propName.get())->getStringValue();
      bool propertySet = false;
      for (const auto &blockProperty : blockProperties) {
        if (blockProperty->getName() == propertyName) {
          propertySet = true;
          break;
        }
      }
      if (!propertySet) {
        ASTNode defaultValueNode = portDescription->getPropertyValue("default");
        std::shared_ptr<PropertyNode> newProperty =
            std::make_shared<PropertyNode>(
                propertyName, defaultValueNode->deepCopy(),
                portDescription->getFilename().data(),
                portDescription->getLine());
        destBlock->addProperty(newProperty);
      }
    }
  } else if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> destFunc =
        std::static_pointer_cast<FunctionNode>(node);
    std::vector<std::shared_ptr<PropertyNode>> blockProperties =
        destFunc->getProperties();
    std::shared_ptr<DeclarationNode> functionModule = ASTQuery::findDeclaration(
        destFunc->getName(), {{nullptr, scopeNodes}}, nullptr);
    if (functionModule) {
      if (functionModule->getObjectType() == "module" ||
          functionModule->getObjectType() == "reaction" ||
          functionModule->getObjectType() == "loop") {
        std::vector<ASTNode> typeProperties =
            functionModule->getPropertyValue("ports")->getChildren();
        if (!functionModule->getPropertyValue("ports")) {
          std::cerr << "ERROR: fillDefaultProperties() No type definition for "
                    << destFunc->getName() << std::endl;
          return;
        }
        for (const auto &property : blockProperties) {
          fillDefaultPropertiesForNode(property->getValue(), scopeNodes);
        }

        for (const ASTNode &propertyListMember : typeProperties) {
          assert(propertyListMember->getNodeType() == AST::Declaration);
          DeclarationNode *propertyDecl =
              static_cast<DeclarationNode *>(propertyListMember.get());

          if (propertyDecl->getObjectType().substr(0, 8) == "property") {
            auto propertyNameValue = propertyDecl->getPropertyValue("name");
            if (propertyNameValue &&
                propertyNameValue->getNodeType() == AST::String) {
              bool propertySet = false;
              std::string propertyName =
                  std::static_pointer_cast<ValueNode>(propertyNameValue)
                      ->getStringValue();
              for (const auto &blockProperty : blockProperties) {
                if (blockProperty->getName() == propertyName) {
                  propertySet = true;
                  break;
                }
              }
              if (!propertySet) {
                ASTNode defaultValueNode =
                    propertyDecl->getPropertyValue("default");
                if (defaultValueNode) {
                  std::shared_ptr<PropertyNode> newProperty =
                      std::make_shared<PropertyNode>(
                          propertyName, defaultValueNode,
                          propertyDecl->getFilename().data(),
                          propertyDecl->getLine());
                  destFunc->addProperty(newProperty);
                }
              }
            }
          }
        }

      } else if (functionModule->getObjectType() == "platformModule") {
        // Is there anything to do here for platform modules?
      }
    }
  } else if (node->getNodeType() == AST::List ||
             node->getNodeType() == AST::Expression) {
    ListNode *list = static_cast<ListNode *>(node.get());
    for (const ASTNode &listElement : list->getChildren()) {
      ASTFunctions::fillDefaultPropertiesForNode(listElement, scopeNodes);
    }
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *stream = static_cast<StreamNode *>(node.get());
    for (const ASTNode &streamElement : stream->getChildren()) {
      fillDefaultPropertiesForNode(streamElement, scopeNodes);
    }
  }
}

void ASTFunctions::processAnoymousDeclarations(ASTNode tree) {
  auto newDecls = ASTFunctions::processAnonDeclsForScope(tree->getChildren());

  for (const auto &node : newDecls) {
    tree->addChild(node);
  }
}

void ASTFunctions::resolveConstantsInNode(ASTNode node, ScopeStack scope,
                                          ASTNode tree) {
  if (node->getNodeType() == AST::Stream) {
    std::shared_ptr<StreamNode> stream =
        std::static_pointer_cast<StreamNode>(node);
    resolveConstantsInNode(stream->getLeft(), scope, tree);
    if (stream->getLeft()->getNodeType() == AST::Expression) {
      std::shared_ptr<ExpressionNode> expr =
          std::static_pointer_cast<ExpressionNode>(stream->getLeft());
      std::shared_ptr<ValueNode> newValue =
          reduceConstExpression(expr, scope, tree);
      if (newValue) {
        stream->setLeft(newValue);
      }
    } else if (stream->getLeft()->getNodeType() == AST::PortProperty) {
      std::shared_ptr<PortPropertyNode> propertyNode =
          std::static_pointer_cast<PortPropertyNode>(stream->getLeft());
      std::shared_ptr<DeclarationNode> block =
          ASTQuery::findDeclaration(propertyNode->getPortName(), scope, tree);
      if (block) {
        ASTNode property = block->getPropertyValue(propertyNode->getName());
        if (property) { // First replace if pointing to a name
          if (property->getNodeType() == AST::Block ||
              property->getNodeType() == AST::Bundle) {
            stream->setLeft(property);
          }
          std::shared_ptr<ValueNode> newValue =
              ASTFunctions::resolveConstant(stream->getLeft(), scope, tree);
          if (newValue) {
            stream->setLeft(newValue);
          }
        }
      }
    }
    resolveConstantsInNode(stream->getRight(), scope, tree);
  } else if (node->getNodeType() == AST::Function) {
    std::shared_ptr<FunctionNode> func =
        std::static_pointer_cast<FunctionNode>(node);
    std::vector<std::shared_ptr<PropertyNode>> properties =
        func->getProperties();
    for (auto property : properties) {
      std::shared_ptr<ValueNode> newValue =
          ASTFunctions::resolveConstant(property->getValue(), scope, tree);
      if (newValue) {
        property->replaceValue(newValue);
      }
    }
  } else if (node->getNodeType() == AST::Declaration) {
    std::shared_ptr<DeclarationNode> decl =
        std::static_pointer_cast<DeclarationNode>(node);
    std::vector<std::shared_ptr<PropertyNode>> properties =
        decl->getProperties();
    std::shared_ptr<ListNode> internalBlocks =
        std::static_pointer_cast<ListNode>(decl->getPropertyValue("blocks"));
    if ((decl->getObjectType() == "module" ||
         decl->getObjectType() == "reaction" ||
         decl->getObjectType() == "loop") &&
        internalBlocks) {
      if (internalBlocks->getNodeType() == AST::List) {
        auto blocks = internalBlocks->getChildren();
        scope.push_back({node, blocks});
      }
    }
    // FIXME This is a hack to protect constants that are context dependent.
    // Should the language mark this?
    //    if (decl->getObjectType() != "type") {

    auto frameworkNode = node->getCompilerProperty("framework");
    std::string frameworkName;
    if (frameworkNode) {
      frameworkName =
          std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }
    //    if (frameworkName == "") {
    //      // Now try to see if domain belongs to a framework

    //      auto domainId =
    //          CodeValidator::getNodeDomainName(node, ScopeStack(), tree);

    //      frameworkName = CodeValidator::getFrameworkForDomain(domainId,
    //      tree);
    //    }

    for (std::shared_ptr<PropertyNode> property : properties) {
      resolveConstantsInNode(property->getValue(), scope, tree);
      std::shared_ptr<ValueNode> newValue =
          resolveConstant(property->getValue(), scope, tree, frameworkName);
      if (!newValue) {
        // try to resolve on global namespace
        newValue =
            ASTFunctions::resolveConstant(property->getValue(), scope, tree);
      }
      if (newValue) {
        property->replaceValue(newValue);
      }
    }
    //    }
  } else if (node->getNodeType() == AST::BundleDeclaration) {
    std::shared_ptr<DeclarationNode> decl =
        std::static_pointer_cast<DeclarationNode>(node);
    std::vector<std::shared_ptr<PropertyNode>> properties =
        decl->getProperties();
    std::shared_ptr<ListNode> internalBlocks =
        std::static_pointer_cast<ListNode>(decl->getPropertyValue("blocks"));
    if (internalBlocks) {
      if (internalBlocks->getNodeType() == AST::List) {
        auto blocks = internalBlocks->getChildren();
        scope.push_back({node, blocks});
      }
    }

    auto frameworkNode = node->getCompilerProperty("framework");
    std::string frameworkName;
    if (frameworkNode) {
      frameworkName =
          std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
    }

    for (std::shared_ptr<PropertyNode> property : properties) {
      resolveConstantsInNode(property->getValue(), scope, tree);
      std::shared_ptr<ValueNode> newValue = ASTFunctions::resolveConstant(
          property->getValue(), scope, tree, frameworkName);
      if (!newValue) {
        // try to resolve on global namespace
        newValue = ASTFunctions::resolveConstant(property->getValue(), scope,
                                                 tree, frameworkName);
      }
      if (newValue) {
        property->replaceValue(newValue);
      }
    }
    std::shared_ptr<BundleNode> bundle = decl->getBundle();
    resolveConstantsInNode(bundle->index(), scope, tree);
  } else if (node->getNodeType() == AST::Expression) {
    std::shared_ptr<ExpressionNode> expr =
        std::static_pointer_cast<ExpressionNode>(node);
    if (expr->isUnary()) {
      resolveConstantsInNode(expr->getValue(), scope, tree);
      if (expr->getValue()->getNodeType() == AST::Expression) {
        std::shared_ptr<ExpressionNode> exprValue =
            std::static_pointer_cast<ExpressionNode>(expr->getValue());
        std::shared_ptr<ValueNode> newValue =
            reduceConstExpression(exprValue, scope, tree);
        if (newValue) {
          exprValue->replaceValue(newValue);
        }
      }
    } else {
      resolveConstantsInNode(expr->getLeft(), scope, tree);
      resolveConstantsInNode(expr->getRight(), scope, tree);
      if (expr->getLeft()->getNodeType() == AST::Expression) {
        std::shared_ptr<ExpressionNode> exprValue =
            std::static_pointer_cast<ExpressionNode>(expr->getLeft());
        std::shared_ptr<ValueNode> newValue =
            reduceConstExpression(exprValue, scope, tree);
        if (newValue) {
          expr->replaceLeft(newValue);
        }
      }
      if (expr->getRight()->getNodeType() == AST::Expression) {
        std::shared_ptr<ExpressionNode> exprValue =
            std::static_pointer_cast<ExpressionNode>(expr->getRight());
        std::shared_ptr<ValueNode> newValue =
            reduceConstExpression(exprValue, scope, tree);
        if (newValue) {
          expr->replaceRight(newValue);
        }
      }
    }
  } else if (node->getNodeType() == AST::List) {
    std::map<ASTNode, ASTNode> replaceMap;
    for (ASTNode element : node->getChildren()) {
      resolveConstantsInNode(element, scope, tree);
      std::shared_ptr<ValueNode> newValue;
      if (element->getNodeType() == AST::Expression) {
        newValue = reduceConstExpression(
            std::static_pointer_cast<ExpressionNode>(element), scope, tree);

      } else {
        newValue = ASTFunctions::resolveConstant(element, scope, tree);
      }
      if (newValue) {
        replaceMap[element] = newValue;
      }
    }
    std::shared_ptr<ListNode> list = std::static_pointer_cast<ListNode>(node);
    for (auto &values : replaceMap) {
      list->replaceMember(values.second, values.first);
    }

  } else if (node->getNodeType() == AST::Bundle) {
    std::shared_ptr<BundleNode> bundle =
        std::static_pointer_cast<BundleNode>(node);
    std::shared_ptr<ListNode> index = bundle->index();
    resolveConstantsInNode(index, scope, tree);
  }
}

std::shared_ptr<ValueNode>
ASTFunctions::resolveConstant(ASTNode value, ScopeStack scope, ASTNode tree,
                              std::string framework) {
  std::shared_ptr<ValueNode> newValue = nullptr;
  if (value->getNodeType() == AST::Expression) {
    std::shared_ptr<ExpressionNode> expr =
        std::static_pointer_cast<ExpressionNode>(value);
    newValue = reduceConstExpression(expr, scope, tree);
    if (newValue) {
      newValue->setCompilerProperty("resolvedFrom", expr);
    }
    return newValue;
  } else if (value->getNodeType() == AST::Block) {
    BlockNode *name = static_cast<BlockNode *>(value.get());
    std::shared_ptr<DeclarationNode> block = ASTQuery::findDeclaration(
        name->getName(), scope, tree, name->getNamespaceList(), framework);
    if (block && block->getNodeType() == AST::Declaration &&
        block->getObjectType() == "constant") { // Size == 1
      //            string namespaceValue = name->getScopeAt(0);
      ASTNode declarationNamespace = block->getPropertyValue("namespace");
      //            if (namespaceValue.size() == 0 || namespaceValue)
      ASTNode blockValue = block->getPropertyValue("value");
      if (blockValue->getNodeType() == AST::Int ||
          blockValue->getNodeType() == AST::Real ||
          blockValue->getNodeType() == AST::String) {
        blockValue = blockValue->deepCopy();
        blockValue->setCompilerProperty("resolvedFrom", block);
        return std::static_pointer_cast<ValueNode>(blockValue);
      }
      newValue = ASTFunctions::resolveConstant(block->getPropertyValue("value"),
                                               scope, tree, framework);
      return newValue;
    }
  } else if (value->getNodeType() == AST::Bundle) {
    // What does this mean??
  } else if (value->getNodeType() == AST::PortProperty) {
    PortPropertyNode *propertyNode =
        static_cast<PortPropertyNode *>(value.get());
    std::shared_ptr<DeclarationNode> block =
        ASTQuery::findDeclaration(propertyNode->getPortName(), scope, tree);
    if (block) {
      ASTNode propertyValue = block->getPropertyValue(propertyNode->getName());
      if (propertyValue) {
        //                || propertyValue->getNodeType() == AST::Block ||
        //                propertyValue->getNodeType() == AST::Bundle
        if (propertyValue->getNodeType() == AST::Int ||
            propertyValue->getNodeType() == AST::Real ||
            propertyValue->getNodeType() == AST::String) {
          propertyValue = propertyValue->deepCopy();
          propertyValue->setCompilerProperty("resolvedFrom", block);
          return std::static_pointer_cast<ValueNode>(propertyValue);
        }
      }
    }
  }
  return nullptr;
}

std::shared_ptr<ValueNode>
ASTFunctions::reduceConstExpression(std::shared_ptr<ExpressionNode> expr,
                                    ScopeStack scope, ASTNode tree) {
  ASTNode left = nullptr, right = nullptr;
  bool isConstant;

  if (!expr->isUnary()) {
    left = expr->getLeft();
  } else {
    left = expr->getValue();
  }

  std::shared_ptr<ValueNode> newValue = resolveConstant(left, scope, tree);
  if (newValue) {
    if (expr->isUnary()) {
      expr->replaceValue(newValue);
    } else {
      expr->replaceLeft(newValue);
    }
    left = newValue;
  }
  if (!expr->isUnary()) {
    right = expr->getRight();
    newValue = resolveConstant(right, scope, tree);
    if (newValue) {
      expr->replaceRight(newValue);
      right = newValue;
    }
    isConstant =
        (left->getNodeType() == AST::Int || left->getNodeType() == AST::Real) &&
        (right->getNodeType() == AST::Int || right->getNodeType() == AST::Real);
  } else {
    isConstant =
        (left->getNodeType() == AST::Int || left->getNodeType() == AST::Real);
  }

  if (isConstant) {
    std::shared_ptr<ValueNode> result = nullptr;
    switch (expr->getExpressionType()) {
    case ExpressionNode::Multiply:
      result = ASTRuntime::multiply(std::static_pointer_cast<ValueNode>(left),
                                    std::static_pointer_cast<ValueNode>(right));
      break;
    case ExpressionNode::Divide:
      result = ASTRuntime::divide(std::static_pointer_cast<ValueNode>(left),
                                  std::static_pointer_cast<ValueNode>(right));
      break;
    case ExpressionNode::Add:
      result = ASTRuntime::add(std::static_pointer_cast<ValueNode>(left),
                               std::static_pointer_cast<ValueNode>(right));
      break;
    case ExpressionNode::Subtract:
      result = ASTRuntime::subtract(std::static_pointer_cast<ValueNode>(left),
                                    std::static_pointer_cast<ValueNode>(right));
      break;
    case ExpressionNode::And:
      result =
          ASTRuntime::logicalAnd(std::static_pointer_cast<ValueNode>(left),
                                 std::static_pointer_cast<ValueNode>(right));
      break;
    case ExpressionNode::Or:
      result =
          ASTRuntime::logicalOr(std::static_pointer_cast<ValueNode>(left),
                                std::static_pointer_cast<ValueNode>(right));
      break;
    case ExpressionNode::UnaryMinus:
      result =
          ASTRuntime::unaryMinus(std::static_pointer_cast<ValueNode>(left));
      break;
    case ExpressionNode::LogicalNot:
      result =
          ASTRuntime::logicalNot(std::static_pointer_cast<ValueNode>(left));
      break;
    default:
      assert(0 == 1); // Should never get here
      break;
    }
    if (result) {
      return result;
    }
  }
  return nullptr;
}

std::vector<ASTNode>
ASTFunctions::processAnonDeclsForScope(const std::vector<ASTNode> scopeTree) {
  std::vector<ASTNode> newDecls;
  for (const auto &node : scopeTree) {
    if (node->getNodeType() == AST::Stream) {
      auto streamDecls =
          extractStreamDeclarations(std::static_pointer_cast<StreamNode>(node));
      newDecls.insert(newDecls.end(), streamDecls.begin(), streamDecls.end());
    } else if (node->getNodeType() == AST::Declaration ||
               node->getNodeType() == AST::BundleDeclaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "reaction" ||
          decl->getObjectType() == "module" ||
          decl->getObjectType() == "loop") {
        auto streams = decl->getPropertyValue("streams");
        auto blocks = decl->getPropertyValue("blocks");
        //          Q_ASSERT(streams && blocks);
        if (streams && blocks) {

          auto streamDecls = processAnonDeclsForScope(streams->getChildren());
          for (const auto &node : streamDecls) {
            blocks->addChild(node);
          }

          auto newInternalBlocks =
              ASTFunctions::processAnonDeclsForScope(blocks->getChildren());
          for (const auto &node : newInternalBlocks) {
            blocks->addChild(node);
          }
        }
      }
    }
  }

  return newDecls;
}

std::vector<std::shared_ptr<DeclarationNode>>
ASTFunctions::extractStreamDeclarations(std::shared_ptr<StreamNode> stream) {
  std::vector<std::shared_ptr<DeclarationNode>> streamDeclarations;
  auto node = stream->getLeft();
  do {
    if (node->getNodeType() == AST::Declaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      auto newBlock = std::make_shared<FunctionNode>(
          decl->getName(), std::make_shared<ListNode>(__FILE__, __LINE__),
          __FILE__, __LINE__);
      streamDeclarations.push_back(decl);
      if (node == stream->getLeft()) {
        stream->setLeft(newBlock);
      } else {
        stream->setRight(newBlock);
        break;
      }
    }
    if (stream->getRight() == node) {
      node = nullptr;
    } else if (stream->getRight()->getNodeType() == AST::Stream) {
      stream = std::static_pointer_cast<StreamNode>(stream->getRight());
      node = stream->getLeft();
    } else {
      node = stream->getRight();
    }
  } while (node);
  return streamDeclarations;
}

bool ASTFunctions::resolveInherits(std::shared_ptr<DeclarationNode> decl,
                                   ASTNode tree) {
  auto inheritsNode = decl->getPropertyValue("inherits");
  if (inheritsNode && inheritsNode->getNodeType() == AST::Block) {
    auto inheritedName =
        std::static_pointer_cast<BlockNode>(inheritsNode)->getName();
    auto inheritedDecl = ASTQuery::findDeclarationWithType(
        inheritedName, decl->getObjectType(), ScopeStack(), tree);
    if (inheritedDecl) {
      for (const auto &prop : inheritedDecl->getProperties()) {
        if (!decl->getPropertyValue(prop->getName())) {
          decl->setPropertyValue(prop->getName(), prop->getValue());
        }
      }
      return true;
    } else {
      std::cerr << __FILE__ << ":" << __LINE__
                << " ERROR could not resolve inheritance: " << inheritedName
                << std::endl;
      return true;
    }
  } else {
    return false;
  }
}
