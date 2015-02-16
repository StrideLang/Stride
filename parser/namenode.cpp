#include "namenode.h"


NameNode::NameNode(string name, int line) :
    AST(AST::Name, line)
{
    m_name = name;
}

NameNode::~NameNode()
{

}
