
#include <QStringList>
#include <QDir>
#include <QDebug>

#include "ast.h"
#include "valuenode.h"

#include "stridelibrary.hpp"
#include "codevalidator.h"

StrideLibrary::StrideLibrary(QString libraryPath)
{
    readLibrary(libraryPath);
    readLibraryTypes(libraryPath);
}

StrideLibrary::~StrideLibrary()
{
    foreach(AST * node, m_platformTrees) {
        node->deleteChildren();
        delete node;
    }
    foreach(AST * node, m_platformTypes) {
        node->deleteChildren();
        delete node;
    }
}

BlockNode *StrideLibrary::findTypeInLibrary(QString typeName)
{
    foreach (AST *rootNode, m_platformTypes) {
        foreach (AST *node, rootNode->getChildren()) {
            if (node->getNodeType() == AST::Block) {
                BlockNode *block = static_cast<BlockNode *>(node);
                if (block->getObjectType() != "type") {
                    continue;
                }
                foreach(PropertyNode *property, block->getProperties()) {
                    QString propertyName = QString::fromStdString(property->getName());
                    if (propertyName == "typeName") {
                        AST * value = property->getValue();
                        if (value->getNodeType()  == AST::String) {
                            QString libTypeName = QString::fromStdString(static_cast<ValueNode *>(value)->getStringValue());
                            if (libTypeName == typeName) {
                                return block;
                            }
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

bool StrideLibrary::isValidBlock(BlockNode *block)
{
    BlockNode *type = findTypeInLibrary(QString::fromStdString(block->getObjectType()));
    if (type) {
        foreach(PropertyNode *property, block->getProperties()) {
            if (isValidProperty(property, type)) {
                return true;
            }
            // Now check for inherited properties
            QList<BlockNode *> parentTypes = getParentTypes(block);
            bool propertyInParent = false;
            foreach(BlockNode *parent, parentTypes) {
                propertyInParent |= isValidProperty(property, parent);
            }
            if (propertyInParent) return true;
        }
    }
    return false;
}

bool StrideLibrary::isValidProperty(PropertyNode *property, BlockNode *type)
{
    Q_ASSERT(type->getObjectType() == "type");
    PropertyNode *portsInType = CodeValidator::findPropertyByName(type->getProperties(), "ports");
    ListNode *portList = static_cast<ListNode *>(portsInType->getValue());
    Q_ASSERT(portList->getNodeType() == AST::List);
    foreach(AST * port, portList->getChildren()) {
        BlockNode *portBlock = static_cast<BlockNode *>(port);
        Q_ASSERT(portBlock->getNodeType() == AST::Block);
        Q_ASSERT(portBlock->getObjectType() == "port");
        PropertyNode *portName = CodeValidator::findPropertyByName(portBlock->getProperties(), "name");
        string portNameInType = static_cast<ValueNode *>(portName->getValue())->getStringValue();
        if(property->getName() == portNameInType) {
            return true;
//            PropertyNode *validTypes = CodeValidator::findPropertyByName(portBlock->getProperties(), "types");
//            ListNode *validTypesList = static_cast<ListNode *>(validTypes->getValue());
//            Q_ASSERT(validTypesList->getNodeType() == AST::List);
//            foreach(AST * validType, validTypesList->getChildren()) {
//                Q_ASSERT(validType->getNodeType() == AST::String);
//                QString typeCode = QString::fromStdString(static_cast<ValueNode *>(validType)->getStringValue());
//                AST *value = property->getValue();
//                if (value->getNodeType() == AST::String)
//                if (typeCode == "CSP" && value->getNodeType() == AST::String) {
//                    return true;
//                } else if (typeCode == "CSP" && value->getNodeType() == AST::String) {
//                    return true;
//                } else if (value->getNodeType() == AST::Name) {
//                    return true; // We will validate this later when we know the context
//                }
//            }

        }
    }
    return false;
}

QList<BlockNode *> StrideLibrary::getParentTypes(BlockNode *type)
{
    PropertyNode *inheritProperty = CodeValidator::findPropertyByName(type->getProperties(), "inherits");
    if (inheritProperty) {
        AST *parentType = inheritProperty->getValue();
        if (parentType->getNodeType() == AST::String) {
            string parentBlockName = static_cast<ValueNode *>(parentType)->getStringValue();
            BlockNode *parentBlock = findTypeInLibrary(QString::fromStdString(parentBlockName));
            return getParentTypes(parentBlock);
        }
    }
    return QList<BlockNode *>();
}

void StrideLibrary::readLibrary(QString rootDir)
{
    QStringList nameFilters;
    nameFilters << "*.stride";
    QString subpath = "/library";
    QStringList libraryFiles =  QDir(rootDir + subpath).entryList(nameFilters);
    foreach (QString file, libraryFiles) {
        QString fileName = rootDir + subpath + QDir::separator() + file;
        AST *tree = AST::parseFile(fileName.toLocal8Bit().data());
        if(tree) {
            m_platformTrees.append(tree);
        }
    }
}

void StrideLibrary::readLibraryTypes(QString rootDir)
{
    QStringList nameFilters;
    nameFilters << "*.stride";
    QString subpath = "/library/types";
    QStringList typeFiles =  QDir(rootDir + subpath).entryList(nameFilters);
    foreach (QString file, typeFiles) {
        QString fileName = rootDir + subpath + QDir::separator() + file;
        QString cleanPath = QDir::cleanPath(fileName); // Path contains ../
        AST *tree = AST::parseFile(cleanPath.toLocal8Bit().data());
        if(tree) {
            m_platformTypes.append(tree);
        }
    }
}
