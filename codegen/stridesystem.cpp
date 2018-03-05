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
                           QMap<QString, QStringList> importList) :
    m_strideRoot(strideRoot), m_systemName(systemName), m_majorVersion(majorVersion), m_minorVersion(minorVersion),
    m_testing(false)
{
    QString versionString = QString("%1.%2").arg(m_majorVersion).arg(m_minorVersion);
    m_systemPath = QDir(strideRoot + QDir::separator()
                            + "systems" + QDir::separator()
                            + systemName + QDir::separator()
                            + versionString).absolutePath();
    QString systemFile = m_systemPath + QDir::separator() + "System.stride";

    m_library.setLibraryPath(strideRoot, importList);


    if (QFile::exists(systemFile)) {
        ASTNode systemTree = AST::parseFile(systemFile.toStdString().c_str(), nullptr);
        if (systemTree) {
            parseSystemTree(systemTree);

            // Add subpaths for included modules
            vector<string> subPaths;
            subPaths.push_back("");
            QMapIterator<QString, QStringList> it(importList);
            while (it.hasNext()) {
                it.next();
                subPaths.push_back(it.key().toStdString());
            }
            // Iterate through platforms reading them.
            // TODO Should optimize this to not reread platform if already done.
            for(std::shared_ptr<StridePlatform> platform: m_platforms) {
                QStringList nameFilters;
                nameFilters.push_back("*.stride");
                string platformPath = platform->buildLibPath(m_strideRoot.toStdString());
                foreach(string subPath, subPaths) {
                    QString includeSubPath = QString::fromStdString(platformPath + "/" + subPath);
                    QStringList libraryFiles =  QDir(includeSubPath).entryList(nameFilters);
                    foreach (QString file, libraryFiles) {
                        QString fileName = includeSubPath + QDir::separator() + file;
                        ASTNode tree = AST::parseFile(fileName.toLocal8Bit().data(), nullptr);
                        if(tree) {
                            platform->addTree(file.toStdString(),tree);
                        } else {
                            vector<LangError> errors = AST::getParseErrors();
                            foreach(LangError error, errors) {
                                qDebug() << QString::fromStdString(error.getErrorText());
                            }
                            continue;
                        }

                        for(ASTNode node : tree->getChildren()) {
                            size_t indexExtension = file.toStdString().find(".stride");
                            string scopeName = file.toStdString().substr(0, indexExtension);
                            node->appendToPropertyValue("validScopes",
                                                        std::make_shared<ValueNode>(scopeName, __FILE__, __LINE__) );
                        }
                        for (QString namespaceName: importList[QString::fromStdString(subPath)]) {
                            if (!namespaceName.isEmpty()) {
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

            // Load testing trees
            for(std::shared_ptr<StridePlatform> platform: m_platforms) {
                QStringList nameFilters;
                nameFilters.push_back("*.stride");
                string platformPath = platform->buildTestingLibPath(m_strideRoot.toStdString());
                QFileInfoList libraryFiles =  QDir(QString::fromStdString(platformPath)).entryInfoList(nameFilters);
                for (auto fileInfo : libraryFiles) {
                    ASTNode tree = AST::parseFile(fileInfo.absoluteFilePath().toLocal8Bit().data(), nullptr);
                    if(tree) {
                        platform->addTestingTree(fileInfo.baseName().toStdString(),tree);
                    } else {
                        vector<LangError> errors = AST::getParseErrors();
                        foreach(LangError error, errors) {
                            qDebug() << QString::fromStdString(error.getErrorText());
                        }
                        continue;
                    }
                }
            }
//                m_platformPath = fullPath;
//                m_api = PythonTools;
//                m_types = getPlatformTypeNames();
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
//    return nullptr;
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
//    return nullptr;
//}

void StrideSystem::parseSystemTree(ASTNode systemTree)
{
    vector<string> usedPlatformNames;
    vector<map<string, string>> platformDefinitions;
    vector<string> platformDefinitionNames;
    vector<AST *> connectionDefinitions; // TODO process connection definitions
    for(ASTNode systemNode:systemTree->getChildren()) {
        if(systemNode->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> declaration = static_pointer_cast<DeclarationNode>(systemNode);
            if (declaration->getObjectType() == "platform") {
                map<string, string> definition;
                vector<std::shared_ptr<PropertyNode>> properties = declaration->getProperties();
                for (std::shared_ptr<PropertyNode> prop: properties) {
                    if (prop->getValue()->getNodeType() == AST::String) {
                        definition[prop->getName()] =
                                static_cast<ValueNode *>(prop->getValue().get())->getStringValue();
                    }
                }
                platformDefinitions.push_back(definition);
                platformDefinitionNames.push_back(declaration->getName());
                m_platformDefinitions.push_back(static_pointer_cast<DeclarationNode>(declaration));
            } else if (declaration->getObjectType() == "connection") {
                // FIXME add connections
//                connectionDefinitions.push_back(declaration->deepCopy());
            } else if (declaration->getObjectType() == "system") {
                ASTNode platforms = declaration->getPropertyValue("platforms");
                if (platforms->getNodeType() == AST::List) {
                    ListNode *platformsList = static_cast<ListNode *>(platforms.get());
                    for(ASTNode platformName:platformsList->getChildren()) {
                        if (platformName->getNodeType() == AST::Block) {
                            usedPlatformNames.push_back(static_cast<BlockNode *>(platformName.get())->getName());
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
        for (size_t i = 0; i < platformDefinitionNames.size(); i++) {
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
                std::shared_ptr<StridePlatform> newPlatform = std::make_shared<StridePlatform>(framework, framworkVersion, hardware, hardwareVersion);
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
    vector<ASTNode> libObjects = m_library.getLibraryMembers();
    for(ASTNode node : libObjects) {
        if (node->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(node);
            if (block->getObjectType() == "type"
                    || block->getObjectType() == "platformModule"
                    || block->getObjectType() == "platformBlock") {
                ValueNode *name = static_cast<ValueNode *>(block->getPropertyValue("typeName").get());
                if (name && name->getNodeType() == AST::String) {
                    typeNames << QString::fromStdString(name->getStringValue());
                }
            }
        }
    }
    return typeNames;
}

QStringList StrideSystem::getFunctionNames()
{
    QStringList funcNames;

    map<string, vector<ASTNode>> refObjects = getBuiltinObjectsReference();

    for (auto it = refObjects.begin(); it != refObjects.end(); it++ ) {

        for(ASTNode node : it->second) {
            if (node->getNodeType() == AST::Declaration) {
                DeclarationNode *block = static_cast<DeclarationNode *>(node.get());
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

void StrideSystem::enableTesting(bool enable)
{
    m_testing = enable;
}

vector<Builder *> StrideSystem::createBuilders(QString fileName, vector<string> usedFrameworks)
{
    vector<Builder *> builders;
    QString projectDir = makeProject(fileName);
    if (projectDir.isEmpty()) {
        qDebug() << "Error creating project path";
        return builders;
    }
    for (auto platform: m_platforms) {
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
            return nullptr;
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
    map<string, vector<ASTNode>> libObjects = getBuiltinObjectsReference();
    if (libObjects.find(namespaceName) == libObjects.end()) { // Invalid namespace
        assert(0 == 1);
        return "";
    }
    for(ASTNode object:libObjects[namespaceName]) {
        if (object->getNodeType() == AST::Declaration) {
            std::shared_ptr<DeclarationNode> block = static_pointer_cast<DeclarationNode>(object);
            if (block->getObjectType() == "constant"
                    && block->getName() == "PlatformDomain") {
                QList<LangError> errors;
                std::string domainName;

                if (block->getPropertyValue("value")->getNodeType() == AST::Block) {

                } else {
                    domainName = CodeValidator::evaluateConstString(block->getPropertyValue("value"), QVector<ASTNode >::fromStdVector(libObjects[namespaceName]), nullptr, errors);
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

map<string, vector<ASTNode> > StrideSystem::getBuiltinObjectsCopy()
{
    map<string, vector<ASTNode>> objects;
    map<string, vector<ASTNode>> refObjects = getBuiltinObjectsReference();

    for (auto it = refObjects.begin(); it != refObjects.end(); it++ ) {
        objects[it->first] = vector<ASTNode>();
        for(ASTNode object: it->second) {
            objects[it->first].push_back(object->deepCopy());
        }
    }
    return objects;
}

map<string, vector<ASTNode>> StrideSystem::getBuiltinObjectsReference()
{
    map<string, vector<ASTNode>> objects;
    objects[""] = vector<ASTNode>();
    for (auto platform: m_platforms) {
        vector<ASTNode> platformObjects = platform->getPlatformObjectsReference();

        if (m_testing) {
            vector<ASTNode> testingObjs = platform->getPlatformTestingObjectsRef();
            for (size_t i = 0; i < platformObjects.size(); i++) {
                if(platformObjects[i]->getNodeType() == AST::Declaration
                        || platformObjects[i]->getNodeType() == AST::BundleDeclaration) {
                    std::shared_ptr<DeclarationNode> decl = static_pointer_cast<DeclarationNode>(platformObjects[i]);
                    for(ASTNode testingObj: testingObjs) {
                        if (testingObj->getNodeType() == AST::Declaration
                                || testingObj->getNodeType() == AST::BundleDeclaration) {
                            std::shared_ptr<DeclarationNode> testDecl = static_pointer_cast<DeclarationNode>(testingObj);
                            if (decl->getName() == testDecl->getName()) { // FIXME we need to check for namespace too
                                platformObjects[i] = testDecl;
                                break;
                            }
                        } else {
                            qDebug() << "Unexpected node in testing file.";
                        }
                    }
                }
            }
        }

        // Add all platform objects with their namespace name
        objects[platform->getFramework()] = platformObjects;
        // Then put first platform's objects in default namespace
        if (m_platforms.at(0) == platform) {
            for (auto node: platformObjects) {
                objects[""].push_back(node);
                if (node->getNodeType() == AST::Declaration || node->getNodeType() == AST::BundleDeclaration) {
                    auto validScopes = node->getCompilerProperty("validScopes");
                    if (validScopes) {
                        assert(validScopes->getNodeType() == AST::List);
                        bool rootFound = false;
                        bool relativeScopeFound = false;
                        for (auto child: validScopes->getChildren()) {
                            std::shared_ptr<ValueNode> stringValue = std::static_pointer_cast<ValueNode>(child);
                            assert(child->getNodeType() == AST::String);
                            if (stringValue->getStringValue() == "::") {
                                rootFound = true;
                            } else if (stringValue->getStringValue() == "") {
                                relativeScopeFound = true;
                            }
                        }
                        if (!rootFound) {
                            validScopes->addChild(std::make_shared<ValueNode>(string("::"), __FILE__, __LINE__));
                        }
                        if (!relativeScopeFound) {
                            validScopes->addChild(std::make_shared<ValueNode>(string(""), __FILE__, __LINE__));
                        }
                    } else {
                        auto newList = std::make_shared<ListNode>(std::make_shared<ValueNode>(string("::"), __FILE__, __LINE__),
                                                                  __FILE__, __LINE__);
                        newList->addChild(std::make_shared<ValueNode>(string(""), __FILE__, __LINE__));
                        node->setCompilerProperty("validScopes", newList);
                    }

                }
            }
        }
    }

    // finally add library objects.
    for(ASTNode object: m_library.getLibraryMembers()) {
        objects[""].push_back(object);
//        auto nodeNoNameSpace = object->deepCopy();
//        nodeNoNameSpace->setNamespaceList(vector<string>());
//        objects[""].push_back(nodeNoNameSpace);
    }
    for(std::shared_ptr<DeclarationNode> decl: m_platformDefinitions) {
        objects[""].push_back(decl);
//        auto nodeNoNameSpace = decl->deepCopy();
//        nodeNoNameSpace->setNamespaceList(vector<string>());
//        objects[""].push_back(nodeNoNameSpace);
    }
    return objects;
}

vector<ASTNode> StrideSystem::getOptionTrees()
{
    vector<ASTNode> optionTrees;
    QStringList nameFilters;
    nameFilters.push_back("*.stride");
    QString optionPath = m_systemPath + QDir::separator() + "options";
    QFileInfoList optionFiles =  QDir(optionPath).entryInfoList(nameFilters);
    for (auto fileInfo : optionFiles) {
        ASTNode optionTree = AST::parseFile(fileInfo.absoluteFilePath().toStdString().c_str(), nullptr);
        if (optionTree) {
            optionTrees.push_back(optionTree);
        } else {
            qDebug() << "Error parsing option file: " << fileInfo.absolutePath();
        }
    }
    return optionTrees;
}

QString StrideSystem::makeProject(QString fileName)
{
    QFileInfo info(fileName);
    QString dirName = info.absolutePath() + QDir::separator()
            + info.fileName() + "_Products";
    if (!QFile::exists(dirName)) {
        if (!QDir().mkpath(dirName)) {
            return QString();
        }
    }
    return dirName;
}
