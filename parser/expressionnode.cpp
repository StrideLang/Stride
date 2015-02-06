#include <cassert>

#include "expressionnode.h"

ExpressionNode::ExpressionNode(ExpressionType type, AST *left, AST *right) :
    AST(AST::Expression)
{
    m_type = type;
    assert(m_type != ExpressionNode::UnaryMinus && m_type != ExpressionNode::LogicalNot);
    addChild(left);
    addChild(right);
}

ExpressionNode::ExpressionNode(ExpressionNode::ExpressionType type, AST *value) :
    AST(AST::Expression)
{
    m_type = type;
    assert(m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
    addChild(value);
}

ExpressionNode::~ExpressionNode()
{

}

AST *ExpressionNode::getValue() const
{
    assert(m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
    return m_children.at(0);
}


