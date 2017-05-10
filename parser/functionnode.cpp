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
    m_rate = -1;
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
    m_rate = -1;
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

void FunctionNode::setDomainString(string domain)
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
    if (scope) {
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
    FunctionNode *newFunctionNode = new FunctionNode(m_name, newProps, m_type, m_filename.data(), m_line);
    newProps->deleteChildren();
    delete newProps;
    for (unsigned int i = 0; i < this->getScopeLevels(); i++) {
        newFunctionNode->addScope(this->getScopeAt(i));
    }
    newFunctionNode->setRate(getRate());
    return newFunctionNode;
}

double FunctionNode::getRate() const
{
    return m_rate;
}

void FunctionNode::setRate(double rate)
{
    m_rate = rate;
}

