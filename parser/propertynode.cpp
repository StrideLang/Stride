#include "propertynode.h"

PropertyNode::PropertyNode(string name, AST *value, int line):
    AST(AST::Property, line)
{
    m_name = name;
    addChild(value);
}

PropertyNode::~PropertyNode()
{

}\

