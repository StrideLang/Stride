#include <cassert>

#include "functionnode.h"
#include "scopenode.h"
#include "valuenode.h"

FunctionNode::FunctionNode(string name, AST *propertiesList, FunctionType type,
                           const char *filename, int line) :
    AST(AST::Function, filename, line)
{
    m_name = name;
    if (propertiesList) {
        propertiesList->giveChildren(this);
    }
    m_type = type;
}

FunctionNode::FunctionNode(string name, AST* scope, AST *propertiesList, FunctionType type,
                           const char *filename, int line) :
    AST(AST::Function, filename, line)
{
    m_name = name;
    if (propertiesList) {
        propertiesList->giveChildren(this);
    }
    m_type = type;
    resolveScope(scope);
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

AST *FunctionNode::getDomain()
{
    AST *domainValue = getPropertyValue("domain");
    return domainValue;
}

void FunctionNode::setDomain(string domain)
{
    bool domainSet = false;
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == "domain") {
            m_properties.at(i)->replaceValue(new ValueNode(domain, "", -1));
            domainSet = true;
        }
    }
    if (!domainSet) {
        addProperty(new PropertyNode("domain", new ValueNode(domain, "", -1), "", -1));
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

void FunctionNode::addProperty(PropertyNode *newProperty)
{
    if (!getPropertyValue(newProperty->getName())) {
        addChild(newProperty);
//        m_properties.push_back(newProperty);
    }
}

AST *FunctionNode::getPropertyValue(string propertyName)
{
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == propertyName) {
            return m_properties.at(i)->getValue();
        }
    }
    return NULL;
}

void FunctionNode::resolveScope(AST *scope)
{
    if (scope)
    {
        for (unsigned int i = 0; i < scope->getChildren().size(); i++) {
            assert(scope->getChildren().at(i)->getNodeType() == AST::Scope);
            m_scope.push_back((static_cast<ScopeNode *>(scope->getChildren().at(i)))->getName());
        }
    }
}

AST *FunctionNode::deepCopy()
{
    AST * newProps = new AST();
    for(unsigned int i = 0; i< m_properties.size(); i++) {
        newProps->addChild(m_properties[i]->deepCopy());
    }
    AST *newFunctionNode = new FunctionNode(m_name, newProps, m_type, m_filename.data(), m_line);
    newProps->deleteChildren();
    delete newProps;
    newFunctionNode->setRate(m_rate);
    for (unsigned int i = 0; i < this->getScopeLevels(); i++) {
        newFunctionNode->addScope(this->getScopeAt(i));
    }
    return newFunctionNode;
}

