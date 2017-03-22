#include <cassert>

#include "declarationnode.h"
#include "valuenode.h"

DeclarationNode::DeclarationNode(string name, string objectType, AST *propertiesList,
                     const char *filename, int line):
    AST(AST::Declaration, filename, line)
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

DeclarationNode::DeclarationNode(BundleNode *bundle, string objectType, AST *propertiesList,
                     const char *filename, int line) :
    AST(AST::BundleDeclaration, filename, line)
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

DeclarationNode::~DeclarationNode()
{
}

string DeclarationNode::getName() const
{
    if(getNodeType() == AST::Declaration) {
        return m_name;
    } else if(getNodeType() == AST::BundleDeclaration) {
        return getBundle()->getName();
    }
    assert(0 == 1);
    return string();
}

BundleNode *DeclarationNode::getBundle() const
{
    assert(getNodeType() == AST::BundleDeclaration);
    return static_cast<BundleNode *>(m_children.at(0));
}

vector<PropertyNode *> DeclarationNode::getProperties() const
{
    return m_properties;
}

bool DeclarationNode::addProperty(PropertyNode *newProperty)
{
    for (PropertyNode *prop:m_properties) {
        if (prop->getName() == newProperty->getName()) {
            return false; // Property is not replaced
        }
    }
    addChild(newProperty);
    m_properties.push_back(newProperty);
	return true;
}

AST *DeclarationNode::getPropertyValue(string propertyName)
{
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == propertyName) {
            return m_properties.at(i)->getValue();
        }
    }
    return NULL;
}

void DeclarationNode::replacePropertyValue(string propertyName, AST *newValue)
{
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == propertyName) {
            m_properties.at(i)->replaceValue(newValue);
            break;
        }
    }
}

AST *DeclarationNode::getDomain()
{
    AST *domainValue = getPropertyValue("domain");
    return domainValue;
}

void DeclarationNode::setDomainString(string domain)
{
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == "domain") {
            m_properties.at(i)->replaceValue(new ValueNode(domain, "", -1));
        }
    }
}

string DeclarationNode::getObjectType() const
{
    return m_objectType;
}

AST *DeclarationNode::deepCopy()
{
    AST * newProps = new AST();
    AST *node = NULL;
    for(unsigned int i = 0; i< m_properties.size(); i++) {
        newProps->addChild(m_properties[i]->deepCopy());
    }
    if (getNodeType() == AST::BundleDeclaration) {
        node = new DeclarationNode(static_cast<BundleNode *>(getBundle()->deepCopy()),
                             m_objectType, newProps, m_filename.data(), m_line);
    } else if (getNodeType() == AST::Declaration) {
        node = new DeclarationNode(m_name, m_objectType, newProps, m_filename.data(), m_line);
    }
    assert(node);
    node->setRate(m_rate);
    delete newProps;
    return node;
}


