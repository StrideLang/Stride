#include <cassert>

#include "functionnode.h"

FunctionNode::FunctionNode(string name, AST *propertiesList, FunctionType type, int line, string namespace_) :
    AST(AST::Function, line)
{
    m_name = name;
    if (propertiesList) {
        propertiesList->giveChildren(this);
    }
    m_type = type;

    for (unsigned int i = 0; i < m_children.size(); i++) {
        assert(m_children.at(i)->getNodeType() == AST::Property);
        m_properties.push_back(static_cast<PropertyNode *>(m_children.at(i)));
    }
    m_parallelInstances = 0;

    m_namespace = namespace_;
}

FunctionNode::~FunctionNode()
{

}

vector<PropertyNode *> FunctionNode::getProperties() const
{
    return m_properties;
}

AST *FunctionNode::deepCopy()
{
    AST * newProps = new AST();
    for(unsigned int i = 0; i< m_properties.size(); i++) {
        newProps->addChild(m_properties[i]->deepCopy());
    }
    AST *output = new FunctionNode(m_name, newProps, m_type, m_line);
    newProps->deleteChildren();
    delete newProps;
    output->setRate(m_rate);
    return output;
}

