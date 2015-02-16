#ifndef OBJECTNODE_H
#define OBJECTNODE_H

#include <string>

#include "ast.h"
#include "bundlenode.h"
#include "propertynode.h"

class BlockNode : public AST
{
public:
    BlockNode(string name, string objectType, AST *propertiesList, int line);
    BlockNode(BundleNode *bundle, string objectType, AST *propertiesList, int line);
    ~BlockNode();

    string getName() const;
    BundleNode *getBundle() const;
    vector<PropertyNode *> getProperties() const;

    string getObjectType() const;

private:
    string m_name;
    string m_objectType;
    vector<PropertyNode *> m_properties;
};

#endif // OBJECTNODE_H
