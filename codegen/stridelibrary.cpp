
#include <QStringList>
#include <QDir>
#include <QDebug>

#include "ast.h"
#include "valuenode.h"

#include "stridelibrary.hpp"

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
                    if (propertyName == "type") {
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
