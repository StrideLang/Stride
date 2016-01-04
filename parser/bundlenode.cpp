#include <cassert>

#include "bundlenode.h"

BundleNode::BundleNode(string name, ListNode *indexList, int line) :
    AST(AST::Bundle, line)
{
    addChild(indexList);
    m_name = name;
    m_namespace = "";
}

BundleNode::BundleNode(string name, string namespace_, ListNode *indexList, int line):
    AST(AST::Bundle, line)
{
    addChild(indexList);
    m_name = name;
    m_namespace = namespace_;

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

string BundleNode::getNamespace() const
{
    return m_namespace;
}

ListNode *BundleNode::index() const
{
    return static_cast<ListNode *>(m_children.at(0));
}

AST *BundleNode::deepCopy()
{
    assert(getNodeType() == AST::Bundle);
    if(getNodeType() == AST::Bundle) {
        AST *bundle = new BundleNode(m_name, static_cast<ListNode *>(index()->deepCopy()), m_line);
        bundle->setRate(m_rate);
        return bundle;
    }
    assert(0 == 1);
    return 0;
}
