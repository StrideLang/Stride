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

bool ExpressionNode::isUnary() const
{
    return (m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
}

AST *ExpressionNode::getLeft() const
{
    assert(!this->isUnary());
    return m_children.at(0);
}

AST *ExpressionNode::getRight() const
{
    assert(!this->isUnary());
    return m_children.at(1);
}

void ExpressionNode::replaceLeft(AST *newLeft)
{
    AST *right = getRight();
    getLeft()->deleteChildren();
    delete getLeft();
    m_children.clear();
    addChild(newLeft);
    addChild(right);
}

void ExpressionNode::replaceRight(AST *newRight)
{
    AST *left = getLeft();
    getRight()->deleteChildren();
    delete getRight();
    m_children.clear();
    addChild(left);
    addChild(newRight);
}

void ExpressionNode::replaceValue(AST *newValue)
{
    deleteChildren();
    m_children.push_back(newValue);
}

AST *ExpressionNode::getValue() const
{
    assert(isUnary());
    return m_children.at(0);
}

ExpressionNode::ExpressionType ExpressionNode::getExpressionType() const
{
    return m_type;
}

AST *ExpressionNode::deepCopy()
{
    if (m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot) {
        return new ExpressionNode(m_type, m_children.at(0)->deepCopy(), m_line);
    } else {
        return new ExpressionNode(m_type, m_children.at(0)->deepCopy(), m_children.at(1)->deepCopy(), m_line);
    }
}


