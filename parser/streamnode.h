#ifndef STREAMNODE_H
#define STREAMNODE_H

#include "ast.h"

class StreamNode : public AST
{
public:
    StreamNode(AST *left, AST *right, int line);
    ~StreamNode();

    AST *getLeft() const {return m_children.at(0); }
    AST *getRight() const { return m_children.at(1); }

    void setLeft(AST *newLeft);
    void setRight(AST *newRight);

    AST *deepCopy();
private:
};

#endif // STREAMNODE_H
