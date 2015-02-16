#include "bundlenode.h"

BundleNode::BundleNode(string name, AST *indexExp, int line) :
    AST(AST::Bundle, line)
{
    addChild(indexExp);
    m_name = name;
}

BundleNode::~BundleNode()
{

}

string BundleNode::getName() const
{
    return m_name;
}
