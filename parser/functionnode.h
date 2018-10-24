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

#ifndef FUNCTIONNODE_H
#define FUNCTIONNODE_H

#include <string>

#include "ast.h"
#include "propertynode.h"

class FunctionNode : public AST
{
public:

    FunctionNode(std::string name, ASTNode propertiesList, const char *filename, int line);
    FunctionNode(std::string name, ASTNode scope, ASTNode propertiesList, const char *filename, int line);
    ~FunctionNode();

    virtual void addChild(ASTNode t) override;
    virtual void setChildren(std::vector<ASTNode > &newChildren) override;
//    virtual void deleteChildren() override;

    std::string getName() const { return m_name; }
    std::vector<std::shared_ptr<PropertyNode>> getProperties() const;

    void addProperty(std::shared_ptr<PropertyNode> newProperty);
    ASTNode getPropertyValue(std::string propertyName);
    void setPropertyValue(std::string propertyName, ASTNode value);
    bool replacePropertyValue(std::string propertyName, ASTNode newValue);

    ASTNode getDomain();
    void setDomainString(std::string domain);

    virtual void resolveScope(ASTNode scope) override;

    virtual ASTNode deepCopy() override;

    // FIXME We should provide input and output rates for functions.
    double getRate() const;
    void setRate(double rate);

private:
    double m_rate;
    std::string m_name;
    std::vector<std::shared_ptr<PropertyNode>> m_properties;
};

#endif // FUNCTIONNODE_H
