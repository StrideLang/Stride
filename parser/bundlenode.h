#ifndef BUNDLENODE_H
#define BUNDLENODE_H

#include <string>

#include "ast.h"

class BundleNode : public AST
{
public:
    BundleNode(string getName, AST *indexExp);
    ~BundleNode();

    string getName() const;
    AST *index() const { return m_children.at(0); }

private:
    string m_name;
};

#endif // BUNDLENODE_H
