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

    ExpressionNode(ExpressionType type, AST *left, AST *right, const char *filename, int line);
    ExpressionNode(ExpressionType type, AST *value, const char *filename, int line);
    ~ExpressionNode();

    bool isUnary() const;

    AST *getLeft() const;
    AST *getRight() const;

    void replaceLeft(AST *newLeft);
    void replaceRight(AST *newRight);
    void replaceValue(AST *newValue);

    AST *getValue() const;

    ExpressionType getExpressionType() const;
    string getExpressionTypeString() const;
    AST *deepCopy();

private:
    ExpressionType m_type;
};

#endif // EXPRESSIONNODE_H
