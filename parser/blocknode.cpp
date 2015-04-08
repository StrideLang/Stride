#include <cassert>

#include "blocknode.h"


BlockNode::BlockNode(string name, string objectType, AST *propertiesList, int line):
    AST(AST::Block, line)
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

BlockNode::BlockNode(BundleNode *bundle, string objectType, AST *propertiesList, int line) :
    AST(AST::BlockBundle, line)
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
    if(getNodeType() == AST::Block) {
        return m_name;
    } else if(getNodeType() == AST::BlockBundle) {
        return static_cast<BundleNode *>(m_children.at(0))->getName();
    }
    assert(0 == 1);
    return string();
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

AST *BlockNode::deepCopy()
{
    AST * newProps = new AST();
    for(unsigned int i = 0; i< m_properties.size(); i++) {
        newProps->addChild(m_properties[i]->deepCopy());
    }
    AST *node = new BlockNode(m_name, m_objectType, newProps, m_line);
    node->setRate(m_rate);
    delete newProps;
    return node;
}


