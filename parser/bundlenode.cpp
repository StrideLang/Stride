#include "bundlenode.h"

BundleNode::BundleNode(string name, AST *indexExp) :
    AST(AST::Bundle)
{
    addChild(indexExp);
    m_name = name;
}

BundleNode::~BundleNode()
{

}
string BundleNode::name() const
{
    return m_name;
}