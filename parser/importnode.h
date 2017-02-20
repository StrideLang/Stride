#ifndef IMPORTNODE_H
#define IMPORTNODE_H

#include <string>

#include "ast.h"
#include "scopenode.h"

class ImportNode : public AST
{
public:
    ImportNode(string name, AST *scope, const char *filename, int line, string alias = string());
    ImportNode(string name, const char *filename, int line, string alias = string());

    string importName() const;
    void setImportName(const string &importName);

    string importAlias() const;
    void setImportAlias(const string &importAlias);

    void resolveScope(AST *scope);

    AST *deepCopy();

private:
    string m_importName;
    string m_importAlias;
};

#endif // IMPORTNODE_H
