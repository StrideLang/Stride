#ifndef TREEWALKER_H
#define TREEWALKER_H

#include <vector>

#include "ast.h"

class TreeWalker
{
public:
    TreeWalker(AST *tree);
    ~TreeWalker();

    vector<AST *> findPlatform();

private:
    AST *m_tree;
};

#endif // TREEWALKER_H
