#ifndef STRIDELIBRARY_HPP
#define STRIDELIBRARY_HPP

#include <vector>

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

    std::vector<AST *> getNodes();

private:

    bool isValidProperty(PropertyNode *property, BlockNode *type);
    QList<BlockNode *> getParentTypes(BlockNode *type);

    void readLibrary(QString rootDir);
    void readLibraryTypes(QString rootDir);
    QList<AST *> m_libraryTrees;
    QList<AST *> m_libraryTypes;
};

#endif // STRIDELIBRARY_HPP
