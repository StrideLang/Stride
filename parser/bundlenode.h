#ifndef BUNDLENODE_H
#define BUNDLENODE_H

#include <string>

#include "ast.h"
#include "listnode.h"

class BundleNode : public AST
{
public:
    BundleNode(string name, ListNode *indexList, const char *filename, int line);
    BundleNode(string name, AST *scope, ListNode *indexList, const char *filename, int line);
    BundleNode(string name, string namespace_, ListNode *indexList, const char *filename, int line);
    virtual ~BundleNode();

    string getName() const;
    ListNode *index() const;

    void resolveScope(AST *scope);

    AST *deepCopy();

private:
    string m_name;
};

#endif // BUNDLENODE_H
