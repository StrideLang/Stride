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

#include <QMutexLocker>
#include <QTemporaryFile>
#include <QVector>
#include <QDebug>

#include "codemodel.hpp"

#include "codevalidator.h"
#include "declarationnode.h"
#include "valuenode.h"
#include "listnode.h"
#include "blocknode.h"

CodeModel::CodeModel(QObject *parent) :
    QObject(parent),
    m_lastValidTree(nullptr)
{

}

CodeModel::~CodeModel()
{
}

QString CodeModel::getHtmlDocumentation(QString symbol)
{
    if (!m_lastValidTree) {
        return tr("Parsing error. Can't update tree.");
    }
    QString header = R"(<head>
                     <style>
           body {
               background-color: lighgrey;
               font-family: Mono;
               font-size: small;
           }
           h1 {
               color: maroon;
               margin-left: 40px;
               border-bottom:1px solid #CCC;
               padding-bottom:3px;
           }
           h2{
               color: maroon;
               margin-left: 30px;
               border-bottom:1px solid #CCC;
               padding-bottom:2px;
           }
           table {
               width: 100%;
           }

           th, td {
               border-bottom: 1px solid #ddd;
           }
           </style></head>)";
    QList<LangError> errors;
    if (symbol[0].toLower() == symbol[0]) {
        QMutexLocker locker(&m_validTreeLock);
        std::shared_ptr<DeclarationNode> typeBlock = CodeValidator::findTypeDeclarationByName(symbol, QVector<ASTNode>(), m_lastValidTree, errors);
        if (typeBlock) {
            AST *metaValue = typeBlock->getPropertyValue("meta").get();
            if (metaValue) {
                Q_ASSERT(metaValue);
                Q_ASSERT(metaValue->getNodeType() == AST::String);
                QString docHtml = "<h1>" + symbol + "</h1>\n";
                docHtml += QString::fromStdString(static_cast<ValueNode *>(metaValue)->getStringValue());
                vector<std::shared_ptr<PropertyNode> > properties = typeBlock->getProperties();
                QString propertiesHtml = tr("<h2>Ports</h2>") + "\n";
                QString propertiesTable = "<table><tr><td><b>Name</b></td><td><b>Types</b></td><td><b>Default</b></td><td><b>Direction</b></td></tr>";
                QVector<ASTNode> ports = CodeValidator::getPortsForTypeBlock(typeBlock, QVector<ASTNode>(), m_lastValidTree);
                for(ASTNode port : ports) {
                    DeclarationNode *portBlock = static_cast<DeclarationNode *>(port.get());
                    Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
                    if (portBlock->getNodeType() == AST::Declaration) {
                        QString portName = QString::fromStdString(
                                    static_cast<ValueNode *>(portBlock->getPropertyValue("name").get())->getStringValue());
                        if (portName != "inherits" && portName != "meta") {
                            AST *portMetaNode = portBlock->getPropertyValue("meta").get();
                            QString portMeta;
                            if (portMetaNode) {
                                portMeta = QString::fromStdString(static_cast<ValueNode *>(portMetaNode)->getStringValue());
                            }
                            propertiesHtml += "<h3>" + portName + "</h3>" + portMeta;
                            propertiesTable += "<tr><td>" + portName;
                            AST *portTypesValue = portBlock->getPropertyValue("types").get();
//                            Q_ASSERT(portTypesValue);
//                            Q_ASSERT(portTypesValue->getNodeType() == AST::List);
                            if (portTypesValue && portTypesValue->getNodeType() == AST::List) {
                                ListNode *validTypesList = static_cast<ListNode *>(portTypesValue);
                                QString typesText;
                                for(ASTNode validTypeNode : validTypesList->getChildren()) {
                                    if (validTypeNode->getNodeType() == AST::String) {
                                        string typeName = static_cast<ValueNode *>(validTypeNode.get())->getStringValue();
                                        typesText += QString::fromStdString(typeName + ", ");
                                    } else if (validTypeNode->getNodeType() == AST::Block) {
                                        string typeName = static_cast<BlockNode *>(validTypeNode.get())->getName();
                                        typesText += QString::fromStdString(typeName + ", ");
                                    } else {
                                        typesText += "---";
                                    }
                                }
                                typesText.chop(2);
                                propertiesTable += "<td>" + typesText + "</td>";
                            }
                            AST *defaultValue = portBlock->getPropertyValue("default").get();
                            if (defaultValue) {
                                if (defaultValue->getNodeType() == AST::None) {
                                    propertiesTable += "<td>None</td>";
                                } else if (defaultValue->getNodeType() == AST::String) {
                                    propertiesTable += "<td>" + QString::fromStdString(static_cast<ValueNode *>(defaultValue)->getStringValue()) + "</td>";
                                } else if (defaultValue->getNodeType() == AST::Int) {
                                    propertiesTable += "<td>" +  QString("%1").arg(static_cast<ValueNode *>(defaultValue)->getIntValue()) + "</td>";
                                } else if (defaultValue->getNodeType() == AST::Real) {
                                    propertiesTable += "<td>" +  QString("%1").arg(static_cast<ValueNode *>(defaultValue)->getRealValue()) + "</td>";
                                } else {
                                    propertiesTable += "<td>---</td>";
                                }
                            }
                            propertiesTable += "</tr>";
                        }
                    }
                }
                propertiesTable += "</table>";
                QString finalHtml = "<html>" + header + "<body>" +  docHtml + propertiesHtml + propertiesTable;
                finalHtml += "</body></html>";
                return finalHtml;
            }
        }
    } else if (symbol[0].toUpper() == symbol[0]) { // Check if it is a declared module
        QMutexLocker locker(&m_validTreeLock);
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(symbol, QVector<ASTNode>(), m_lastValidTree);
        if (declaration) {
            AST *metaValue = declaration->getPropertyValue("meta").get();
            Q_ASSERT(metaValue);
            if (metaValue) {
                Q_ASSERT(metaValue->getNodeType() == AST::String);
                QString docHtml = "<h1>" + symbol + "</h1>\n";
                docHtml += QString::fromStdString(static_cast<ValueNode *>(metaValue)->getStringValue());
                QString propertiesTable = "<table> <tr><td><b>Name</b></td><td><b>Main</b></td><td><b>Default</b></td><td><b>Direction</b></td></tr>";
                QString propertiesHtml = tr("<h2>Ports</h2>") + "\n";
                AST *properties = declaration->getPropertyValue("ports").get();
                if (properties && properties->getNodeType() == AST::List) {
                    Q_ASSERT(properties->getNodeType() == AST::List);
                    ListNode *propertiesList = static_cast<ListNode *>(properties);
                    for(ASTNode member : propertiesList->getChildren()) {
                        DeclarationNode *portBlock = static_cast<DeclarationNode *>(member.get());
                        Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
                        if (portBlock->getNodeType() == AST::Declaration) {
                            QString portName = QString::fromStdString(
                                        static_cast<ValueNode *>(portBlock->getPropertyValue("name").get())->getStringValue());
                            if (portName != "inherits" && portName != "meta") {
                                AST *portMetaNode = portBlock->getPropertyValue("meta").get();
                                QString portMeta;
                                if (portMetaNode) {
                                    portMeta = QString::fromStdString(static_cast<ValueNode *>(portMetaNode)->getStringValue());
                                }
                                propertiesHtml += "<h3>" + portName + "</h3>" + portMeta;
                                propertiesTable += "<tr><td>" + portName + "</td>";
                                if (portBlock->getObjectType() == "mainInputPort"
                                        || portBlock->getObjectType() == "mainOutputPort") {
                                    propertiesTable += "<td>on</td>";
                                } else {
                                    propertiesTable += "<td>off</td>";
                                }
//                                AST *portTypesValue = portBlock->getPropertyValue("types");
////                                Q_ASSERT(portTypesValue);
////                                Q_ASSERT(portTypesValue->getNodeType() == AST::List);
//                                if (portTypesValue && portTypesValue->getNodeType() == AST::List) {
//                                    ListNode *validTypesList = static_cast<ListNode *>(portTypesValue);
//                                    foreach(AST *validTypeNode, validTypesList->getChildren()) {
//                                        if (validTypeNode->getNodeType() == AST::String) {
//                                            string typeName = static_cast<ValueNode *>(validTypeNode)->getStringValue();
//                                            propertiesTable += QString::fromStdString("<td>" + typeName + "</td>");
//                                        } else if (validTypeNode->getNodeType() == AST::Block) {
//                                            string typeName = static_cast<BlockNode *>(validTypeNode)->getName();
//                                            propertiesTable += QString::fromStdString("<td>" + typeName + "</td>");
//                                        } else {
//                                            propertiesTable += "<td>---</td>";
//                                        }
//                                    }
//                                }
                                AST *defaultValue = portBlock->getPropertyValue("default").get();
                                if (defaultValue) {
                                    if (defaultValue->getNodeType() == AST::None) {
                                        propertiesTable += "<td>None</td>";
                                    } else if (defaultValue->getNodeType() == AST::String) {
                                        propertiesTable += "<td>" + QString::fromStdString(static_cast<ValueNode *>(defaultValue)->getStringValue()) + "</td>";
                                    } else if (defaultValue->getNodeType() == AST::Int) {
                                        propertiesTable += "<td>" +  QString("%1").arg(static_cast<ValueNode *>(defaultValue)->getIntValue()) + "</td>";
                                    } else if (defaultValue->getNodeType() == AST::Real) {
                                        propertiesTable += "<td>" +  QString("%1").arg(static_cast<ValueNode *>(defaultValue)->getRealValue()) + "</td>";
                                    } else {
                                        propertiesTable += "<td>---</td>";
                                    }
                                }
                                AST *direction = portBlock->getPropertyValue("direction").get();
                                if (direction && direction->getNodeType() == AST::String) {
                                    propertiesTable += "<td>" + QString::fromStdString(static_cast<ValueNode *>(direction)->getStringValue()) + "</td>";
                                } else {
                                    propertiesTable += "<td>---</td>";
                                }
                                propertiesTable += "</tr>";
                            }
                        }
                    }
                    propertiesTable += "</table>";

                    QString finalHtml = "<html>" + header + "<body>" +  docHtml + propertiesHtml + propertiesTable;
                    finalHtml += "</body></html>";
//                    qDebug() << finalHtml;
                    return finalHtml;
                }
            }

        }
    }
    return QString();
}

