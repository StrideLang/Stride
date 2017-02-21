
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
    foreach(AST * node, m_libraryTrees) {
        node->deleteChildren();
        delete node;
    }
}

void StrideLibrary::setLibraryPath(QString libraryPath, QMap<QString, QString> importList)
{
    foreach(AST *node, m_libraryTrees) {
        node->deleteChildren();
        delete node;
    }
    m_libraryTrees.clear();

    readLibrary(libraryPath, importList);
}

DeclarationNode *StrideLibrary::findTypeInLibrary(QString typeName)
{
    foreach (AST *rootNode, m_libraryTrees) {
        foreach (AST *node, rootNode->getChildren()) {
            if (node->getNodeType() == AST::Declaration) {
                DeclarationNode *block = static_cast<DeclarationNode *>(node);
                if (block->getObjectType() != "type") {
                    continue;
                }
                AST * value = block->getPropertyValue("typeName");
                if (value->getNodeType()  == AST::String) {
                    QString libTypeName = QString::fromStdString(static_cast<ValueNode *>(value)->getStringValue());
                    if (libTypeName == typeName) {
                        return block;
                    }
                }
            }
        }
    }
    return NULL;
}

bool StrideLibrary::isValidBlock(DeclarationNode *block)
{
    DeclarationNode *type = findTypeInLibrary(QString::fromStdString(block->getObjectType()));
    if (type) {
        if (block->getProperties().size() == 0) {
            return true; // FIXME we need to check if properties are required
        }
        foreach(PropertyNode *property, block->getProperties()) {
            if (isValidProperty(property, type)) {
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

std::vector<AST *> StrideLibrary::getNodes()
{
    std::vector<AST *> nodes;
    foreach(AST *tree, m_libraryTrees) {
        foreach(AST *node, tree->getChildren()) {
            nodes.push_back(node);
        }
    }
    return nodes;
}

bool StrideLibrary::isValidProperty(PropertyNode *property, DeclarationNode *type)
{
    Q_ASSERT(type->getObjectType() == "type");
    PropertyNode *portsInType = CodeValidator::findPropertyByName(type->getProperties(), "properties");
    ListNode *portList = static_cast<ListNode *>(portsInType->getValue());
    Q_ASSERT(portList->getNodeType() == AST::List);
    foreach(AST * port, portList->getChildren()) {
        DeclarationNode *portBlock = static_cast<DeclarationNode *>(port);
        Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
        Q_ASSERT(portBlock->getObjectType() == "typeProperty");
        PropertyNode *portName = CodeValidator::findPropertyByName(portBlock->getProperties(), "name");
        string portNameInType = static_cast<ValueNode *>(portName->getValue())->getStringValue();
        if(property->getName() == portNameInType) {
            return true;
        }
    }
    PropertyNode *inherits = CodeValidator::findPropertyByName(type->getProperties(), "inherits");
    if (inherits) {
        ValueNode *inheritedTypeName = static_cast<ValueNode *>(inherits->getValue());
        Q_ASSERT(inheritedTypeName->getNodeType() == AST::String);
        DeclarationNode *inheritedType = findTypeInLibrary(QString::fromStdString(inheritedTypeName->getStringValue()));
        Q_ASSERT(inheritedType != NULL);
        if (isValidProperty(property, inheritedType)) {
            return true;
        }
    }
    return false;
}

QList<DeclarationNode *> StrideLibrary::getParentTypes(DeclarationNode *type)
{
    PropertyNode *inheritProperty = CodeValidator::findPropertyByName(type->getProperties(), "inherits");
    if (inheritProperty) {
        AST *parentType = inheritProperty->getValue();
        if (parentType->getNodeType() == AST::String) {
            string parentBlockName = static_cast<ValueNode *>(parentType)->getStringValue();
            DeclarationNode *parentBlock = findTypeInLibrary(QString::fromStdString(parentBlockName));
            return getParentTypes(parentBlock);
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
            AST *tree = AST::parseFile(fileName.toLocal8Bit().data());
            if(tree) {
                QString namespaceName = importList[subPath];
                if (!namespaceName.isEmpty()) {
                    foreach(AST *node, tree->getChildren()) {
                        // Do we need to set namespace recursively or would this do?
                        node->setNamespace(namespaceName.toStdString());
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

