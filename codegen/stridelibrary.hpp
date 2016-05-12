#ifndef STRIDELIBRARY_HPP
#define STRIDELIBRARY_HPP

#include <vector>

#include <QString>
#include <QList>
#include <QMap>

#include "blocknode.h"
#include "langerror.h"

class StrideLibrary
{
public:
    StrideLibrary();
    StrideLibrary(QString libraryPath, QMap<QString,QString> importList = QMap<QString,QString>());
   ~StrideLibrary();

    void setLibraryPath(QString libraryPath, QMap<QString,QString> importList = QMap<QString,QString>());

    BlockNode *findTypeInLibrary(QString typeName);

    bool isValidBlock(BlockNode *block);

    std::vector<AST *> getNodes();

private:

    bool isValidProperty(PropertyNode *property, BlockNode *type);
    QList<BlockNode *> getParentTypes(BlockNode *type);

    void readLibrary(QString rootDir, QMap<QString, QString> importList);
    QList<AST *> m_libraryTrees;
    int m_majorVersion;
    int m_minorVersion;
};

#endif // STRIDELIBRARY_HPP