QString CodeModel::getTooltipText(QString symbol)
{
    QString text;
    if (symbol[0].toUpper() == symbol[0]) { // Check if it is a declared module
        QMutexLocker locker(&m_validTreeLock);
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(symbol, QVector<ASTNode>(), m_lastValidTree);
        if (declaration) {
            AST *metaValue = declaration->getPropertyValue("meta").get();
            if (metaValue) {
                Q_ASSERT(metaValue->getNodeType() == AST::String);
                AST *properties = declaration->getPropertyValue("ports").get();
                if (properties && properties->getNodeType() == AST::List) {
                    text += "<b>" + symbol + "</b>\n(";
                    Q_ASSERT(properties->getNodeType() == AST::List);
                    ListNode *propertiesList = static_cast<ListNode *>(properties);
                    for(ASTNode member : propertiesList->getChildren()) {
                        DeclarationNode *portBlock = static_cast<DeclarationNode *>(member.get());
                        Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
                        if (portBlock->getNodeType() == AST::Declaration) {
                            if (portBlock->getPropertyValue("name")) {
                                QString portName = QString::fromStdString(
                                            static_cast<ValueNode *>(portBlock->getPropertyValue("name").get())->getStringValue());
                                if (portName != "inherits" && portName != "meta") {
                                    AST *portMetaNode = portBlock->getPropertyValue("meta").get();
                                    QString portMeta;
                                    if (portMetaNode) {
                                        portMeta = QString::fromStdString(static_cast<ValueNode *>(portMetaNode)->getStringValue());
                                    }
                                    text += "<i>" + portName + "</i>:" + portMeta + "\n";
                                }
                            }
                        }
                    }
                    text += ")";
                }
            }
        }
    } else { // word starts with lower case letter
        QList<LangError> errors;
        std::shared_ptr<DeclarationNode> typeBlock = CodeValidator::findTypeDeclarationByName(symbol, QVector<ASTNode>(), m_lastValidTree, errors);
        if (typeBlock) {
            text = "type: " + symbol;
        }
    }
    return text;
}

