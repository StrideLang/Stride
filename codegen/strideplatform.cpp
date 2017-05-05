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

#include "strideplatform.hpp"

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
    auto it = m_platformTrees.begin();
    while(it != m_platformTrees.end()) {
        it->second->deleteChildren();
        delete it->second;
        it++;
    }
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

string StridePlatform::buildLibPath(string strideRoot)
{
    string path = buildPlatformPath(strideRoot) + "/";
    path += "platformlib";
    return path;
}

void StridePlatform::addTree(string treeName, AST *treeRoot)
{
    if (m_platformTrees.find(treeName) != m_platformTrees.end()) {
        m_platformTrees[treeName]->deleteChildren();
        delete m_platformTrees[treeName];
    }
    m_platformTrees[treeName] = treeRoot;
}

vector<AST *> StridePlatform::getPlatformObjectsReference()
{
    vector<AST *> objects;
    auto blockGroup = m_platformTrees.begin();
    while (blockGroup != m_platformTrees.end()) {
        foreach(AST *element, blockGroup->second->getChildren()) {
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
