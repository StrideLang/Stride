#include "namenode.h"


NameNode::NameNode(string name, int line) :
    AST(AST::Name, line), m_namespace("")
{
    m_name = name;
}

NameNode::NameNode(string name, string namespace_, int line) :
    AST(AST::Name, line)
{
    m_name = name;
    m_namespace = namespace_;
}

NameNode::~NameNode()
{

}


AST *NameNode::deepCopy()
{
    NameNode *node = new NameNode(m_name, m_line);
    node->setRate(m_rate);
    return node;
}
