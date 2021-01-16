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

#include <QDebug>
#include <QDir>
#include <QStringList>

#include "ast.h"
#include "valuenode.h"

#include "coderesolver.h"
#include "codevalidator.h"
#include "stridelibrary.hpp"
#include "stridesystem.hpp"

StrideLibrary::StrideLibrary() : m_majorVersion(1), m_minorVersion(0) {}

StrideLibrary::~StrideLibrary() {}

void StrideLibrary::initializeLibrary(QString strideRootPath) {
  m_libraryTrees.clear();
  readLibrary(strideRootPath);
}

std::shared_ptr<DeclarationNode>
StrideLibrary::findTypeInLibrary(QString typeName) {
  for (auto libraryTree : m_libraryTrees) {
    auto nodes = libraryTree.nodes;
    for (ASTNode node : nodes) {
      if (node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> block =
            static_pointer_cast<DeclarationNode>(node);
        if (block->getObjectType() != "type") {
          continue;
        }
        ASTNode value = block->getPropertyValue("typeName");
        if (value->getNodeType() == AST::String) {
          QString libTypeName = QString::fromStdString(
              static_pointer_cast<ValueNode>(value)->getStringValue());
          if (libTypeName == typeName) {
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
      findTypeInLibrary(QString::fromStdString(block->getObjectType()));
  if (type) {
    if (block->getProperties().size() == 0) {
      return true; // FIXME we need to check if properties are required
    }
    for (std::shared_ptr<PropertyNode> property : block->getProperties()) {
      if (isValidProperty(property, type.get())) {
        return true;
      }
      // Now check for inherited properties
      QList<DeclarationNode *> parentTypes = getParentTypes(block);
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
  for (auto libraryTree : m_libraryTrees) {
    if (libNamespace.find(libraryTree.importAs) == libNamespace.end()) {
      libNamespace[libraryTree.importAs] = std::vector<ASTNode>();
    }
    for (ASTNode node : libraryTree.nodes) {
      libNamespace[libraryTree.importAs].push_back(node);
    }
  }
  return libNamespace;
}

std::vector<ASTNode> StrideLibrary::loadImport(std::string importName,
                                               std::string importAs) {

  std::string path = m_libraryPath.toStdString();
  if (importName.size() > 0) {
    path += "/" + importName;
  }
  // FIXME support namespace as file name
  // TODO add warning if local file/directory shadows system one
  // For library we don't need support for file name namespace at the root, but
  // there needs to be support for nested files.

  auto newNodes = CodeValidator::loadAllInDirectory(path);
  // FIXME support importing library files with multiple different importAs.
  // Currently broken if library imported multiple times with different alias.
  for (auto node : newNodes) {
    if (importAs.size() > 0) {
      node->appendToPropertyValue(
          "namespaceTree",
          std::make_shared<ValueNode>(importAs, __FILE__, __LINE__));
    }
  }
  m_libraryTrees.push_back(LibraryTree{importName, importAs, newNodes, {}});
  return newNodes;
}

bool StrideLibrary::isValidProperty(std::shared_ptr<PropertyNode> property,
                                    DeclarationNode *type) {
  Q_ASSERT(type->getObjectType() == "type");
  std::shared_ptr<PropertyNode> portsInType =
      CodeValidator::findPropertyByName(type->getProperties(), "properties");
  ListNode *portList = static_cast<ListNode *>(portsInType->getValue().get());
  Q_ASSERT(portList->getNodeType() == AST::List);
  for (ASTNode port : portList->getChildren()) {
    DeclarationNode *portBlock = static_cast<DeclarationNode *>(port.get());
    Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
    Q_ASSERT(portBlock->getObjectType() == "typeProperty");
    std::shared_ptr<PropertyNode> portName =
        CodeValidator::findPropertyByName(portBlock->getProperties(), "name");
    string portNameInType =
        static_cast<ValueNode *>(portName->getValue().get())->getStringValue();
    if (property->getName() == portNameInType) {
      return true;
    }
  }
  std::shared_ptr<PropertyNode> inherits =
      CodeValidator::findPropertyByName(type->getProperties(), "inherits");
  if (inherits) {
    ValueNode *inheritedTypeName =
        static_cast<ValueNode *>(inherits->getValue().get());
    Q_ASSERT(inheritedTypeName->getNodeType() == AST::String);
    std::shared_ptr<DeclarationNode> inheritedType = findTypeInLibrary(
        QString::fromStdString(inheritedTypeName->getStringValue()));
    Q_ASSERT(inheritedType != nullptr);
    if (isValidProperty(property, inheritedType.get())) {
      return true;
    }
  }
  return false;
}

QList<DeclarationNode *> StrideLibrary::getParentTypes(DeclarationNode *type) {
  std::shared_ptr<PropertyNode> inheritProperty =
      CodeValidator::findPropertyByName(type->getProperties(), "inherits");
  if (inheritProperty) {
    ASTNode parentType = inheritProperty->getValue();
    if (parentType->getNodeType() == AST::String) {
      string parentBlockName =
          static_cast<ValueNode *>(parentType.get())->getStringValue();
      std::shared_ptr<DeclarationNode> parentBlock =
          findTypeInLibrary(QString::fromStdString(parentBlockName));
      return getParentTypes(parentBlock.get());
    }
  }
  return QList<DeclarationNode *>();
}

void StrideLibrary::readLibrary(QString rootDir) {
  QString basepath =
      QString("/library/%1.%2").arg(m_majorVersion).arg(m_minorVersion);
  //    QMapIterator<QString, QStringList> it(importList);
  //    importList[""] = QStringList() << ""; // Add root namespace
  m_libraryPath = rootDir + basepath;

  loadImport("", "");
}
