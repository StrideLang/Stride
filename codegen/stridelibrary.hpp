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

#ifndef STRIDELIBRARY_HPP
#define STRIDELIBRARY_HPP

#include <vector>

#include <QString>
#include <QList>
#include <QMap>

#include "declarationnode.h"
#include "langerror.h"

class StrideLibrary
{
public:
    StrideLibrary();
    StrideLibrary(QString libraryPath, QMap<QString,QString> importList = QMap<QString,QString>());
   ~StrideLibrary();

    void setLibraryPath(QString libraryPath, QMap<QString,QString> importList = QMap<QString,QString>());

    DeclarationNode *findTypeInLibrary(QString typeName);

    bool isValidBlock(DeclarationNode *block);

    std::vector<AST *> getLibraryMembers();

private:

    bool isValidProperty(PropertyNode *property, DeclarationNode *type);
    QList<DeclarationNode *> getParentTypes(DeclarationNode *type);

    void readLibrary(QString rootDir, QMap<QString, QString> importList);
    QList<AST *> m_libraryTrees;
    int m_majorVersion;
    int m_minorVersion;
};

#endif // STRIDELIBRARY_HPP
