#ifndef OBJECTNODE_H
#define OBJECTNODE_H

#include <string>

#include "ast.h"
#include "bundlenode.h"

class BlockNode : public AST
{
public:
    BlockNode(string name, string objectType, AST *propertiesList);
    BlockNode(BundleNode *bundle, string objectType, AST *propertiesList);
    ~BlockNode();

    string getName() const;
    BundleNode *getBundle() const;

    string getObjectType() const;

private:
    string m_name;
    string m_objectType;
    BundleNode * m_bundle;
};

#endif // OBJECTNODE_H
