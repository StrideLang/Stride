#include "blocknode.h"

BlockNode::BlockNode(string name, string objectType, AST *propertiesList):
    AST(AST::Object)
{
    m_name = name;
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
    return m_name;
}
string BlockNode::getObjectType() const
{
    return m_objectType;
}


