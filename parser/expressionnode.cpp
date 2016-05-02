#include <cassert>

#include "expressionnode.h"

ExpressionNode::ExpressionNode(ExpressionType type, AST *left, AST *right,
                               const char *filename, int line) :
    AST(AST::Expression, filename, line)
{
    m_type = type;
    assert(m_type != ExpressionNode::UnaryMinus && m_type != ExpressionNode::LogicalNot);
    addChild(left);
    addChild(right);
}

ExpressionNode::ExpressionNode(ExpressionNode::ExpressionType type, AST *value,
                               const char *filename, int line) :
    AST(AST::Expression, filename, line)
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
    assert(!this->isUnary());
    AST *right = getRight();
    getLeft()->deleteChildren();
    delete getLeft();
    m_children.clear();
    addChild(newLeft);
    addChild(right);
}

void ExpressionNode::replaceRight(AST *newRight)
{
    assert(!this->isUnary());
    AST *left = getLeft();
    getRight()->deleteChildren();
    delete getRight();
    m_children.clear();
    addChild(left);
    addChild(newRight);
}

void ExpressionNode::replaceValue(AST *newValue)
{
	assert(this->isUnary());
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

string ExpressionNode::getExpressionTypeString() const
{
    switch (m_type) {
    case Multiply:
        return "Multiply";
        break;
    case Divide:
        return "Divide";
        break;
    case Add:
        return "Add";
        break;
    case Subtract:
        return "Subtract";
        break;
    case And:
        return "And";
        break;
    case Or:
        return "Or";
        break;
    case UnaryMinus:
        return "UnaryMinus";
        break;
    case LogicalNot:
        return "LogicalNot";
        break;
    case Greater:
        return "Greater";
        break;
    case Lesser:
        return "Lesser";
        break;
    case Equal:
        return "Equal";
        break;
    case NotEqual:
        return "NotEqual";
        break;
    case GreaterEqual:
        return "GreaterEqual";
        break;
    case LesserEqual:
        return "LesserEqual";
        break;
    default:
        return "";
    }
}

AST *ExpressionNode::deepCopy()
{
    if (m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot) {
        return new ExpressionNode(m_type, m_children.at(0)->deepCopy(), m_filename.data(), m_line);
    } else {
        return new ExpressionNode(m_type, m_children.at(0)->deepCopy(), m_children.at(1)->deepCopy(), m_filename.data(), m_line);
    }
}


