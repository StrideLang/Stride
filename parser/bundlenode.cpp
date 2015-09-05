#include <cassert>

#include "bundlenode.h"

BundleNode::BundleNode(string name, AST *indexList, int line) :
    AST(AST::Bundle, line)
{
    addChild(indexList);
    m_name = name;
}

//BundleNode::BundleNode(string name, AST *indexStartExp, AST *indexEndExp, int line):
//    AST(AST::BundleRange, line)
//{
//    addChild(indexStartExp);
//    addChild(indexEndExp);
//    m_name = name;
//}

BundleNode::~BundleNode()
{

}

string BundleNode::getName() const
{
    return m_name;
}

AST *BundleNode::index() const
{
    assert(getNodeType() == AST::Bundle);
    return m_children.at(0);
}

AST *BundleNode::deepCopy()
{
    assert(getNodeType() == AST::Bundle);
    if(getNodeType() == AST::Bundle) {
        AST *bundle = new BundleNode(m_name, m_children.at(0)->deepCopy(), m_line);
        bundle->setRate(m_rate);
        return bundle;
    }
    assert(0 == 1);
    return 0;
}
