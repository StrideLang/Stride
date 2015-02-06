#include <cassert>

#include "blocknode.h"

BlockNode::BlockNode(string name, string objectType, AST *propertiesList):
    AST(AST::Block)
{
    m_name = name;
    m_objectType = objectType;
    if (propertiesList) {
        propertiesList->giveChildren(this);
    }
}

BlockNode::BlockNode(BundleNode *bundle, string objectType, AST *propertiesList) :
    AST(AST::BlockBundle)
{
    m_bundle = bundle;
    m_objectType = objectType;
    if (propertiesList) {
        propertiesList->giveChildren(this);
    }
}

BlockNode::~BlockNode()
{

}
string BlockNode::getName() const
{
    assert(getNodeType() == AST::Block);
    return m_name;
}

BundleNode *BlockNode::getBundle() const
{
    assert(getNodeType() == AST::BlockBundle);
    return m_bundle;
}
string BlockNode::getObjectType() const
{
    return m_objectType;
}


