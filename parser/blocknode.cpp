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
    for (unsigned int i = 0; i < m_children.size(); i++) {
        assert(m_children.at(i)->getNodeType() == AST::Property);
        m_properties.push_back(static_cast<PropertyNode *>(m_children.at(i)));
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
    for (unsigned int i = 1; i < m_children.size(); i++) {
        assert(m_children.at(i)->getNodeType() == AST::Property);
        m_properties.push_back(static_cast<PropertyNode *>(m_children.at(i)));
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

vector<PropertyNode *> BlockNode::getProperties() const
{
    return m_properties;
}

string BlockNode::getObjectType() const
{
    return m_objectType;
}


