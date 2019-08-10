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

#include <iostream>

#include "strideplatform.hpp"
#include "../parser/declarationnode.h"
#include "../parser/valuenode.h"

using namespace std;

//bool StreamPlatform::typeHasPort(QString typeName, QString propertyName)
//{
//    QVector<AST *> ports = getPortsForType(typeName);
//    if (!ports.isEmpty()) {
//        foreach(AST *port, ports) {
//            DeclarationNode *block = static_cast<DeclarationNode *>(port);
//            Q_ASSERT(block->getNodeType() == AST::Declaration);
//            ValueNode *nameValueNode = static_cast<ValueNode *>(block->getPropertyValue("name"));
//            Q_ASSERT(nameValueNode->getNodeType() == AST::String);
//            if (nameValueNode->getStringValue() == propertyName.toStdString()) {
//                return true;
//            }
//        }
//    }
//    return false;
//}

StridePlatform::~StridePlatform()
{
//    auto it = m_platformTrees.begin();
//    while(it != m_platformTrees.end()) {
//        it->second->deleteChildren();
////        delete it->second;
//        it++;
//    }
}

string StridePlatform::getFramework() const
{
    return m_framework;
}

string StridePlatform::getFrameworkVersion() const
{
    return m_frameworkVersion;
}

string StridePlatform::getHardware() const
{
    return m_hardware;
}

string StridePlatform::getHardwareVersion() const
{
    return m_hardwareVersion;
}

string StridePlatform::getRootNamespace() const
{
    return m_rootNamespace;
}

bool StridePlatform::getRequired() const
{
    return m_required;
}

StridePlatform::PlatformAPI StridePlatform::getAPI() const
{
    return m_api;
}

string StridePlatform::buildPlatformPath(string strideRoot)
{
    string path = strideRoot + "/";
    path += "frameworks/" + m_framework + "/" + m_frameworkVersion;
    return path;
}

string StridePlatform::buildPlatformLibPath(string strideRoot)
{
    string path = buildPlatformPath(strideRoot) + "/";
    path += "platformlib";
    return path;
}

string StridePlatform::buildTestingLibPath(string strideRoot)
{
    string path = buildPlatformPath(strideRoot) + "/";
    path += "platformlib/testing";
    return path;
}

std::vector<string> StridePlatform::getDomainIds()
{
    for (auto treeEntry: m_platformTrees) {
        for (auto node: treeEntry.second->getChildren()) {

        }
    }
    return std::vector<string>();
}

void StridePlatform::addTree(string treeName, ASTNode treeRoot)
{
    if (m_platformTrees.find(treeName) != m_platformTrees.end()) {
        std::cerr << "WARNING: tree: '" << treeName << "' exists. Replacing." << std::endl;
    } else {
        m_platformTrees[treeName] = treeRoot;
    }
}

void StridePlatform::addTestingTree(string treeName, ASTNode treeRoot)
{
    m_platformTestTrees[treeName] = treeRoot;
}

vector<ASTNode> StridePlatform::getPlatformObjectsReference()
{
    vector<ASTNode> objects;
    auto blockGroup = m_platformTrees.begin();
    while (blockGroup != m_platformTrees.end()) {
        for(ASTNode element : blockGroup->second->getChildren()) {
            if (element->getNodeType() == AST::Declaration) {
                objects.push_back(element);
            } else {
                objects.push_back(element); // TODO: This inserts everything. Only insert what is needed
            }
        }
        blockGroup++;
    }
    return objects;
}

vector<ASTNode> StridePlatform::getPlatformTestingObjectsRef()
{
    vector<ASTNode> objects;
    auto blockGroup = m_platformTestTrees.begin();
    while (blockGroup != m_platformTestTrees.end()) {
        for(ASTNode element : blockGroup->second->getChildren()) {
            if (element->getNodeType() == AST::Declaration) {
                objects.push_back(element);
            } else {
                objects.push_back(element); // TODO: This inserts everything. Only insert what is needed
            }
        }
        blockGroup++;
    }
    return objects;
}

bool StridePlatform::getPluginDetails(string &pluginName, int &majorVersion, int &minorVersion)
{
    for(auto declNode: m_platformTrees[""]->getChildren()) {
        if (declNode->getNodeType() == AST::Declaration) {
            auto decl = static_pointer_cast<DeclarationNode>(declNode);
            if (decl->getObjectType() == "generatorDirectives") {
                auto nameNode = decl->getPropertyValue("codeGenerator");
                if (nameNode && nameNode->getNodeType() == AST::String) {
                    pluginName = static_pointer_cast<ValueNode>(nameNode)->getStringValue();
                }
                auto majorVersionNode = decl->getPropertyValue("codeGeneratorMajorVersion");
                if (majorVersionNode && majorVersionNode->getNodeType() == AST::Int) {
                    majorVersion = static_pointer_cast<ValueNode>(majorVersionNode)->getIntValue();
                }
                auto minorVersionNode = decl->getPropertyValue("codeGeneratorMinorVersion");
                if (minorVersionNode && minorVersionNode->getNodeType() == AST::Int) {
                    minorVersion = static_pointer_cast<ValueNode>(minorVersionNode)->getIntValue();
                }
                return true;
            }
        }
    }
    return false;
}
