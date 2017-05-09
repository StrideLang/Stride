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

#ifndef DECLARATIONNODE_H
#define DECLARATIONNODE_H

#include <string>

#include "ast.h"
#include "bundlenode.h"
#include "propertynode.h"

class DeclarationNode : public AST
{
public:
    DeclarationNode(string name, string objectType, AST *propertiesList, const char *filename, int line, vector<string> scope = vector<string>());
    DeclarationNode(BundleNode *bundle, string objectType, AST *propertiesList, const char *filename, int line, vector<string> scope = vector<string>());
    ~DeclarationNode();

    string getName() const;
    BundleNode *getBundle() const;
    vector<PropertyNode *> getProperties() const;
    bool addProperty(PropertyNode *newProperty);
    AST *getPropertyValue(string propertyName);
    void setPropertyValue(string propertyName, AST *value);
    void replacePropertyValue(string propertyName, AST *newValue);

    AST *getDomain();
    void setDomainString(string domain);

    string getObjectType() const;
    AST *deepCopy();

private:
    string m_name;
    string m_objectType;
    vector<PropertyNode *> m_properties;
};

#endif // DECLARATIONNODE_H
