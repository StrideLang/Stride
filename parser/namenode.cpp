#include "namenode.h"


NameNode::NameNode(string name, const char *filename, int line) :
    AST(AST::Name, filename, line), m_namespace("")
{
    m_name = name;
}

NameNode::NameNode(string name, string namespace_, const char *filename, int line) :
    AST(AST::Name, filename, line)
{
    m_name = name;
    m_namespace = namespace_;
}

NameNode::~NameNode()
{

}


AST *NameNode::deepCopy()
{
    NameNode *node = new NameNode(m_name, m_filename.data(), m_line);
    node->setRate(m_rate);
    return node;
}
