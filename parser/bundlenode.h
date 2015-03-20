#ifndef BUNDLENODE_H
#define BUNDLENODE_H

#include <string>

#include "ast.h"

class BundleNode : public AST
{
public:
    BundleNode(string name, AST *indexExp, int line);
    BundleNode(string name, AST *indexStartExp, AST *indexEndExp, int line);
    virtual ~BundleNode();

    string getName() const;
    AST *index() const;
    AST *startIndex() const;
    AST *endIndex() const;

    int getBundleSize();
    AST *deepCopy();

private:
    string m_name;
};

#endif // BUNDLENODE_H
