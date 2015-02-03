#include <cassert>

#include "valuenode.h"

ValueNode::ValueNode(int value) :
    AST(AST::Int)
{
    m_intValue = value;
}

ValueNode::ValueNode(float value) :
    AST(AST::Float)
{
    m_floatValue = value;
}

ValueNode::ValueNode(string value) :
    AST(AST::String)
{
    m_stringValue = value;
}

ValueNode::~ValueNode()
{

}

int ValueNode::getIntValue() const
{
    assert(m_token == AST::Int);
    return m_intValue;
}

float ValueNode::getFloatValue() const
{
    assert(m_token == AST::Float);
    return m_floatValue;
}

string ValueNode::getStringValue() const
{
    assert(m_token == AST::String);
    return m_stringValue;
}



