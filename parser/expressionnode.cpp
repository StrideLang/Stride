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

#include "expressionnode.h"
#include "listnode.h"

using namespace std;

ExpressionNode::ExpressionNode(ExpressionType type, ASTNode left, ASTNode right,
                               const char *filename, int line) :
    AST(AST::Expression, filename, line)
{
    m_type = type;
    assert(m_type != ExpressionNode::UnaryMinus && m_type != ExpressionNode::LogicalNot);
    addChild(left);
    addChild(right);

    m_CompilerProperties = make_shared<ListNode>(__FILE__, __LINE__);
}

ExpressionNode::ExpressionNode(ExpressionNode::ExpressionType type, ASTNode value,
                               const char *filename, int line) :
    AST(AST::Expression, filename, line)
{
    m_type = type;
    assert(m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
    addChild(value);

    m_CompilerProperties = make_shared<ListNode>(__FILE__, __LINE__);
}

ExpressionNode::~ExpressionNode()
{

}

bool ExpressionNode::isUnary() const
{
    return (m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
}

ASTNode ExpressionNode::getLeft() const
{
    assert(!this->isUnary());
    return m_children.at(0);
}

ASTNode ExpressionNode::getRight() const
{
    assert(!this->isUnary());
    return m_children.at(1);
}

void ExpressionNode::replaceLeft(ASTNode newLeft)
{
    assert(!this->isUnary());
    m_children.at(0) = newLeft;
//    ASTNode right = getRight();
////    getLeft()->deleteChildren();
//    m_children.clear();
//    addChild(newLeft);
//    addChild(right);
}

void ExpressionNode::replaceRight(ASTNode newRight)
{
    assert(!this->isUnary());
    m_children.at(1) = newRight;
//    ASTNode left = getLeft();
//    getRight()->deleteChildren();
//    m_children.clear();
//    addChild(left);
//    addChild(newRight);
}

void ExpressionNode::replaceValue(ASTNode newValue)
{
	assert(this->isUnary());
    m_children.at(0) = newValue;
//    deleteChildren();
//    m_children.push_back(newValue);
}

ASTNode ExpressionNode::getValue() const
{
    assert(isUnary());
    return m_children.at(0);
}

ExpressionNode::ExpressionType ExpressionNode::getExpressionType() const
{
    return m_type;
}

string ExpressionNode::getExpressionTypeString() const
{
    switch (m_type) {
    case Multiply:
        return "Multiply";
    case Divide:
        return "Divide";
    case Add:
        return "Add";
    case Subtract:
        return "Subtract";
    case And:
        return "And";
    case Or:
        return "Or";
    case UnaryMinus:
        return "UnaryMinus";
    case LogicalNot:
        return "LogicalNot";
    default:
        return "";
    }
}

ASTNode ExpressionNode::deepCopy()
{
    if (m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot) {
        auto newNode = std::make_shared<ExpressionNode>(m_type, m_children.at(0)->deepCopy(), m_filename.data(), m_line);
        newNode->m_CompilerProperties = this->m_CompilerProperties;
        return newNode;

    } else {
        auto newNode = std::make_shared<ExpressionNode>(m_type, m_children.at(0)->deepCopy(), m_children.at(1)->deepCopy(), m_filename.data(), m_line);
        newNode->m_CompilerProperties = this->m_CompilerProperties;
        return newNode;
    }
}


