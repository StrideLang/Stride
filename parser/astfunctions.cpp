#include "astfunctions.h"
#include "astquery.h"
#include "blocknode.h"
#include "expressionnode.h"
#include "functionnode.h"
#include "streamnode.h"
#include "valuenode.h"

#include <algorithm>
#include <iostream>
#include <vector>

extern AST *parse(const char *fileName, const char *sourceFilename);
extern std::vector<LangError> getErrors();

ASTNode ASTFunctions::parseFile(const char *fileName,
                                const char *sourceFilename) {
  return std::shared_ptr<AST>(parse(fileName, sourceFilename));
}

std::vector<LangError> ASTFunctions::getParseErrors() { return getErrors(); }

void ASTFunctions::insertBuiltinObjects(
    ASTNode tree, std::map<std::string, std::vector<ASTNode>> importTrees) {
  auto children = tree->getChildren();
  for (ASTNode object : children) {
    insertBuiltinObjectsForNode(object, importTrees, tree);
  }
}

void ASTFunctions::insertDependentTypes(
    std::shared_ptr<DeclarationNode> typeDeclaration,
    std::map<std::string, std::vector<ASTNode>> &objects, ASTNode tree) {
  std::vector<std::shared_ptr<DeclarationNode>> blockList;
  //    std::shared_ptr<DeclarationNode> existingDecl =
  //    CodeValidator::findTypeDeclaration(typeDeclaration, ScopeStack(),
  //    m_tree);
  for (auto it = objects.begin(); it != objects.end(); it++) {
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
      insertDependentTypes(inheritedType, objects, tree);
    }
  }
}

void ASTFunctions::insertBuiltinObjectsForNode(
    ASTNode node, std::map<std::string, std::vector<ASTNode>> &objects,
    ASTNode tree, std::string currentFramework) {
  std::vector<std::shared_ptr<DeclarationNode>> blockList;
  if (node->getNodeType() == AST::List) {
    for (ASTNode child : node->getChildren()) {
      insertBuiltinObjectsForNode(child, objects, tree, currentFramework);
    }
  } else if (node->getNodeType() == AST::Stream) {
    StreamNode *stream = static_cast<StreamNode *>(node.get());
    insertBuiltinObjectsForNode(stream->getLeft(), objects, tree);
    insertBuiltinObjectsForNode(stream->getRight(), objects, tree);
  } else if (node->getNodeType() == AST::Expression) {
    ExpressionNode *expr = static_cast<ExpressionNode *>(node.get());
    if (expr->isUnary()) {
      insertBuiltinObjectsForNode(expr->getValue(), objects, tree);
    } else {
      insertBuiltinObjectsForNode(expr->getLeft(), objects, tree);
      insertBuiltinObjectsForNode(expr->getRight(), objects, tree);
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
      insertBuiltinObjectsForNode(property->getValue(), objects, tree);
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
      insertBuiltinObjectsForNode(usedBlock, objects, tree, fw);
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
        insertBuiltinObjectsForNode(typeDecl, objects, tree);
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
          insertBuiltinObjectsForNode(property->getValue(), objects, tree,
                                      framework);
        }
        // Process index for bundle declarations
        if (node->getNodeType() == AST::BundleDeclaration) {
          insertBuiltinObjectsForNode(declaration->getBundle()->index(),
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
        insertBuiltinObjectsForNode(usedBlock, objects, tree);
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
        insertBuiltinObjectsForNode(usedBlock, objects, tree);
      }
    }
  } else if (node->getNodeType() == AST::Property) {
    std::shared_ptr<PropertyNode> prop =
        std::static_pointer_cast<PropertyNode>(node);
    insertBuiltinObjectsForNode(prop->getValue(), objects, tree);
  }
}

bool ASTFunctions::resolveInherits(std::shared_ptr<DeclarationNode> decl,
                                   ASTNode tree) {
  auto inheritsNode = decl->getPropertyValue("inherits");
  if (inheritsNode && inheritsNode->getNodeType() == AST::Block) {
    auto inheritedName =
        std::static_pointer_cast<BlockNode>(inheritsNode)->getName();
    auto inheritedDecl = ASTQuery::findDeclarationWithType(
        inheritedName, decl->getObjectType(), tree);
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
