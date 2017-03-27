#ifndef STRIDELIBRARY_HPP
#define STRIDELIBRARY_HPP

#include <vector>

#include <QString>
#include <QList>
#include <QMap>

#include "declarationnode.h"
#include "langerror.h"

class StrideLibrary
{
public:
    StrideLibrary();
    StrideLibrary(QString libraryPath, QMap<QString,QString> importList = QMap<QString,QString>());
   ~StrideLibrary();

    void setLibraryPath(QString libraryPath, QMap<QString,QString> importList = QMap<QString,QString>());

    DeclarationNode *findTypeInLibrary(QString typeName);

    bool isValidBlock(DeclarationNode *block);

    std::vector<AST *> getLibraryMembers();

private:

    bool isValidProperty(PropertyNode *property, DeclarationNode *type);
    QList<DeclarationNode *> getParentTypes(DeclarationNode *type);

    void readLibrary(QString rootDir, QMap<QString, QString> importList);
    QList<AST *> m_libraryTrees;
    int m_majorVersion;
    int m_minorVersion;
};

#endif // STRIDELIBRARY_HPP
