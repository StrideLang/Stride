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

ExpressionNode::ExpressionNode(ExpressionType type, AST *left, AST *right,
                               const char *filename, int line) :
    AST(AST::Expression, filename, line)
{
    m_type = type;
    assert(m_type != ExpressionNode::UnaryMinus && m_type != ExpressionNode::LogicalNot);
    addChild(left);
    addChild(right);
}

ExpressionNode::ExpressionNode(ExpressionNode::ExpressionType type, AST *value,
                               const char *filename, int line) :
    AST(AST::Expression, filename, line)
{
    m_type = type;
    assert(m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
    addChild(value);
}

ExpressionNode::~ExpressionNode()
{

}

bool ExpressionNode::isUnary() const
{
    return (m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot);
}

AST *ExpressionNode::getLeft() const
{
    assert(!this->isUnary());
    return m_children.at(0);
}

AST *ExpressionNode::getRight() const
{
    assert(!this->isUnary());
    return m_children.at(1);
}

void ExpressionNode::replaceLeft(AST *newLeft)
{
    assert(!this->isUnary());
    AST *right = getRight();
    getLeft()->deleteChildren();
    delete getLeft();
    m_children.clear();
    addChild(newLeft);
    addChild(right);
}

void ExpressionNode::replaceRight(AST *newRight)
{
    assert(!this->isUnary());
    AST *left = getLeft();
    getRight()->deleteChildren();
    delete getRight();
    m_children.clear();
    addChild(left);
    addChild(newRight);
}

void ExpressionNode::replaceValue(AST *newValue)
{
	assert(this->isUnary());
    deleteChildren();
    m_children.push_back(newValue);
}

AST *ExpressionNode::getValue() const
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
        break;
    case Divide:
        return "Divide";
        break;
    case Add:
        return "Add";
        break;
    case Subtract:
        return "Subtract";
        break;
    case And:
        return "And";
        break;
    case Or:
        return "Or";
        break;
    case UnaryMinus:
        return "UnaryMinus";
        break;
    case LogicalNot:
        return "LogicalNot";
        break;
    default:
        return "";
    }
}

AST *ExpressionNode::deepCopy()
{
    if (m_type == ExpressionNode::UnaryMinus || m_type == ExpressionNode::LogicalNot) {
        return new ExpressionNode(m_type, m_children.at(0)->deepCopy(), m_filename.data(), m_line);
    } else {
        return new ExpressionNode(m_type, m_children.at(0)->deepCopy(), m_children.at(1)->deepCopy(), m_filename.data(), m_line);
    }
}


