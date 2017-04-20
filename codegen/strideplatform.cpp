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

#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

#include "strideplatform.hpp"
#include "builder.h"
#include "pythonproject.h"


StridePlatform::StridePlatform(QStringList platformPaths, QString platform, QString version, QMap<QString, QString> importList) :
    m_platformName(platform), m_version(version), m_api(NullPlatform)
{
    QString objectsJson;
    QString typesJson;
    QString functionsJson;
//    QString platformPath = m_platformRootPath + QDir::separator() + m_platformName
//            + QDir::separator() + m_version + QDir::separator() + "plugins";
    platformPaths << "/home/andres/Documents/src/Stride/StreamStack/platforms"; // Add this default for my own convenience :)
    foreach(QString path, platformPaths) {
        if (m_api != NullPlatform) {
            break; // Stop looking if platform has been found.
        }
        // FIXME move loading somewhere else where it's done less often (Or don't create a new StreamPlatform every time you parse)
        QString fullPath = QDir(path + QDir::separator() + m_platformName
                                + QDir::separator() + m_version + QDir::separator() + "platformlib").absolutePath();
        if (m_api != NullPlatform) {
            break; // Stop looking if platform has been found.
        }
        m_library.setLibraryPath(path, importList);
        // Now try to find Python platform
        if (QFile::exists(fullPath)) {
            QStringList nameFilters;
            nameFilters << "*.stride";
            QStringList subPaths;
            subPaths << "";
            QMapIterator<QString, QString> it(importList);
            while (it.hasNext()) {
                it.next();
                subPaths << it.key();
            }
            foreach(QString subPath, subPaths) {
                QStringList libraryFiles =  QDir(fullPath + "/" + subPath).entryList(nameFilters);
                foreach (QString file, libraryFiles) {
                    QString fileName = fullPath + QDir::separator() + file;
                    AST *tree = AST::parseFile(fileName.toLocal8Bit().data());
                    if(tree) {
                        Q_ASSERT(!m_platform.contains(file));
                        m_platform[file] = tree;
                    } else {
                        vector<LangError> errors = AST::getParseErrors();
                        foreach(LangError error, errors) {
                            qDebug() << QString::fromStdString(error.getErrorText());
                        }
                        continue;
                    }
                    QString namespaceName = importList[subPath];
                    if (!namespaceName.isEmpty()) {
                        foreach(AST *node, tree->getChildren()) {
                            // Do we need to set namespace recursively or would this do?
//                            node->setNamespace(namespaceName.toStdString());
                        }
                    }
                }
            }
            m_platformPath = fullPath;
            m_api = PythonTools;
            m_types = getPlatformTypeNames();
            break;
        }
    }

    if (m_api == NullPlatform) {
        qDebug() << "Platform not found!";
    }
}

StridePlatform::~StridePlatform()
{
    foreach(AST *tree, m_platform) {
        tree->deleteChildren();
        delete tree;
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

QString StridePlatform::getPlatformPath()
{
    return m_platformPath;
}

QStringList StridePlatform::getErrors()
{
    return m_errors;
}

QStringList StridePlatform::getWarnings()
{
    return m_warnings;
}

QStringList StridePlatform::getPlatformTypeNames()
{
    QStringList typeNames;
    foreach(AST* group, m_platform) {
        foreach(AST *node, group->getChildren()) {
            if (node->getNodeType() == AST::Declaration) {
                DeclarationNode *block = static_cast<DeclarationNode *>(node);
                if (block->getObjectType() == "platformType") {
                    ValueNode *name = static_cast<ValueNode *>(block->getPropertyValue("typeName"));
                    if (name) {
                        Q_ASSERT(name->getNodeType() == AST::String);
                        typeNames << QString::fromStdString(name->getStringValue());
                    } else {
                        qDebug() << "Error. platform Type missing typeName port.";
                    }
                }
            }
        }
    }
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

QStringList StridePlatform::getFunctionNames()
{
    QStringList typeNames;
    foreach(AST* node, getBuiltinObjectsReference()) {
        if (node->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(node);
            if (block->getObjectType() == "module") {
                typeNames << QString::fromStdString(block->getName());
            }
        }
    }
    return typeNames;
}

Builder *StridePlatform::createBuilder(QString projectDir)
{
    Builder *builder = NULL;
    if (m_api == StridePlatform::PythonTools) {
        QString pythonExec = "python";
        builder = new PythonProject(m_platformPath, projectDir, pythonExec);
    } else if(m_api == StridePlatform::PluginPlatform) {
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
    }
    if (builder && !builder->isValid()) {
        delete builder;
        builder = NULL;
    }
    return builder;
}

QString StridePlatform::getPlatformDomain()
{
    QList<AST *> libObjects = getBuiltinObjectsReference();
    foreach(AST *object, libObjects) {
        if (object->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(object);
            if (block->getObjectType() == "constant"
                    && block->getName() == "PlatformDomain") {
                ValueNode *domainValue = static_cast<ValueNode *>(block->getPropertyValue("value"));
                Q_ASSERT(domainValue->getNodeType() == AST::String);
                return QString::fromStdString(domainValue->getStringValue());
            }
        }
    }
    return "";
}

QList<AST *> StridePlatform::getBuiltinObjectsCopy()
{
    QList<AST *> objects;
    QList<AST *> libObjects = getBuiltinObjectsReference();
    foreach(AST *object, libObjects) {
        objects << object->deepCopy();
    }
    return objects;
}

QList<AST *> StridePlatform::getBuiltinObjectsReference()
{
    QList<AST *> objects;
    QMapIterator<QString, AST *> blockGroup(m_platform);
    while (blockGroup.hasNext()) {
        blockGroup.next();
        foreach(AST *element, blockGroup.value()->getChildren()) {
            if (element->getNodeType() == AST::Declaration) {
                objects << element;
            } else {
                objects << element; // TODO: This inserts everything. Only insert what is needed
            }
        }
    }
    vector<AST *> libObjects = m_library.getLibraryMembers();
    foreach(AST *object, libObjects) {
        objects << object;
    }
    return objects;
}

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


