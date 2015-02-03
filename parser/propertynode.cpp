#include "propertynode.h"

PropertyNode::PropertyNode(string name, AST *value):
    AST(AST::Property)
{
    m_name = name;
    addChild(value);
}

PropertyNode::~PropertyNode()
{

}\

