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

#include "bundlenode.h"
#include "scopenode.h"

BundleNode::BundleNode(string name, ListNode *indexList, const char *filename, int line, vector<string> scope) :
    AST(AST::Bundle, filename, line, scope)
{
    addChild(indexList);
    m_name = name;
}

BundleNode::BundleNode(string name, AST *scope, ListNode *indexList, const char *filename, int line) :
    AST(AST::Bundle, filename, line)
{
    addChild(indexList);
    m_name = name;
    resolveScope(scope);
}

BundleNode::~BundleNode()
{

}

string BundleNode::getName() const
{
    return m_name;
}

ListNode *BundleNode::index() const
{
    return static_cast<ListNode *>(m_children.at(0));
}

void BundleNode::resolveScope(AST *scope)
{
    if (scope) {
        for (unsigned int i = 0; i < scope->getChildren().size(); i++) {
            assert(scope->getChildren().at(i)->getNodeType() == AST::Scope);
            m_scope.push_back((static_cast<ScopeNode *>(scope->getChildren().at(i)))->getName());
        }
    }
}

AST *BundleNode::deepCopy()
{
    assert(getNodeType() == AST::Bundle);
    if(getNodeType() == AST::Bundle) {
        AST *newBundle = new BundleNode(m_name, static_cast<ListNode *>(index()->deepCopy()), m_filename.data(), m_line);
        newBundle->setRate(m_rate);
        for (unsigned int i = 0; i < this->getScopeLevels(); i++) {
            newBundle->addScope(this->getScopeAt(i));
        }
        return newBundle;
    }
    assert(0 == 1);
    return 0;
}
