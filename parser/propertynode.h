#ifndef PROPERTYNODE_H
#define PROPERTYNODE_H

#include <string>

#include "ast.h"

class PropertyNode : public AST
{
public:
    PropertyNode(string name, AST *value);
    ~PropertyNode();

    string getName() const { return m_name; }

private:
    string m_name;
};

#endif // PROPERTYNODE_H
