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

#ifndef STRIDESYSTEM_HPP
#define STRIDESYSTEM_HPP

#include <QList>
#include <QString>
#include <QStringList>
#include <QLibrary>
#include <QMap>
#include <QFileInfo>
#include <QDir>

#include "strideparser.h"
#include "stridelibrary.hpp"
#include "strideplatform.hpp"

#include "builder.h"

class StrideSystem
{
public:
    StrideSystem() {}
    StrideSystem(QString strideRoot, QString systemName,
                 int majorVersion, int minorVersion,
                 QMap<QString, QStringList> importList);
    ~StrideSystem();

    QStringList getErrors();
    QStringList getWarnings();
    QStringList getPlatformTypeNames();
    QStringList getFunctionNames();

    void enableTesting(bool enable); // Uses testing objects instead of regular platform objects

    vector<Builder *> createBuilders(QString fileName, vector<string> usedFrameworks = vector<string>());

    ASTNode getPlatformDomain(string namespaceName = "");  // The platform's default domain

//    DeclarationNode *getFunction(QString functionName);
    vector<string> getFrameworkNames();
    map<string, vector<ASTNode>> getBuiltinObjectsCopy();
    map<string, vector<ASTNode> > getBuiltinObjectsReference(); // The key to the map is the namespace name

//    bool typeHasPort(QString typeName, QString propertyName);

    vector<ASTNode> getOptionTrees();

    ASTNode getImportTree(std::string importName, std::string platformName);

    std::map<std::string, std::shared_ptr<StridePlatform>> m_platforms;
    vector<std::shared_ptr<DeclarationNode>> m_platformDefinitions;
    std::vector<std::shared_ptr<DeclarationNode>> m_connectionDefinitions;

private:
    QVector<ASTNode> getPortsForTypeBlock(DeclarationNode *block);
//    ListNode *getPortsForFunction(QString typeName);

    QString makeProject(QString fileName);

    QString readFile(QString fileName);

    void parseSystemTree(ASTNode systemTree);

    QString m_strideRoot;
    QString m_systemName;
    int m_majorVersion;
    int m_minorVersion;
    QString m_systemPath;
    QString m_rootPath;
    bool m_testing;

    QStringList m_errors;
    QStringList m_warnings;

    QStringList m_types;


    QMap<QString, QString> m_importList;
    StrideLibrary m_library;
};



#endif // STRIDESYSTEM_HPP
