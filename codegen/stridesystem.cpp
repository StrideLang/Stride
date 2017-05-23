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

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "stridesystem.hpp"

#include "propertynode.h"
#include "declarationnode.h"
#include "pythonproject.h"
#include "codevalidator.h"


StrideSystem::StrideSystem(QString strideRoot, QString systemName,
                           int majorVersion, int minorVersion,
                           QMap<QString, QString> importList) :
    m_strideRoot(strideRoot), m_systemName(systemName), m_majorVersion(majorVersion), m_minorVersion(minorVersion)
{
    QString versionString = QString("%1.%2").arg(m_majorVersion).arg(m_minorVersion);
    QString fullPath = QDir(strideRoot + QDir::separator()
                            + "systems" + QDir::separator()
                            + systemName + QDir::separator()
                            + versionString).absolutePath();
    QString systemFile = fullPath + QDir::separator() + "System.stride";

    m_library.setLibraryPath(strideRoot, importList);
    if (QFile::exists(systemFile)) {
        AST *systemTree = AST::parseFile(systemFile.toStdString().c_str());
        if (systemTree) {
            parseSystemTree(systemTree);

            // Add subpaths for included modules
            vector<string> subPaths;
            subPaths.push_back("");
            QMapIterator<QString, QString> it(importList);
            while (it.hasNext()) {
                it.next();
                subPaths.push_back(it.key().toStdString());
            }
            // Iterate through platforms reading them.
            // TODO Should optimize this to not reread platform if already done.
            for(StridePlatform *platform: m_platforms) {
                QStringList nameFilters;
                nameFilters.push_back("*.stride");
                string platformPath = platform->buildLibPath(m_strideRoot.toStdString());
                foreach(string subPath, subPaths) {
                    QString includeSubPath = QString::fromStdString(platformPath + "/" + subPath);
                    QStringList libraryFiles =  QDir(includeSubPath).entryList(nameFilters);
                    foreach (QString file, libraryFiles) {
                        QString fileName = includeSubPath + QDir::separator() + file;
                        AST *tree = AST::parseFile(fileName.toLocal8Bit().data());
                        if(tree) {
                            platform->addTree(file.toStdString(),tree); // TODO check if this is being freed
                        } else {
                            vector<LangError> errors = AST::getParseErrors();
                            foreach(LangError error, errors) {
                                qDebug() << QString::fromStdString(error.getErrorText());
                            }
                            continue;
                        }
                        QString namespaceName = importList[QString::fromStdString(subPath)];
                        if (!namespaceName.isEmpty()) {
                            for(AST *node : tree->getChildren()) {
                                // Do we need to set namespace recursively or would this do?
                                //                            node->setNamespace(namespaceName.toStdString());
                            }
                        }
                    }
                }
//                m_platformPath = fullPath;
//                m_api = PythonTools;
//                m_types = getPlatformTypeNames();
            }
            systemTree->deleteChildren();
            delete systemTree;
        } else {
            qDebug() << "Error parsing system tree in:" << systemFile;
        }
    } else {
        qDebug() << "System file not found:" << systemFile;
    }

//    if (m_api == NullPlatform) {
//        qDebug() << "Platform not found!";
//    }
}

StrideSystem::~StrideSystem()
{
    for(DeclarationNode *node: m_platformDefinitions) {
        node->deleteChildren();
        delete node;
    }
    for(auto platform: m_platforms) {
        delete platform;
    }
}

//ListNode *StreamPlatform::getPortsForFunction(QString typeName)
//{
//    foreach(AST* group, m_platform) {
//        foreach(AST *node, group->getChildren()){
//            if (node->getNodeType() == AST::Declaration) {
//                DeclarationNode *block = static_cast<DeclarationNode *>(node);
//                if (block->getObjectType() == "platformModule") {
//                    if (block->getObjectType() == typeName.toStdString()) {
//                        vector<PropertyNode *> ports = block->getProperties();
//                        foreach(PropertyNode *port, ports) {
//                            if (port->getName() == "ports") {
//                                ListNode *portList = static_cast<ListNode *>(port->getValue());
//                                Q_ASSERT(portList->getNodeType() == AST::List);
//                                return portList;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//    return NULL;
//}

//DeclarationNode *StreamPlatform::getFunction(QString functionName)
//{
//    QStringList typeNames;
//    foreach(AST* node, m_platform) {
//        if (node->getNodeType() == AST::Declaration) {
//            DeclarationNode *block = static_cast<DeclarationNode *>(block);
//            if (block->getObjectType() == "platformModule"
//                    || block->getObjectType() == "module") {
//                if (block->getName() == functionName.toStdString()) {
//                    return block;
//                }
//            }
//        }
//    }
//    foreach(AST* node, m_library.getNodes()) {

