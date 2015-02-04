#ifndef OBJECTNODE_H
#define OBJECTNODE_H

#include <string>

#include "ast.h"

class BlockNode : public AST
{
public:
    BlockNode(string name, string objectType, AST *propertiesList);
    ~BlockNode();

    string getName() const;

    string getObjectType() const;

private:
    string m_name;
    string m_objectType;
};

#endif // OBJECTNODE_H