QPair<QString, int> CodeModel::getSymbolLocation(QString symbol)
{
    QPair<QString, int> location;
    if (!m_lastValidTree) {
        return location;
    }

    QMutexLocker locker(&m_validTreeLock);
    for(ASTNode node : m_lastValidTree->getChildren()) {
        if (node->getNodeType() == AST::Declaration ||
                node->getNodeType() == AST::BundleDeclaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(node.get());
            if (block->getName() == symbol.toStdString()) {
                QString fileName = QString::fromStdString(block->getFilename());
                location.first = fileName;
                location.second = block->getLine();
                return location;
            }
            if (block->getObjectType() == "type") {
                AST *namePropertyValue = block->getPropertyValue("typeName").get();
                Q_ASSERT(namePropertyValue);
                if (namePropertyValue->getNodeType() == AST::String) {
                    ValueNode *nameValue = static_cast<ValueNode *>(namePropertyValue);
                    if (nameValue->getStringValue() == symbol.toStdString()) {
                        QString fileName = QString::fromStdString(block->getFilename());
                        location.first = fileName;
                        location.second = block->getLine();
                        return location;
                    }
                }
            }
        }
    }
    return location;
}

AST * CodeModel::getOptimizedTree()
{
    QMutexLocker locker(&m_validTreeLock);
    AST * optimizedTree = nullptr;
    if (m_lastValidTree) {
        optimizedTree = new AST;
        for(ASTNode node : m_lastValidTree->getChildren()) {
            optimizedTree->addChild(node->deepCopy());
        }
    }
    return optimizedTree;
}

