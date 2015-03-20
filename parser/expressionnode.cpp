#include <cassert>

#include "expressionnode.h"

ExpressionNode::ExpressionNode(ExpressionType type, AST *left, AST *right, int line) :
    AST(AST::Expression, line)
{
    m_type = type;
    assert(m_type != ExpressionNode::UnaryMinus && m_type != ExpressionNode::LogicalNot);
    addChild(left);
    addChild(right);
}

ExpressionNode::ExpressionNode(ExpressionNode::ExpressionType type, AST *value, int line) :
    AST(AST::Expression, line)
{
    m_type = type;
    assert(m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
    addChild(value);
}

ExpressionNode::~ExpressionNode()
{

}

AST *ExpressionNode::getLeft() const {return m_children.at(0); }

AST *ExpressionNode::getRight() const { return m_children.at(1); }

AST *ExpressionNode::getValue() const
{
    assert(m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
    return m_children.at(0);
}

ExpressionNode::ExpressionType ExpressionNode::getExpressionType() const { return m_type; }

AST *ExpressionNode::deepCopy()
{
    if (m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot) {
        return new ExpressionNode(m_type, m_children.at(0)->deepCopy(), m_line);
    } else {
        return new ExpressionNode(m_type, m_children.at(0)->deepCopy(), m_children.at(1)->deepCopy(), m_line);
    }
}


