/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <cassert>

#include "declarationnode.h"
#include "valuenode.h"
#include "blocknode.h"

using namespace std;

DeclarationNode::DeclarationNode(string name, string objectType, ASTNode propertiesList,
                     const char *filename, int line, vector<string> scope):
    AST(AST::Declaration, filename, line, scope)
{
    m_name = name;
    m_objectType = objectType;
    if (propertiesList) {
        for (ASTNode child: propertiesList->getChildren()) {
            addChild(child);
        }
    }
    for (unsigned int i = 0; i < m_children.size(); i++) {
        assert(m_children.at(i)->getNodeType() == AST::Property);
        m_properties.push_back(static_pointer_cast<PropertyNode>(m_children.at(i)));
    }

    m_CompilerProperties = make_shared<ListNode>(__FILE__, __LINE__);
}

DeclarationNode::DeclarationNode(std::shared_ptr<BundleNode> bundle, string objectType, ASTNode propertiesList,
                     const char *filename, int line, vector<string> scope) :
    AST(AST::BundleDeclaration, filename, line, scope)
{
    addChild(bundle);
    m_objectType = objectType;
    if (propertiesList) {
        for (ASTNode child: propertiesList->getChildren()) {
            addChild(child);
        }
    }
    for (unsigned int i = 1; i < m_children.size(); i++) {
        assert(m_children.at(i)->getNodeType() == AST::Property);
        m_properties.push_back(static_pointer_cast<PropertyNode>(m_children.at(i)));
    }

    m_CompilerProperties = make_shared<ListNode>(__FILE__, __LINE__);
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

std::shared_ptr<BundleNode> DeclarationNode::getBundle() const
{
    assert(getNodeType() == AST::BundleDeclaration);
    return static_pointer_cast<BundleNode>(m_children.at(0));
}

vector<std::shared_ptr<PropertyNode> > DeclarationNode::getProperties() const
{
    return m_properties;
}

bool DeclarationNode::addProperty(std::shared_ptr<PropertyNode> newProperty)
{
    for (auto prop:m_properties) {
        if (prop->getName() == newProperty->getName()) {
            return false; // Property is not replaced
        }
    }
    addChild(newProperty);
    m_properties.push_back(newProperty);
    return true;
}

ASTNode DeclarationNode::getPropertyValue(string propertyName)
{
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == propertyName) {
            return m_properties.at(i)->getValue();
        }
    }
    return nullptr;
}

void DeclarationNode::setPropertyValue(string propertyName, ASTNode value)
{
    if (getPropertyValue(propertyName)) {
        replacePropertyValue(propertyName, value);
    } else {
        addProperty(std::make_shared<PropertyNode>(propertyName, value, value->getFilename().c_str(), value->getLine()));
    }
}

bool DeclarationNode::replacePropertyValue(string propertyName, ASTNode newValue)
{
    bool replaced = false;
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == propertyName) {
            m_properties.at(i)->replaceValue(newValue);
            replaced = true;
            break;
        }
    }
    return replaced;
}

ASTNode DeclarationNode::getDomain()
{
    ASTNode domainValue = getPropertyValue("domain");
    return domainValue;
}

void DeclarationNode::setDomainString(string domain)
{
    replacePropertyValue("domain", make_shared<BlockNode>(domain, __FILE__, __LINE__));
}

string DeclarationNode::getObjectType() const
{
    return m_objectType;
}

ASTNode DeclarationNode::deepCopy()
{
    ASTNode newProps = std::make_shared<AST>();
    std::shared_ptr<DeclarationNode> node;
    for(unsigned int i = 0; i< m_properties.size(); i++) {
        newProps->addChild(m_properties[i]->deepCopy());
    }
    if (getNodeType() == AST::BundleDeclaration) {
        node = std::make_shared<DeclarationNode>(static_pointer_cast<BundleNode>(getBundle()->deepCopy()),
                             m_objectType, newProps, m_filename.data(), m_line, m_scope);
    } else if (getNodeType() == AST::Declaration) {
        node = std::make_shared<DeclarationNode>(m_name, m_objectType, newProps, m_filename.data(), m_line, m_scope);
    }
    node->m_CompilerProperties = this->m_CompilerProperties;
    assert(node);
//    newProps.reset();
    return node;
}


