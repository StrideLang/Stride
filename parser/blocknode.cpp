#include "blocknode.h"

BlockNode::BlockNode(string name, const char *filename, int line) :
    AST(AST::Name, filename, line)
{
    m_name = name;
}

BlockNode::BlockNode(string name, string namespace_, const char *filename, int line) :
    AST(AST::Name, filename, line)
{
    m_name = name;
    setNamespace(namespace_);
}

BlockNode::~BlockNode()
{

}

AST *BlockNode::deepCopy()
{
    BlockNode *node = new BlockNode(m_name, m_filename.data(), m_line);
    node->setRate(m_rate);
    return node;
}
