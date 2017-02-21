#ifndef BLOCKNODE_H
#define BLOCKNODE_H

#include <string>

#include "ast.h"

class BlockNode : public AST
{
public:
    BlockNode(string name, const char *filename, int line);
    BlockNode(string name, AST *scope, const char *filename, int line);

    ~BlockNode();

    string getName() const {return m_name;}

    void resolveScope(AST *scope);

    AST *deepCopy();

private:
    string m_name;
};

#endif // BLOCKNODE_H
