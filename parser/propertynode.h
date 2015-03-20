#ifndef PROPERTYNODE_H
#define PROPERTYNODE_H

#include <string>

#include "ast.h"

class PropertyNode : public AST
{
public:
    PropertyNode(string name, AST *value, int line);
    ~PropertyNode();

    string getName() const { return m_name; }
    AST *getValue() const { return m_children[0]; }

    AST *deepCopy();

private:
    string m_name;
};

#endif // PROPERTYNODE_H
