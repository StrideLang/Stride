#include <cassert>
#include <sstream>

#include "valuenode.h"

ValueNode::ValueNode(const char * filename, int line) :
    AST(AST::None, filename, line)
{
}

ValueNode::ValueNode(int value, const char * filename, int line) :
    AST(AST::Int, filename, line)
{
    m_intValue = value;
}

ValueNode::ValueNode(float value, const char * filename, int line) :
    AST(AST::Real, filename, line)
{
    m_floatValue = value;
}

ValueNode::ValueNode(double value, const char * filename, int line) :
    AST(AST::Real, filename, line)
{
    m_floatValue = value;
}

ValueNode::ValueNode(string value, const char * filename, int line) :
    AST(AST::String, filename, line)
{
    m_stringValue = value;
}

ValueNode::ValueNode(bool value, const char * filename, int line) :
    AST(AST::Switch, filename, line)
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

double ValueNode::toReal() const
{
    if (getNodeType() == AST::Real) {
        return m_floatValue;
    } else if (getNodeType() == AST::Int) {
        return m_intValue;
    }
    return 0;
}

string ValueNode::getStringValue() const
{
    assert(getNodeType() == AST::String);
    return m_stringValue;
}

string ValueNode::toString() const
{
    if (getNodeType() == AST::Real) {
        stringstream ss(stringstream::in);
        ss << m_floatValue;
        return ss.str();
    } else if (getNodeType() == AST::Int) {
        stringstream ss(stringstream::in);
        ss << m_intValue;
        return ss.str();
    } else if (getNodeType() == AST::String) {
        return m_stringValue;
    } else if (getNodeType() == AST::Switch) {
        if (m_switch) {
            return "On";
        } else {
            return "Off";
        }
    }
    return "";
}

bool ValueNode::getSwitchValue() const
{
    assert(getNodeType() == AST::Switch);
    return m_switch;
}

AST *ValueNode::deepCopy()
{
    if (getNodeType() == AST::Int) {
        return new ValueNode(getIntValue(), m_filename.data(), getLine());
    } else if (getNodeType() == AST::Real) {
        return new ValueNode(getRealValue(), m_filename.data(), getLine());
    } else if (getNodeType() == AST::String) {
        return new ValueNode(getStringValue(), m_filename.data(), getLine());
    } else if (getNodeType() == AST::Switch) {
        return new ValueNode(getSwitchValue(), m_filename.data(), getLine());
    } else if (getNodeType() == AST::None) {
        return new ValueNode(m_filename.data(), getLine());
    }  else {
        assert(0); // Invalid type
    }
}



