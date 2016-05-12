#include <cassert>

#include "functionnode.h"

FunctionNode::FunctionNode(string name, AST *propertiesList, FunctionType type,
                           const char *filename, int line, string namespace_) :
    AST(AST::Function, filename, line)
{
    m_name = name;
    if (propertiesList) {
        propertiesList->giveChildren(this);
    }
    m_type = type;

    m_namespace = namespace_;
}

FunctionNode::~FunctionNode()
{

}

void FunctionNode::addChild(AST *t)
{
    AST::addChild(t);
    assert(t->getNodeType() == AST::Property);
    m_properties.push_back(static_cast<PropertyNode *>(t));
}

void FunctionNode::setChildren(vector<AST *> &newChildren)
{
    AST::setChildren(newChildren);
    for (unsigned int i = 0; i < m_children.size(); i++) {
        assert(m_children.at(i)->getNodeType() == AST::Property);
        m_properties.push_back(static_cast<PropertyNode *>(m_children.at(i)));
    }
}

void FunctionNode::deleteChildren()
{
    AST::deleteChildren();
    m_properties.clear();
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
    AST *output = new FunctionNode(m_name, newProps, m_type, m_filename.data(), m_line);
    newProps->deleteChildren();
    delete newProps;
    output->setRate(m_rate);
    return output;
}

