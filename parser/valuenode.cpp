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
#include <sstream>

#include "valuenode.h"

ValueNode::ValueNode(const char * filename, int line) :
    AST(AST::None, filename, line)
{
}

ValueNode::ValueNode(int value, const char * filename, int line) :
    AST(AST::Int, filename, line)
{
    m_intValue = value;
}

ValueNode::ValueNode(float value, const char * filename, int line) :
    AST(AST::Real, filename, line)
{
    m_floatValue = value;
}

ValueNode::ValueNode(double value, const char * filename, int line) :
    AST(AST::Real, filename, line)
{
    m_floatValue = value;
}

ValueNode::ValueNode(string value, const char * filename, int line) :
    AST(AST::String, filename, line)
{
    m_stringValue = value;
}

ValueNode::ValueNode(bool value, const char * filename, int line) :
    AST(AST::Switch, filename, line)
{
    m_switch = value;
}

ValueNode::~ValueNode()
{

}

int ValueNode::getIntValue() const
{
    assert(getNodeType() == AST::Int);
    return m_intValue;
}

double ValueNode::getRealValue() const
{
    assert(getNodeType() == AST::Real);
    return m_floatValue;
}

double ValueNode::toReal() const
{
    if (getNodeType() == AST::Real) {
        return m_floatValue;
    } else if (getNodeType() == AST::Int) {
        return m_intValue;
    }
    return 0;
}

string ValueNode::getStringValue() const
{
    assert(getNodeType() == AST::String);
    return m_stringValue;
}

string ValueNode::toString() const
{
    if (getNodeType() == AST::Real) {
        return std::to_string(m_floatValue);
    } else if (getNodeType() == AST::Int) {
        return std::to_string(m_intValue);
    } else if (getNodeType() == AST::String) {
        return m_stringValue;
    } else if (getNodeType() == AST::Switch) {
        if (m_switch) {
            return "On";
        } else {
            return "Off";
        }
    }
    return "";
}

bool ValueNode::getSwitchValue() const
{
    assert(getNodeType() == AST::Switch);
    return m_switch;
}

ASTNode ValueNode::deepCopy()
{
    if (getNodeType() == AST::Int) {
        return std::make_shared<ValueNode>(getIntValue(), m_filename.c_str(), getLine());
    } else if (getNodeType() == AST::Real) {
        return std::make_shared<ValueNode>(getRealValue(), m_filename.c_str(), getLine());
    } else if (getNodeType() == AST::String) {
        return std::make_shared<ValueNode>(getStringValue(), m_filename.c_str(), getLine());
    } else if (getNodeType() == AST::Switch) {
        return std::make_shared<ValueNode>(getSwitchValue(), m_filename.c_str(), getLine());
    } else if (getNodeType() == AST::None) {
        return std::make_shared<ValueNode>(m_filename.data(), getLine());
    }  else {
        assert(0); // Invalid type
    }
}

void ValueNode::setDomain(ASTNode domain)
{
    m_domain = domain;
}

ASTNode ValueNode::getDomain()
{
    return m_domain;
}



