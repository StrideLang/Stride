#ifndef OBJECTNODE_H
#define OBJECTNODE_H

#include <string>

#include "ast.h"

class ObjectNode : public AST
{
public:
    ObjectNode(string name, string objectType, AST *propertiesList);
    ~ObjectNode();

private:
    string m_name;
    string m_objectType;
};

#endif // OBJECTNODE_H
