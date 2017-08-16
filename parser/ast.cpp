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

#include "ast.h"

extern AST *parse(const char* fileName, const char* sourceFilename);
extern std::vector<LangError> getErrors();

AST::AST()
{
    m_token = AST::None;
    m_line = -1;
}

AST::AST(Token token, const char *filename, int line, vector<string> scope)
{
    m_token = token;
    m_filename.append(filename);
    m_line = line;
    m_scope = scope;
}

AST::~AST()
{

}

void AST::addChild(ASTNode t) {
    m_children.push_back(t);
}

//void AST::giveChildren(ASTNode p)
//{
//    for(size_t i = 0; i < m_children.size(); i++) {
//        p->addChild(m_children.at(i));
//    }
//    m_children.clear();
//}

void AST::setChildren(vector<ASTNode> &newChildren)
{
//    deleteChildren();
    m_children = newChildren;
}

//void AST::deleteChildren()
//{
////    for(size_t i = 0; i < m_children.size(); i++) {
////        m_children.at(i)->deleteChildren();
////        m_children.at(i).reset();
////    }
////    m_children.clear();
//}

ASTNode AST::deepCopy()
{
    ASTNode newNode = std::make_shared<AST>(AST::None, m_filename.data(), m_line, m_scope);
    for(unsigned int i = 0; i < m_children.size(); i++) {
        newNode->addChild(m_children.at(i)->deepCopy());
    }
    return newNode;
}

ASTNode AST::parseFile(const char *fileName, const char* sourceFilename)
{
    return std::shared_ptr<AST>(parse(fileName, sourceFilename));
}

vector<LangError> AST::getParseErrors()
{
    return getErrors();
}

string AST::getFilename() const
{
    return m_filename;
}

void AST::setFilename(const string &filename)
{
    m_filename = filename;
}

void AST::resolveScope(ASTNode scope)
{
    (void) scope; // To remove warning
    assert(0 == 1); // Each type should resolve its scope
}

void AST::addScope(string newScope)
{
    m_scope.push_back(newScope);
}

void AST::setRootScope(string scopeName)
{
    if (scopeName != "") {
        if (m_scope.size() == 0 || m_scope.at(0) != scopeName) {
            m_scope.insert(m_scope.begin(), scopeName);
        }
    }
}


size_t AST::getScopeLevels()
{
    return m_scope.size();
}

string AST::getScopeAt(unsigned int scopeLevel)
{
    return m_scope.at(scopeLevel);
}

vector<string> AST::getNamespaceList()
{
    return m_scope;
}

void AST::setNamespaceList(vector<string> list)
{
    m_scope = list;
}
