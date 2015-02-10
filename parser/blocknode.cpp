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
    addChild(bundle);
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
    return static_cast<BundleNode *>(m_children.at(0));
}

string BlockNode::getObjectType() const
{
    return m_objectType;
}


