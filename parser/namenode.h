#ifndef NAMENODE_H
#define NAMENODE_H

#include <string>

#include "ast.h"

class NameNode : public AST
{
public:
    NameNode(string name, const char *filename, int line);

    NameNode(string name, string namespace_, const char *filename, int line);
    ~NameNode();

    string getName() const {return m_name;}

    AST *deepCopy();

private:
    string m_name;
};



#endif // NAMENODE_H
