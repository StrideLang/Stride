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

    string getNamespace() const {return m_namespace;}

    AST *deepCopy();

private:
    string m_name;
    string m_namespace;
};



#endif // NAMENODE_H
