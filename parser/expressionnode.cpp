#include "expressionnode.h"

ExpressionNode::ExpressionNode(ExpressionType type, AST *left, AST *right) :
    AST(AST::Expression)
{
    m_type = type;
    addChild(left);
    addChild(right);
}

ExpressionNode::~ExpressionNode()
{

}


