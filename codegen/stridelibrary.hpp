#ifndef STRIDELIBRARY_HPP
#define STRIDELIBRARY_HPP

#include <QString>
#include <QList>

class StrideLibrary
{
public:
    StrideLibrary(QString libraryPath);

private:
    void readLibrary(QString rootDir);
    QList<AST *> m_platformTrees;
};

#endif // STRIDELIBRARY_HPP
