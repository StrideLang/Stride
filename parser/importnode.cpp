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

#include "importnode.h"
#include "scopenode.h"

using namespace std;

ImportNode::ImportNode(string name, ASTNode scope, const char *filename, int line, string alias) :
    AST(AST::Import, filename, line)
{
    m_importName = name;
    m_importAlias = alias;
    resolveScope(scope);
}

ImportNode::ImportNode(string name, const char *filename, int line, string alias) :
    AST(AST::Import, filename, line)
{
    m_importName = name;
    m_importAlias = alias;
}

string ImportNode::importName() const
{
    return m_importName;
}

void ImportNode::setImportName(const string &importName)
{
    m_importName = importName;
}
string ImportNode::importAlias() const
{
    return m_importAlias;
}

void ImportNode::setImportAlias(const string &importAlias)
{
    m_importAlias = importAlias;
}

void ImportNode::resolveScope(ASTNode scope)
{
    if (scope) {
        for (unsigned int i = 0; i < scope->getChildren().size(); i++) {
            assert(scope->getChildren().at(i)->getNodeType() == AST::Scope);
            m_scope.push_back((static_cast<ScopeNode *>(scope->getChildren().at(i).get()))->getName());
        }
    }
}

ASTNode ImportNode::deepCopy()
{
    auto newImportNode = std::make_shared<ImportNode>(m_importName, m_filename.data(), getLine(), m_importAlias);
    for (unsigned int i = 0; i < this->getScopeLevels(); i++) {
        newImportNode->addScope(this->getScopeAt(i));
    }
    newImportNode->m_CompilerProperties = this->m_CompilerProperties;
    return newImportNode;
}

