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

#ifndef AST_H
#define AST_H

#include <memory>

#include "langerror.h"

using namespace std;

class AST;

typedef shared_ptr<AST> ASTNode;

class AST
{
public:
    AST();

    typedef enum {
        None,
        Platform,
        Bundle,
        Declaration,
        BundleDeclaration,
        Stream,
        Property,
        Range,
        List,
        Import,
        For,
        Scope,
        PortProperty,

        // Built-in types (leaf nodes)
        Int = 0x80,
        Real = 0x81,
        String = 0x82,
        Switch = 0x83,

        Block = 0x20,
        Expression = 0x21,
        Function = 0x22,
        Keyword = 0x23,

        // Invalid
        Invalid
    } Token;

    AST(Token token, const char *filename, int line = -1, vector<string> scope = vector<string>());
    virtual ~AST();

    Token getNodeType() const { return m_token; }
    virtual void addChild(ASTNode t);
//    void giveChildren(ASTNode p); // Move all children nodes to be children of "parent" and make parent a child of this class
    bool isNil() { return m_token == AST::None; }

    vector<ASTNode> getChildren() const {return m_children;}
    virtual void setChildren(vector<ASTNode> &newChildren);

    int getLine() const {return m_line;}

//    virtual void deleteChildren();

    virtual ASTNode deepCopy();


    static ASTNode parseFile(const char *fileName, const char* sourceFilename = nullptr);
    static vector<LangError> getParseErrors();

    string getFilename() const;
    void setFilename(const string &filename);

    void addScope(string newScope);
    void setRootScope(string scopeName);
    size_t getScopeLevels();
    string getScopeAt(unsigned int scopeLevel);

    vector<string> getNamespaceList();
    void setNamespaceList(vector<string> list);

protected:
    virtual void resolveScope(ASTNode scope);

    Token m_token; // From which token did we create node?
    vector<ASTNode> m_children; // normalized list of children
    string m_filename; // file where the node was generated
    int m_line;
    vector<string> m_scope;
};

#endif // AST_H
