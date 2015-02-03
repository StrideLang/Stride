#include "namenode.h"


NameNode::NameNode(string name) :
    AST(AST::Name)
{
    m_name = name;
}

NameNode::~NameNode()
{

}
