#include "portpropertynode.h"

PortPropertyNode::PortPropertyNode(string name, string port, const char *filename, int line) :
    AST(AST::PortProperty, filename, line)
{
    m_name = name;
    m_port = port;
}

PortPropertyNode::~PortPropertyNode()
{

}

AST *PortPropertyNode::deepCopy()
{
    PortPropertyNode *newPortPropertyNode = new PortPropertyNode(m_name, m_port, m_filename.data(), m_line);
    return newPortPropertyNode;
}
