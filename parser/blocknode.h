#ifndef BLOCKNODE_H
#define BLOCKNODE_H

#include <string>

#include "ast.h"

class BlockNode : public AST
{
public:
    BlockNode(string name, const char *filename, int line);

    BlockNode(string name, string namespace_, const char *filename, int line);
    ~BlockNode();

    string getName() const {return m_name;}

    AST *deepCopy();

private:
    string m_name;
};



#endif // BLOCKNODE_H
