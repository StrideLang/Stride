/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include "stride/codegen/stridelibrary.hpp"
#include "stride/parser/ast.h"
#include "stride/codegen/astfunctions.hpp"
#include "stride/codegen/astquery.hpp"
#include "stride/parser/valuenode.h"

#include <cassert>
#include <filesystem>

StrideLibrary::StrideLibrary() : m_majorVersion(1), m_minorVersion(0) {}

StrideLibrary::~StrideLibrary() {}

void StrideLibrary::initializeLibrary(std::string strideRootPath,
                                      std::vector<std::string> includePaths) {
  m_includePaths = includePaths;
  m_libraryTrees.clear();
  readLibrary(strideRootPath);
}

std::shared_ptr<DeclarationNode>
StrideLibrary::findTypeInLibrary(std::string typeName) {
  for (const auto &libraryTree : m_libraryTrees) {
    auto nodes = libraryTree.nodes;
    for (const ASTNode &node : nodes) {
      if (node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> block =
            std::static_pointer_cast<DeclarationNode>(node);
        if (block->getObjectType() != "type") {
          continue;
        }
        ASTNode value = block->getPropertyValue("typeName");
        if (value->getNodeType() == AST::String) {
          if (std::static_pointer_cast<ValueNode>(value)->getStringValue() ==
              typeName) {
            return block;
          }
        }
      }
    }
  }
  return nullptr;
}

bool StrideLibrary::isValidBlock(DeclarationNode *block) {
  std::shared_ptr<DeclarationNode> type =
      findTypeInLibrary(block->getObjectType());
  if (type) {
    if (block->getProperties().size() == 0) {
      return true; // FIXME we need to check if properties are required
    }
    for (const std::shared_ptr<PropertyNode> &property :
         block->getProperties()) {
      if (isValidProperty(property, type.get())) {
        return true;
      }
      // Now check for inherited properties
      std::vector<DeclarationNode *> parentTypes = getParentTypes(block);
      bool propertyInParent = false;
      for (DeclarationNode *parent : parentTypes) {
        propertyInParent |= isValidProperty(property, parent);
      }
      if (propertyInParent)
        return true;
    }
  }
  return false;
}

std::map<std::string, std::vector<ASTNode>> StrideLibrary::getLibraryMembers() {
  std::map<std::string, std::vector<ASTNode>> libNamespace;
  for (const auto &libraryTree : m_libraryTrees) {
    if (libNamespace.find(libraryTree.importAs) == libNamespace.end()) {
      libNamespace[libraryTree.importAs] = std::vector<ASTNode>();
    }
    for (const ASTNode &node : libraryTree.nodes) {
      libNamespace[libraryTree.importAs].push_back(node);
    }
  }
  return libNamespace;
}

std::vector<ASTNode> StrideLibrary::loadImport(std::string importName,
                                               std::string importAs) {
  std::string path = m_libraryPath;
  if (importName.size() > 0) {

    for (const auto &includePath : m_includePaths) {
      std::filesystem::path p = includePath;
      p += std::filesystem::path::preferred_separator;
      if (std::filesystem::exists(p / importName) &&
          std::filesystem::is_directory(p / importName)) {
        path = (p / importName).string();
        break;
      }
      if (std::filesystem::exists(p / (importName + ".stride"))) {
        path = (p / (importName + ".stride")).string();
        break;
      }
    }
    if (path == m_libraryPath) {

      path += "/" + importName;
    }
  }
  // FIXME support namespace as file name
  // TODO add warning if local file/directory shadows system one
  // For library we don't need support for file name namespace at the root, but
  // there needs to be support for nested files.

  if (std::filesystem::is_directory(path)) {
    auto newNodes = ASTFunctions::loadAllInDirectory(path);
    // FIXME support importing library files with multiple different importAs.
    // Currently broken if library imported multiple times with different alias.
    for (const auto &node : newNodes) {
      if (importAs.size() > 0) {
        node->appendToPropertyValue(
            "namespaceTree",
            std::make_shared<ValueNode>(importAs, __FILE__, __LINE__));
      }
    }
    m_libraryTrees.push_back(LibraryTree{importName, importAs, newNodes, {}});
    return newNodes;
  } else {
    std::vector<ASTNode> nodes;
    auto newTree = AST::parseFile(path.c_str(), nullptr);
    if (newTree) {
      auto children = newTree->getChildren();
      nodes.insert(nodes.end(), children.begin(), children.end());
      m_libraryTrees.push_back(LibraryTree{importName, importAs, nodes, {}});
    }
    return nodes;
  }
}

bool StrideLibrary::isValidProperty(std::shared_ptr<PropertyNode> property,
                                    DeclarationNode *type) {
  assert(type->getObjectType() == "type");
  std::shared_ptr<PropertyNode> portsInType =
      ASTQuery::findPropertyByName(type->getProperties(), "properties");
  ListNode *portList = static_cast<ListNode *>(portsInType->getValue().get());
  assert(portList->getNodeType() == AST::List);
  for (const ASTNode &port : portList->getChildren()) {
    DeclarationNode *portBlock = static_cast<DeclarationNode *>(port.get());
    assert(portBlock->getNodeType() == AST::Declaration);
    assert(portBlock->getObjectType() == "typeProperty");
    std::shared_ptr<PropertyNode> portName =
        ASTQuery::findPropertyByName(portBlock->getProperties(), "name");
    std::string portNameInType =
        static_cast<ValueNode *>(portName->getValue().get())->getStringValue();
    if (property->getName() == portNameInType) {
      return true;
    }
  }
  std::shared_ptr<PropertyNode> inherits =
      ASTQuery::findPropertyByName(type->getProperties(), "inherits");
  if (inherits) {
    ValueNode *inheritedTypeName =
        static_cast<ValueNode *>(inherits->getValue().get());
    assert(inheritedTypeName->getNodeType() == AST::String);
    std::shared_ptr<DeclarationNode> inheritedType =
        findTypeInLibrary(inheritedTypeName->getStringValue());
    assert(inheritedType != nullptr);
    if (isValidProperty(property, inheritedType.get())) {
      return true;
    }
  }
  return false;
}

std::vector<DeclarationNode *>
StrideLibrary::getParentTypes(DeclarationNode *type) {
  std::shared_ptr<PropertyNode> inheritProperty =
      ASTQuery::findPropertyByName(type->getProperties(), "inherits");
  if (inheritProperty) {
    ASTNode parentType = inheritProperty->getValue();
    if (parentType->getNodeType() == AST::String) {
      std::string parentBlockName =
          static_cast<ValueNode *>(parentType.get())->getStringValue();
      std::shared_ptr<DeclarationNode> parentBlock =
          findTypeInLibrary(parentBlockName);
      return getParentTypes(parentBlock.get());
    }
  }
  return std::vector<DeclarationNode *>();
}

void StrideLibrary::readLibrary(std::string rootDir) {
  std::string basepath = "/library/" + std::to_string(m_majorVersion) + "." +
                         std::to_string(m_minorVersion);
  //    QMapIterator<QString, QStringList> it(importList);
  //    importList[""] = QStringList() << ""; // Add root namespace
  m_libraryPath = rootDir + basepath;

  loadImport("", "");
}
