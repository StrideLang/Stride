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
        Or,
        UnaryMinus,
        LogicalNot
    } ExpressionType;

    ExpressionNode(ExpressionType type, AST *left, AST *right, int line);
    ExpressionNode(ExpressionType type, AST *value, int line);
    ~ExpressionNode();

    AST *getLeft() const;
    AST *getRight() const;

    AST *getValue() const;

    ExpressionType getExpressionType() const;
    AST *deepCopy();

private:
    ExpressionType m_type;
};

#endif // EXPRESSIONNODE_H
