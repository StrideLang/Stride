#include <cassert>

#include "propertynode.h"

PropertyNode::PropertyNode(string name, AST *value, int line):
    AST(AST::Property, line)
{
    assert(value != NULL);
    m_name = name;
    addChild(value);
}

PropertyNode::~PropertyNode()
{

}

AST *PropertyNode::deepCopy()
{
    return new PropertyNode(m_name, m_children.at(0)->deepCopy(), m_line);
}

