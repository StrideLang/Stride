#include "namenode.h"


NameNode::NameNode(string name, int line) :
    AST(AST::Name, line)
{
    m_name = name;
}

NameNode::~NameNode()
{

}


AST *NameNode::deepCopy()
{
    return new NameNode(m_name, m_line);
}
