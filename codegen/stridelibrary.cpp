
#include <QStringList>
#include <QDir>
#include <QDebug>

#include "ast.h"

#include "stridelibrary.hpp"

StrideLibrary::StrideLibrary(QString libraryPath)
{
    readLibrary(libraryPath);
}


void StrideLibrary::readLibrary(QString rootDir)
{
    QStringList nameFilters;
    nameFilters << "*.stride";
    QStringList libraryFiles =  QDir(rootDir + "/library").entryList(nameFilters);
    qDebug() << libraryFiles;
    foreach (QString file, libraryFiles) {
        QString fileName = rootDir + "/library/" + file;
        AST *tree = AST::parseFile(fileName.toLocal8Bit().data());
        if(tree) {
            m_platformTrees.append(tree);
        }
    }
}
