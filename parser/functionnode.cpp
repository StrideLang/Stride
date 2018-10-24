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
#include "listnode.h"

using namespace std;

FunctionNode::FunctionNode(string name, ASTNode propertiesList,
                           const char *filename, int line) :
    AST(AST::Function, filename, line)
{
    m_name = name;
    if (propertiesList) {
        for (ASTNode child: propertiesList->getChildren()) {
            addChild(child);
        }
    }
    m_rate = -1;
    m_CompilerProperties = make_shared<ListNode>(__FILE__, __LINE__);
}

FunctionNode::FunctionNode(string name, ASTNode scope, ASTNode propertiesList,
                           const char *filename, int line) :
    AST(AST::Function, filename, line)
{
    m_name = name;
    if (propertiesList) {
        for (ASTNode child: propertiesList->getChildren()) {
            addChild(child);
        }
    }
    resolveScope(scope);
    m_rate = -1;
    m_CompilerProperties = make_shared<ListNode>(__FILE__, __LINE__);
}

FunctionNode::~FunctionNode()
{

}

void FunctionNode::addChild(ASTNode t)
{
    AST::addChild(t);
    assert(t->getNodeType() == AST::Property);
    m_properties.push_back(static_pointer_cast<PropertyNode>(t));
}



void FunctionNode::setChildren(vector<ASTNode> &newChildren)
{
    AST::setChildren(newChildren);
    for (unsigned int i = 0; i < m_children.size(); i++) {
        assert(m_children.at(i)->getNodeType() == AST::Property);
        m_properties.push_back(static_pointer_cast<PropertyNode>(m_children.at(i)));
    }
}

ASTNode FunctionNode::getDomain()
{
    ASTNode domainValue = getPropertyValue("domain");
    return domainValue;
}

void FunctionNode::setDomainString(string domain)
{
    bool domainSet = false;
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == "domain") {
            m_properties.at(i)->replaceValue(std::make_shared<ValueNode>(domain, __FILE__, __LINE__));
            domainSet = true;
        }
    }
    if (!domainSet) {
        addProperty(std::make_shared<PropertyNode>("domain", std::make_shared<ValueNode>(domain, __FILE__, __LINE__), __FILE__, __LINE__));
    }
}

//void FunctionNode::deleteChildren()
//{
////    AST::deleteChildren();
////    m_properties.clear();
//}

vector<std::shared_ptr<PropertyNode>> FunctionNode::getProperties() const
{
    return m_properties;
}

void FunctionNode::addProperty(std::shared_ptr<PropertyNode> newProperty)
{
    if (!getPropertyValue(newProperty->getName())) {
        addChild(newProperty);
//        m_properties.push_back(newProperty);
    }
}

ASTNode FunctionNode::getPropertyValue(string propertyName)
{
    for (unsigned int i = 0; i < m_properties.size(); i++) {
        if (m_properties.at(i)->getName() == propertyName) {
            return m_properties.at(i)->getValue();
        }
    }
    return nullptr;
}

void FunctionNode::setPropertyValue(string propertyName, ASTNode value)
{
    if (getPropertyValue(propertyName)) {
        replacePropertyValue(propertyName, value);
    } else {
        addProperty(std::make_shared<PropertyNode>(propertyName, value, value->getFilename().c_str(), value->getLine()));
    }
}

bool FunctionNode::replacePropertyValue(string propertyName, ASTNode newValue)
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

void FunctionNode::resolveScope(ASTNode scope)
{
    if (scope) {
        for (unsigned int i = 0; i < scope->getChildren().size(); i++) {
            assert(scope->getChildren().at(i)->getNodeType() == AST::Scope);
            m_scope.push_back((static_pointer_cast<ScopeNode>(scope->getChildren().at(i)))->getName());
        }
    }
}

ASTNode FunctionNode::deepCopy()
{
    AST *newProps = new AST();
    for(unsigned int i = 0; i< m_properties.size(); i++) {
        newProps->addChild(m_properties[i]->deepCopy());
    }
    std::shared_ptr<FunctionNode> newFunctionNode
            = std::make_shared<FunctionNode>(m_name, std::shared_ptr<AST>(newProps), m_filename.data(), m_line);
    for (unsigned int i = 0; i < this->getScopeLevels(); i++) {
        newFunctionNode->addScope(this->getScopeAt(i));
    }
    newFunctionNode->setRate(getRate());
    newFunctionNode->m_CompilerProperties = this->m_CompilerProperties;
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

