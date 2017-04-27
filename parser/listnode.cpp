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

#include "listnode.h"

ListNode::ListNode(AST *newMember, const char *filename, int line) :
    AST(AST::List, filename, line)
{
    if (newMember) {
        addChild(newMember);
    }
}

ListNode::~ListNode()
{

}

void ListNode::stealMembers(ListNode *list)
{
    list->giveChildren(this);
}

AST::Token ListNode::getListType()
{
    vector<AST *> children = getChildren();
    if (children.size() == 0) {
        return AST::Invalid;
    }
    Token type = children.at(0)->getNodeType();

    for(unsigned int i = 1; i < children.size(); i++) {
        Token nextType = children.at(i)->getNodeType();
        if (type == AST::Int && nextType == AST::Real) {
            type = AST::Real;
        } else if (type == AST::Real && nextType == AST::Int) {
            // Consider int as real
        } else if(nextType != type) {
            return AST::Invalid;
        }
    }
    return type;
}

int ListNode::size()
{
    return m_children.size();
}

void ListNode::replaceMember(AST *replacement, AST *member)
{
//    vector<AST *> children = getChildren();
    for(unsigned int i = 0; i < m_children.size(); i++) {
        if (m_children.at(i) == member) {
            m_children.at(i) = replacement;
            member->deleteChildren();
            delete member;
            return;
        }
    }
}

AST *ListNode::deepCopy()
{
    vector<AST *> children = getChildren();
    ListNode *newList;
    if (children.size() > 0) {
        newList = new ListNode(children.at(0)->deepCopy(), m_filename.data(), m_line);
    } else {
        newList = new ListNode(NULL, m_filename.data(), m_line);
    }
    for(unsigned int i = 1; i < children.size(); i++) {
        newList->addChild(children.at(i)->deepCopy());
    }
    return newList;
}

