#ifndef BUNDLENODE_H
#define BUNDLENODE_H

#include <string>

#include "ast.h"

class BundleNode : public AST
{
public:
    BundleNode(string name, AST *indexList, int line);
    virtual ~BundleNode();

    string getName() const;
    AST *index() const;

    AST *deepCopy();

private:
    string m_name;
};

#endif // BUNDLENODE_H
