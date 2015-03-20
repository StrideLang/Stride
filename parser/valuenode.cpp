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
    AST(AST::Real, line)
{
    m_floatValue = value;
}

ValueNode::ValueNode(double value, int line) :
    AST(AST::Real, line)
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
    assert(getNodeType() == AST::Int);
    return m_intValue;
}

double ValueNode::getRealValue() const
{
    assert(getNodeType() == AST::Real);
    return m_floatValue;
}

string ValueNode::getStringValue() const
{
    assert(getNodeType() == AST::String);
    return m_stringValue;
}

bool ValueNode::getSwitchValue() const
{
    assert(getNodeType() == AST::Switch);
    return m_switch;
}

AST *ValueNode::deepCopy()
{
    if (getNodeType() == AST::Int) {
        return new ValueNode(getIntValue(), getLine());
    } else if (getNodeType() == AST::Real) {
        return new ValueNode(getRealValue(), getLine());
    } else if (getNodeType() == AST::String) {
        return new ValueNode(getStringValue(), getLine());
    } else if (getNodeType() == AST::Switch) {
        return new ValueNode(getSwitchValue(), getLine());
    } else {
        assert(0); // Invalid type
    }
}



