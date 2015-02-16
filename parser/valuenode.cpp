#include <cassert>

#include "valuenode.h"

ValueNode::ValueNode(int line) :
    AST(AST::None, line)
{

}

ValueNode::ValueNode(int value, int line) :
    AST(AST::Int, line)
{
    m_intValue = value;
}

ValueNode::ValueNode(float value, int line) :
    AST(AST::Float, line)
{
    m_floatValue = value;
}

ValueNode::ValueNode(string value, int line) :
    AST(AST::String, line)
{
    m_stringValue = value;
}

ValueNode::ValueNode(bool value, int line) :
    AST(AST::Switch, line)
{
    m_switch = value;
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

bool ValueNode::getSwitchValue() const
{
    assert(m_token == AST::Switch);
    return m_switch;
}



