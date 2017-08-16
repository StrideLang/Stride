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

#include <QStringList>
#include <QDir>
#include <QDebug>

#include "ast.h"
#include "valuenode.h"

#include "stridelibrary.hpp"
#include "codevalidator.h"

StrideLibrary::StrideLibrary() :
    m_majorVersion(1), m_minorVersion(0)
{
}

StrideLibrary::StrideLibrary(QString libraryPath, QMap<QString, QString> importList) :
    m_majorVersion(1), m_minorVersion(0)
{
    readLibrary(libraryPath, importList);
}

StrideLibrary::~StrideLibrary()
{
    foreach(ASTNode node, m_libraryTrees) {
//        node->deleteChildren();
//        delete node;
    }
}

void StrideLibrary::setLibraryPath(QString strideRootPath, QMap<QString, QString> importList)
{
//    foreach(ASTNode node, m_libraryTrees) {
//        node->deleteChildren();
////        delete node;
//    }
    m_libraryTrees.clear();

    readLibrary(strideRootPath, importList);
}

std::shared_ptr<DeclarationNode> StrideLibrary::findTypeInLibrary(QString typeName)
{
    for (ASTNode rootNode : m_libraryTrees) {
        for (ASTNode node : rootNode->getChildren()) {
            if (node->getNodeType() == AST::Declaration) {
                std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(node);
                if (block->getObjectType() != "type") {
                    continue;
                }
                ASTNode value = block->getPropertyValue("typeName");
                if (value->getNodeType()  == AST::String) {
                    QString libTypeName = QString::fromStdString(static_pointer_cast<ValueNode>(value)->getStringValue());
                    if (libTypeName == typeName) {
                        return block;
                    }
                }
            }
        }
    }
    return nullptr;
}

bool StrideLibrary::isValidBlock(DeclarationNode *block)
{
    std::shared_ptr<DeclarationNode> type = findTypeInLibrary(QString::fromStdString(block->getObjectType()));
    if (type) {
        if (block->getProperties().size() == 0) {
            return true; // FIXME we need to check if properties are required
        }
        for(std::shared_ptr<PropertyNode> property :block->getProperties()) {
            if (isValidProperty(property, type.get())) {
                return true;
            }
            // Now check for inherited properties
            QList<DeclarationNode *> parentTypes = getParentTypes(block);
            bool propertyInParent = false;
            foreach(DeclarationNode *parent, parentTypes) {
                propertyInParent |= isValidProperty(property, parent);
            }
            if (propertyInParent) return true;
        }
    }
    return false;
}

std::vector<ASTNode> StrideLibrary::getLibraryMembers()
{
    std::vector<ASTNode> nodes;
    for (ASTNode tree :m_libraryTrees) {
        for(ASTNode node : tree->getChildren()) {
            nodes.push_back(node);
        }
    }
    return nodes;
}

bool StrideLibrary::isValidProperty(std::shared_ptr<PropertyNode> property, DeclarationNode *type)
{
    Q_ASSERT(type->getObjectType() == "type");
    std::shared_ptr<PropertyNode> portsInType = CodeValidator::findPropertyByName(type->getProperties(), "properties");
    ListNode *portList = static_cast<ListNode *>(portsInType->getValue().get());
    Q_ASSERT(portList->getNodeType() == AST::List);
    for (ASTNode port : portList->getChildren()) {
        DeclarationNode *portBlock = static_cast<DeclarationNode *>(port.get());
        Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
        Q_ASSERT(portBlock->getObjectType() == "typeProperty");
        std::shared_ptr<PropertyNode> portName = CodeValidator::findPropertyByName(portBlock->getProperties(), "name");
        string portNameInType = static_cast<ValueNode *>(portName->getValue().get())->getStringValue();
        if(property->getName() == portNameInType) {
            return true;
        }
    }
    std::shared_ptr<PropertyNode> inherits = CodeValidator::findPropertyByName(type->getProperties(), "inherits");
    if (inherits) {
        ValueNode *inheritedTypeName = static_cast<ValueNode *>(inherits->getValue().get());
        Q_ASSERT(inheritedTypeName->getNodeType() == AST::String);
        std::shared_ptr<DeclarationNode> inheritedType = findTypeInLibrary(QString::fromStdString(inheritedTypeName->getStringValue()));
        Q_ASSERT(inheritedType != nullptr);
        if (isValidProperty(property, inheritedType.get())) {
            return true;
        }
    }
    return false;
}

QList<DeclarationNode *> StrideLibrary::getParentTypes(DeclarationNode *type)
{
    std::shared_ptr<PropertyNode> inheritProperty = CodeValidator::findPropertyByName(type->getProperties(), "inherits");
    if (inheritProperty) {
        ASTNode parentType = inheritProperty->getValue();
        if (parentType->getNodeType() == AST::String) {
            string parentBlockName = static_cast<ValueNode *>(parentType.get())->getStringValue();
            std::shared_ptr<DeclarationNode> parentBlock = findTypeInLibrary(QString::fromStdString(parentBlockName));
            return getParentTypes(parentBlock.get());
        }
    }
    return QList<DeclarationNode *>();
}

void StrideLibrary::readLibrary(QString rootDir, QMap<QString, QString> importList)
{
    QStringList nameFilters;
    nameFilters << "*.stride";
    QString basepath = QString("/library/%1.%2").arg(m_majorVersion).arg(m_minorVersion);
    QStringList subPaths;
    subPaths << "";
    QMapIterator<QString, QString> it(importList);
    while (it.hasNext()) {
        it.next();
        subPaths << it.key();
    }
    foreach(QString subPath, subPaths) {
        QStringList libraryFiles =  QDir(rootDir + basepath + QDir::separator() + subPath).entryList(nameFilters);
        foreach (QString file, libraryFiles) {
            QString fileName = rootDir + basepath + QDir::separator() + subPath + QDir::separator() + file;
            ASTNode tree = AST::parseFile(fileName.toLocal8Bit().data(), nullptr);
            if(tree) {
                QString namespaceName = importList[subPath];
                if (!namespaceName.isEmpty()) {
                    for(ASTNode node : tree->getChildren()) {
                        // Do we need to set namespace recursively or would this do?
//                        node->setNamespace(namespaceName.toStdString());
                    }
                }
                m_libraryTrees.append(tree);
            } else {
                qDebug() << "Not loaded:" << fileName;
                vector<LangError> errors = AST::getParseErrors();
                foreach(LangError error, errors) {
                    qDebug() << QString::fromStdString(error.getErrorText());
                }
            }
        }
    }
}