//        if (node->getNodeType() == AST::Declaration) {
//            DeclarationNode *block = static_cast<DeclarationNode *>(node);
//            if (block->getObjectType() == "module") {
//                if (block->getName() == functionName.toStdString()) {
//                    return block;
//                }
//            }
//        }
//    }
//    return NULL;
//}

void StrideSystem::parseSystemTree(AST *systemTree)
{
    vector<string> usedPlatformNames;
    vector<map<string, string>> platformDefinitions;
    vector<string> platformDefinitionNames;
    vector<AST *> connectionDefinitions;
    for(AST *systemNode:systemTree->getChildren()) {
        if(systemNode->getNodeType() == AST::Declaration) {
            DeclarationNode *declaration = static_cast<DeclarationNode *>(systemNode);
            if (declaration->getObjectType() == "platform") {
                map<string, string> definition;
                vector<PropertyNode *> properties = declaration->getProperties();
                for (PropertyNode *prop: properties) {
                    if (prop->getValue()->getNodeType() == AST::String) {
                        definition[prop->getName()] =
                                static_cast<ValueNode *>(prop->getValue())->getStringValue();
                    }
                }
                platformDefinitions.push_back(definition);
                platformDefinitionNames.push_back(declaration->getName());
                m_platformDefinitions.push_back(static_cast<DeclarationNode *>(declaration->deepCopy()));
            } else if (declaration->getObjectType() == "connection") {
                // FIXME add connections
//                connectionDefinitions.push_back(declaration->deepCopy());
            } else if (declaration->getObjectType() == "system") {
                AST *platforms = declaration->getPropertyValue("platforms");
                if (platforms->getNodeType() == AST::List) {
                    ListNode *platformsList = static_cast<ListNode *>(platforms);
                    for(AST *platformName:platformsList->getChildren()) {
                        if (platformName->getNodeType() == AST::Block) {
                            usedPlatformNames.push_back(static_cast<BlockNode *>(platformName)->getName());
                        }
                    }
                }
            } else {
                qDebug() << "ERROR: Unknown system declaration type";
            }
        }
    }

    // Now connect platforms referenced in system with defined platforms
    for(string usedPlatformName:usedPlatformNames) {
        for (int i = 0; i < platformDefinitionNames.size(); i++) {
            map<string, string> &definition = platformDefinitions.at(i);
            string &name = platformDefinitionNames.at(i);
            if (name == usedPlatformName) {
                string framework, framworkVersion, hardware, hardwareVersion;
                auto defIt = definition.begin();
                while(defIt != definition.end()) {
                    if (defIt->first == "framework") {
                        framework = defIt->second;
//                        qDebug() << "Found framework:" << QString::fromStdString(framework);
                    } else if (defIt->first == "frameworkVersion") {
                        framworkVersion = defIt->second;
                    } else if (defIt->first == "hardware") {
                        hardware = defIt->second;
                    } else if (defIt->first == "hardwareVersion") {
                        hardwareVersion = defIt->second;
                    } else {

                    }
                    defIt++;
                }
                StridePlatform *newPlatform = new StridePlatform(framework, framworkVersion, hardware, hardwareVersion);
                m_platforms.push_back(newPlatform);
            } else {
                // TODO add error
            }
        }
    }

}

QStringList StrideSystem::getErrors()
{
    return m_errors;
}

QStringList StrideSystem::getWarnings()
{
    return m_warnings;
}

QStringList StrideSystem::getPlatformTypeNames()
{
    QStringList typeNames;
//    foreach(AST* group, m_platform) {
//        foreach(AST *node, group->getChildren()) {
//            if (node->getNodeType() == AST::Declaration) {
//                DeclarationNode *block = static_cast<DeclarationNode *>(node);
//                if (block->getObjectType() == "platformType") {
//                    ValueNode *name = static_cast<ValueNode *>(block->getPropertyValue("typeName"));
//                    if (name) {
//                        Q_ASSERT(name->getNodeType() == AST::String);
//                        typeNames << QString::fromStdString(name->getStringValue());
//                    } else {
//                        qDebug() << "Error. platform Type missing typeName port.";
//                    }
//                }
//            }
//        }
//    }
    vector<AST *> libObjects = m_library.getLibraryMembers();
    foreach(AST *node, libObjects) {
        if (node->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(node);
            if (block->getObjectType() == "platformType"
                    || block->getObjectType() == "type") {
                ValueNode *name = static_cast<ValueNode *>(block->getPropertyValue("typeName"));
                Q_ASSERT(name->getNodeType() == AST::String);
                typeNames << QString::fromStdString(name->getStringValue());
            }
        }
    }
    return typeNames;
}

