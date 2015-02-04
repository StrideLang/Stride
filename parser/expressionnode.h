#ifndef EXPRESSIONNODE_H
#define EXPRESSIONNODE_H

#include "ast.h"

class ExpressionNode : public AST
{
public:
    typedef enum {
        Multiply,
        Divide,
        Add,
        Subtract,
        And,
        Or
    } ExpressionType;

    ExpressionNode(ExpressionType type, AST *left, AST *right);
    ~ExpressionNode();

    AST *getLeft() const {return m_children.at(0); }
    AST *getRight() const { return m_children.at(1); }

    ExpressionType getExpressionType() const { return m_type; }

private:
    ExpressionType m_type;
};

#endif // EXPRESSIONNODE_H
