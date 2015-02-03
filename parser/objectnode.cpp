#include "objectnode.h"

ObjectNode::ObjectNode(string name, string objectType, AST *propertiesList):
    AST(AST::Object)
{
    m_name = name;
    m_objectType = objectType;
    if (propertiesList) {
        propertiesList->pushParent(this);
    }
}

ObjectNode::~ObjectNode()
{

}
string ObjectNode::getName() const
{
    return m_name;
}
string ObjectNode::getObjectType() const
{
    return m_objectType;
}


