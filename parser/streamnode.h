#ifndef STREAMNODE_H
#define STREAMNODE_H

#include "ast.h"

class StreamNode : public AST
{
public:
    StreamNode(AST *left, AST *right);
    ~StreamNode();

    AST *getLeft() const {return m_children.at(0); }
    AST *getRight() const { return m_children.at(1); }
private:
};

#endif // STREAMNODE_H
