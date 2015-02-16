#ifndef NAMENODE_H
#define NAMENODE_H

#include <string>

#include "ast.h"

class NameNode : public AST
{
public:
    NameNode(string name, int line);
    ~NameNode();

    string getName() const {return m_name;}

private:
    string m_name;
};

#endif // NAMENODE_H
