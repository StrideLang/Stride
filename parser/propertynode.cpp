#include <cassert>

#include "propertynode.h"

PropertyNode::PropertyNode(string name, AST *value, const char *filename, int line):
    AST(AST::Property, filename, line)
{
    assert(value != NULL);
    m_name = name;
    addChild(value);
}

PropertyNode::~PropertyNode()
{

}

void PropertyNode::replaceValue(AST *newValue)
{
    deleteChildren();
    addChild(newValue);
}

AST *PropertyNode::deepCopy()
{
    return new PropertyNode(m_name, m_children.at(0)->deepCopy(), m_filename.data(), m_line);
}