QStringList StrideSystem::getFunctionNames()
{
    QStringList funcNames;

    map<string, vector<AST *>> refObjects = getBuiltinObjectsReference();

    for (auto it = refObjects.begin(); it != refObjects.end(); it++ ) {

        foreach(AST* node, it->second) {
            if (node->getNodeType() == AST::Declaration) {
                DeclarationNode *block = static_cast<DeclarationNode *>(node);
                if (block->getObjectType() == "module") {
                    if (it->first.size() > 0) {
                        funcNames << QString::fromStdString(it->first) + ":" + QString::fromStdString(block->getName());
                    } else {
                        funcNames << QString::fromStdString(block->getName());
                    }
                }
            }
        }
    }
    return funcNames;
}

vector<Builder *> StrideSystem::createBuilders(QString projectDir, vector<string> usedFrameworks)
{
    vector<Builder *> builders;
    for (StridePlatform *platform: m_platforms) {
        if ( (usedFrameworks.size() == 0)
                || (std::find(usedFrameworks.begin(), usedFrameworks.end(), platform->getFramework()) != usedFrameworks.end())) {
            if (platform->getAPI() == StridePlatform::PythonTools) {
                QString pythonExec = "python";
                Builder *builder = new PythonProject(QString::fromStdString(platform->getFramework()),
                                                     QString::fromStdString(platform->buildPlatformPath(m_strideRoot.toStdString())),
                                                     m_strideRoot, projectDir, pythonExec);
                if (builder) {
                    if (builder->isValid()) {
                        builders.push_back(builder);
                    } else {
                        delete builder;
                    }
                }
            }/* else if(m_api == StrideSystem::PluginPlatform) {
        QString xmosRoot = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";

        QLibrary pluginLibrary(m_pluginName);
        if (!pluginLibrary.load()) {
            qDebug() << pluginLibrary.errorString();
            return NULL;
        }
        create_object_t create = (create_object_t) pluginLibrary.resolve("create_object");
        if (create) {
            builder = create(m_platformPath, projectDir.toLocal8Bit(), xmosRoot.toLocal8Bit());
        }
        pluginLibrary.unload();
    }*/
        }
    }
    return builders;
}

QString StrideSystem::getPlatformDomain(string namespaceName)
{
    map<string, vector<AST *>> libObjects = getBuiltinObjectsReference();
    if (libObjects.find(namespaceName) == libObjects.end()) { // Invalid namespace
        assert(0 == 1);
        return "";
    }
    for(AST *object:libObjects[namespaceName]) {
        if (object->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(object);
            if (block->getObjectType() == "constant"
                    && block->getName() == "PlatformDomain") {
                QList<LangError> errors;
                std::string domainName;

                if (block->getPropertyValue("value")->getNodeType() == AST::Block) {

                } else {
                    domainName = CodeValidator::evaluateConstString(block->getPropertyValue("value"), QVector<AST *>::fromStdVector(libObjects[namespaceName]), nullptr, errors);
                }
                return QString::fromStdString(domainName);
            }
        }
    }
    return "";
}

vector<string> StrideSystem::getFrameworkNames()
{
    vector<string> names;
    for(auto platform: m_platforms) {
        names.push_back(platform->getFramework());
    }
    return names;
}

map<string, vector<AST *> > StrideSystem::getBuiltinObjectsCopy()
{
    map<string, vector<AST *>> objects;
    map<string, vector<AST *>> refObjects = getBuiltinObjectsReference();

    for (auto it = refObjects.begin(); it != refObjects.end(); it++ ) {
        objects[it->first] = vector<AST *>();
        for(AST *object: it->second) {
            objects[it->first].push_back(object->deepCopy());
        }
    }
    return objects;
}

map<string, vector<AST *>> StrideSystem::getBuiltinObjectsReference()
{
    map<string, vector<AST *>> objects;
    objects[""] = vector<AST *>();
    // Add all platform objects with their namespace name, so the namespaced version is picked up first
    for (auto platform: m_platforms) {
        objects[platform->getFramework()] = platform->getPlatformObjectsReference();
    }
    // Then put first platform's objects in default namespace
    if (m_platforms.size() > 0) {
        objects[""] = m_platforms.at(0)->getPlatformObjectsReference();
    }

    // finally add library objects.
    for(AST *object: m_library.getLibraryMembers()) {
        objects[""].push_back(object);
    }
    for(DeclarationNode *decl: m_platformDefinitions) {
        objects[""].push_back(decl);
    }
    return objects;
}
