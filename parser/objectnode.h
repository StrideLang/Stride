#ifndef OBJECTNODE_H
#define OBJECTNODE_H

#include <string>

#include "ast.h"

class ObjectNode : public AST
{
public:
    ObjectNode(string name, string objectType, AST *propertiesList);
    ~ObjectNode();

    string getName() const;

    string getObjectType() const;

private:
    string m_name;
    string m_objectType;
};

#endif // OBJECTNODE_H
