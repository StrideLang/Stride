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
    StrideLibrary();
    StrideLibrary(QString libraryPath);
   ~StrideLibrary();

    void setLibraryPath(QString libraryPath);

    BlockNode *findTypeInLibrary(QString typeName);

    bool isValidBlock(BlockNode *block);

    std::vector<AST *> getNodes();

private:

    bool isValidProperty(PropertyNode *property, BlockNode *type);
    QList<BlockNode *> getParentTypes(BlockNode *type);

    void readLibrary(QString rootDir);
    QList<AST *> m_libraryTrees;
};

#endif // STRIDELIBRARY_HPP