//Builder *CodeModel::createBuilder(QString projectDir)
//{
//    QMutexLocker locker(&m_validTreeLock);
//    return m_platform->createBuilder(projectDir);
//}

QStringList CodeModel::getTypes()
{
    QMutexLocker locker(&m_validTreeLock);
    return m_types;
}

QStringList CodeModel::getFunctions()
{
    QMutexLocker locker(&m_validTreeLock);
    return m_funcs;
}

QStringList CodeModel::getObjectNames()
{
    QMutexLocker locker(&m_validTreeLock);
    return m_objectNames;
}

QString CodeModel::getFunctionSyntax(QString symbol)
{
    if (symbol.isEmpty()) {
        return "";
    }
    QString text;
    if (symbol[0].toUpper() == symbol[0]) { // Check if it is a declared module
        QMutexLocker locker(&m_validTreeLock);
        std::shared_ptr<DeclarationNode> declaration = CodeValidator::findDeclaration(symbol, QVector<ASTNode>(), m_lastValidTree);
        if (declaration) {
            AST *metaValue = declaration->getPropertyValue("meta").get();
            Q_ASSERT(metaValue);
            if (metaValue) {
                Q_ASSERT(metaValue->getNodeType() == AST::String);
                AST *properties = declaration->getPropertyValue("ports").get();
                if (properties && properties->getNodeType() == AST::List) {
                    text += symbol +  "(";
                    Q_ASSERT(properties->getNodeType() == AST::List);
                    ListNode *propertiesList = static_cast<ListNode *>(properties);
                    for(ASTNode member : propertiesList->getChildren()) {
                        DeclarationNode *portBlock = static_cast<DeclarationNode *>(member.get());
                        Q_ASSERT(portBlock->getNodeType() == AST::Declaration);
                        if (portBlock->getNodeType() == AST::Declaration) {
                            QString portName = QString::fromStdString(
                                        static_cast<ValueNode *>(portBlock->getPropertyValue("name").get())->getStringValue());
                            if (portName != "inherits" && portName != "meta") {
//                                AST *portMetaNode = portBlock->getPropertyValue("meta");
//                                QString portMeta;
//                                if (portMetaNode) {
//                                    portMeta = QString::fromStdString(static_cast<ValueNode *>(portMetaNode)->getStringValue());
//                                }
                                QString defaultValue;
                                AST *portDefaultNode = portBlock->getPropertyValue("default").get();
                                if (portDefaultNode) {
                                    ValueNode * valueNode = static_cast<ValueNode *>(portDefaultNode);
                                    defaultValue = QString::fromStdString(valueNode->toString());
                                }
                                text += portName + ":" + defaultValue + " ";
                            }
                        }
                    }
                    text += ")";
                }
            }
        }
    }
    return text;
}

QList<LangError> CodeModel::getErrors()
{
    QMutexLocker locker(&m_validTreeLock);
    return m_errors;
}

void CodeModel::updateCodeAnalysis(QString code, QString platformRootPath)
{
    QMutexLocker locker(&m_validTreeLock);
    QTemporaryFile tmpFile;
    if (tmpFile.open()) {
        tmpFile.write(code.toLocal8Bit());
        tmpFile.close();
        ASTNode tree;
        tree = AST::parseFile(tmpFile.fileName().toLocal8Bit().constData());

        if (tree) {
            CodeValidator validator(platformRootPath, tree);
            m_system = validator.getSystem();
            vector<ASTNode> objects;
            if (m_system) {
                m_types = m_system->getPlatformTypeNames();
                m_funcs = m_system->getFunctionNames();
                objects = m_system->getBuiltinObjectsReference()[""];
            }
            m_objectNames.clear();
            for(ASTNode platObject : objects) {
                if (platObject->getNodeType() == AST::Block) {
                    m_objectNames << QString::fromStdString(static_cast<BlockNode *>(platObject.get())->getName());
                }
            }
            m_errors = validator.getErrors();

            if(m_lastValidTree) {
            }
            m_lastValidTree = tree;
        } else { // !tree
            vector<LangError> syntaxErrors = AST::getParseErrors();
            m_errors.clear();
            for (unsigned int i = 0; i < syntaxErrors.size(); i++) {
                m_errors << syntaxErrors[i];
            }
        }
    }
}
