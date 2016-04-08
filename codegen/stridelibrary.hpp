#ifndef STRIDELIBRARY_HPP
#define STRIDELIBRARY_HPP

#include <QString>
#include <QList>

#include "blocknode.h"
#include "langerror.h"

class StrideLibrary
{
public:
    StrideLibrary(QString libraryPath);
   ~StrideLibrary();

    BlockNode *findTypeInLibrary(QString typeName);

    bool isValidBlock(BlockNode *block);

private:

    bool isValidProperty(PropertyNode *property, BlockNode *type);
    QList<BlockNode *> getParentTypes(BlockNode *type);

    void readLibrary(QString rootDir);
    void readLibraryTypes(QString rootDir);
    QList<AST *> m_platformTrees;
    QList<AST *> m_platformTypes;
};

#endif // STRIDELIBRARY_HPP
