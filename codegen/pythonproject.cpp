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


#include <QDebug>
#include <QDir>
#include <QCoreApplication>

#include "pythonproject.h"
#include "stridesystem.hpp"
#include "codevalidator.h"


PythonProject::PythonProject(QString platformName, QString platformPath, QString strideRoot,
                             QString projectDir,
                             QString pythonExecutable) :
    Builder(projectDir, strideRoot, platformPath),
    m_platformName(platformName),
    m_runningProcess(this),
    m_buildProcess(this)

{
    if(pythonExecutable.isEmpty()) {
        m_pythonExecutable = "python";
    } else {
        m_pythonExecutable = pythonExecutable;
    }

    m_jsonFilename = m_projectDir + QDir::separator() + "tree-" + platformName + ".json";

    QObject::connect(&m_buildProcess, SIGNAL(readyReadStandardOutput()) , this, SLOT(consoleMessage()));
    QObject::connect(&m_buildProcess, SIGNAL(readyReadStandardError()) , this, SLOT(consoleMessage()));
    QObject::connect(&m_runningProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(consoleMessage()));
    QObject::connect(&m_runningProcess, SIGNAL(readyReadStandardError()), this, SLOT(consoleMessage()));
}

PythonProject::~PythonProject()
{
    m_building.store(0);
    m_buildProcess.kill();
    m_buildProcess.waitForFinished();

    m_running.store(0);
    m_runningProcess.kill();
    m_runningProcess.waitForFinished();
}

std::map<string, string> PythonProject::generateCode(ASTNode tree)
{
    writeAST(tree);    // Write configuration file to json
    QJsonDocument configJson = QJsonDocument::fromVariant(m_configuration);
    QFile configFile(m_projectDir + QDir::separator() + "config.json");
    if (configFile.open(QIODevice::WriteOnly)) {
        configFile.write(configJson.toJson());
        configFile.close();
    }
    return std::map<string, string>();
}

bool PythonProject::build(std::map<string, string> domainMap)
{
    QStringList arguments;
    if (m_buildProcess.state() == QProcess::Running) {
        m_buildProcess.close();
        if (!m_buildProcess.waitForFinished(5000)) {
            qDebug() << "Could not stop build process. Not starting again.";
            return false;
        }
    }

    // Start Build
    m_stdErr.clear();
    m_stdOut.clear();
    m_buildProcess.setWorkingDirectory(m_strideRoot);
    // FIXME un hard-code library version
    arguments << "library/1.0/python/build.py" << m_jsonFilename << m_projectDir << m_strideRoot << "build";
    m_buildProcess.start(m_pythonExecutable, arguments);

    m_buildProcess.waitForStarted(15000);
//    qDebug() << "pid:" << m_buildProcess.pid();
    m_building.store(1);
    while(m_building.load() == 1) {
        if(m_buildProcess.waitForFinished(50)) {
            m_building.store(0);
        }
        qApp->processEvents();
    }

    qApp->processEvents();
    if (m_buildProcess.exitStatus() == QProcess::ExitStatus::NormalExit
            && m_buildProcess.exitCode() == 0) {
        emit outputText("Done building. Success.");
        return true;
    } else {
        emit outputText("Done building. Failed.");
        return false;
    }
}

bool PythonProject::run(bool pressed)
{
    if (!pressed) {
        stopRunning();
        return false;
    }
    QStringList arguments;
    if (m_runningProcess.state() == QProcess::Running) {
        m_runningProcess.close();
        if (!m_runningProcess.waitForFinished(5000)) {
            qDebug() << "Could not stop run process. Not starting again.";
            return false;
        }
    }
    m_stdErr.clear();
    m_stdOut.clear();
    m_runningProcess.setWorkingDirectory(m_strideRoot);
    // FIXME un hard-code library version
    arguments << "library/1.0/python/build.py" << m_jsonFilename << m_projectDir << m_strideRoot << "run";
    m_runningProcess.start(m_pythonExecutable, arguments);
    qDebug() << arguments;

    m_runningProcess.waitForStarted(15000);
    qDebug() << "run pid:" << m_runningProcess.pid();
    m_running.store(1);
    while(m_running.load() == 1) {
        if(m_runningProcess.waitForFinished(50)) {
            m_running.store(0);
        }
        qApp->processEvents();
    }
    emit programStopped();
    qApp->processEvents();
    if (m_runningProcess.exitStatus() == QProcess::ExitStatus::NormalExit
            && m_runningProcess.exitCode() == 0) {
        emit outputText("Done running.");
        return true;
    } else {
        emit outputText("Abnormal run exit.");
        return false;
    }
}

void PythonProject::stopRunning()
{
    m_building.store(0);
    m_running.store(0);
    if (m_buildProcess.state() == QProcess::Running) {
        m_buildProcess.kill(); // Taking too long...
    }
    if (m_runningProcess.state() == QProcess::Running) {
        m_runningProcess.kill(); // Taking too long...
    }
//    qDebug() << "Stopping.";
    //m_runningProcess.waitForFinished();
}

void PythonProject::writeAST(ASTNode tree)
{
    QJsonArray treeObject;
    for(ASTNode node : tree->getChildren()) {
        QJsonObject nodeObject;
        if (node->getNodeType() == AST::Platform
                || node->getNodeType() == AST::Stream
                || node->getNodeType() == AST::Declaration
                || node->getNodeType() == AST::BundleDeclaration) {
            astToJson(node, nodeObject);
        }
        treeObject.append(nodeObject);
    }
    QFile saveFile(m_jsonFilename);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    QJsonDocument saveDoc(treeObject);
    saveFile.write(saveDoc.toJson());
}

void PythonProject::astToJson(ASTNode node, QJsonObject &obj)
{
    if (node->getNodeType() == AST::Bundle) {
        QJsonObject newObj;
        newObj["type"] = QString("Bundle");
        newObj["name"] = QString::fromStdString(static_cast<BundleNode *>(node.get())->getName());
        newObj["filename"] = QString::fromStdString(node->getFilename());
        newObj["line"] = node->getLine();

        ListNode *indexList = static_cast<BundleNode *>(node.get())->index().get();
        Q_ASSERT(indexList->size() == 1);
        AST *indexNode = indexList->getChildren().at(0).get();
        if (indexNode->getNodeType() == AST::Int) {
            newObj["index"] = static_cast<ValueNode *>(indexNode)->getIntValue();
        } else if (indexNode->getNodeType() == AST::Block) {
            newObj["index"] = QString::fromStdString(static_cast<BlockNode *>(indexNode)->getName());
        } else if (indexNode->getNodeType() == AST::List) {
            // FIXME implement support for Lists
        } else if (indexNode->getNodeType() == AST::Range) {
            // FIXME implement support for Range
            // Are ranges and lists always unraveled by the compiler?
        }
        newObj["rate"] = CodeValidator::getNodeRate(node);
        obj["bundle"] = newObj;
    } else if (node->getNodeType() == AST::Block) {
        QJsonObject newObj;
        newObj["name"] = QString::fromStdString(static_cast<BundleNode *>(node.get())->getName());
        newObj["filename"] = QString::fromStdString(node->getFilename());
        newObj["line"] = node->getLine();
        obj["name"] = newObj;
    } else if (node->getNodeType() == AST::Expression) {
        QJsonObject newObj;
        expressionToJson(static_pointer_cast<ExpressionNode>(node), newObj);
        newObj["filename"] = QString::fromStdString(node->getFilename());
        newObj["line"] = node->getLine();
        obj["expression"] = newObj;
    } else if (node->getNodeType() == AST::Function) {
        QJsonObject newObj;
        functionToJson(static_pointer_cast<FunctionNode>(node), newObj);
        newObj["filename"] = QString::fromStdString(node->getFilename());
        newObj["line"] = node->getLine();
        obj["function"] = newObj;
    } else if (node->getNodeType() == AST::Stream) {
        QJsonArray array;
        streamToJsonArray(static_pointer_cast<StreamNode>(node), array);
        obj["stream"] = array;
    } else if (node->getNodeType() == AST::Int) {
        QJsonObject newObj;
        obj["value"] = static_cast<ValueNode *>(node.get())->getIntValue();
        QJsonObject domainObj;
        astToJson(static_cast<ValueNode *>(node.get())->getDomain(), domainObj);
        obj["domain"] = domainObj;
    } else if (node->getNodeType() == AST::Real) {
        obj["value"] = static_cast<ValueNode *>(node.get())->getRealValue();
        QJsonObject domainObj;
        astToJson(static_cast<ValueNode *>(node.get())->getDomain(), domainObj);
        obj["domain"] = domainObj;
    } else if (node->getNodeType() == AST::String) {
        obj["value"] = QString::fromStdString(static_cast<ValueNode *>(node.get())->getStringValue());
        QJsonObject domainObj;
        astToJson(static_cast<ValueNode *>(node.get())->getDomain(), domainObj);
        obj["domain"] = domainObj;
    } else if (node->getNodeType() == AST::Switch) {
        obj["value"] = static_cast<ValueNode *>(node.get())->getSwitchValue();
    } else if (node->getNodeType() == AST::Declaration) {
        DeclarationNode *block = static_cast<DeclarationNode *>(node.get());
        QJsonObject newObject;
        newObject["name"] = QString::fromStdString(block->getName());
        newObject["type"] = QString::fromStdString(block->getObjectType());
        vector<string> nsList = block->getNamespaceList();
        QString ns;
        for (string name: nsList) {
            ns += QString::fromStdString(name) + "::";
        }
        ns.chop(2);
        newObject["namespace"] = ns;
        vector<std::shared_ptr<PropertyNode>> props = block->getProperties();
        QJsonObject propObject;
        for(auto prop : props) {
            ASTNode propValue = prop->getValue();
            QJsonObject valueObject;
            astToJson(propValue, valueObject);
            if (!valueObject.isEmpty()) {
                propObject[QString::fromStdString(prop->getName())]
                        = valueObject;
            } else {
                propObject[QString::fromStdString(prop->getName())] = QJsonValue();
            }
            // TODO use astToJson here instead.
            if (propValue->getNodeType() == AST::Int) {
                propObject[QString::fromStdString(prop->getName())]
                        = static_cast<ValueNode *>(propValue.get())->getIntValue();
            } else if (propValue->getNodeType() == AST::Real) {
                propObject[QString::fromStdString(prop->getName())]
                        = static_cast<ValueNode *>(propValue.get())->getRealValue();
            } else if (propValue->getNodeType() == AST::String) {
                propObject[QString::fromStdString(prop->getName())]
                        = QString::fromStdString(static_cast<ValueNode *>(propValue.get())->getStringValue());
            } else if (propValue->getNodeType() == AST::Expression) {
                    // TODO complete this
            } else if (propValue->getNodeType() == AST::Block) {
                QJsonObject nameObject;
                astToJson(propValue, nameObject);
                propObject[QString::fromStdString(prop->getName())] = nameObject;
            } else if (propValue->getNodeType() == AST::List) {
                QJsonArray list;
                listToJsonArray(static_pointer_cast<ListNode>(propValue), list);
                propObject[QString::fromStdString(prop->getName())] = list;
            }
        }
        newObject["ports"] = propObject;
        newObject["filename"] = QString::fromStdString(node->getFilename());
        newObject["line"] = node->getLine();
        obj["block"] = newObject;
    } else if (node->getNodeType() == AST::BundleDeclaration) {
        DeclarationNode *block = static_cast<DeclarationNode *>(node.get());
        QJsonObject newObject;
        std::shared_ptr<BundleNode> bundle = block->getBundle();
        newObject["name"] = QString::fromStdString(bundle->getName());
        newObject["type"] = QString::fromStdString(block->getObjectType());
        vector<string> nsList = block->getNamespaceList();
        QString ns;
        for (string name: nsList) {
            ns += QString::fromStdString(name) + "::";
        }
        ns.chop(2);
        newObject["namespace"] = ns;
        ListNode *indexList = bundle->index().get();
        Q_ASSERT(indexList->size() == 1);
        AST *bundleIndex = indexList->getChildren().at(0).get();
        if (bundleIndex->getNodeType() == AST::Int || bundleIndex->getNodeType() == AST::Real) {
            newObject["size"] = static_cast<ValueNode *>(bundleIndex)->getIntValue();
        } else if (bundleIndex->getNodeType() == AST::Block) {
            // FIXME we need to set the value from the name (it must be a constant)
//            newObject["size"] = QString::fromStdString(static_cast<NameNode *>(bundleIndex)->getName());
            newObject["size"] = 8;
        } else {
            qDebug() << "Type for index not implemented.";
            // TODO Implement support for more index types
        }
        vector<std::shared_ptr<PropertyNode>> props = block->getProperties();
//        QJsonObject propertiesObj;
        QJsonObject propObject;
        for(auto prop : props) {
            ASTNode propValue = prop->getValue();
            QJsonObject valueObject;
            astToJson(propValue, valueObject);
            if (!valueObject.isEmpty()) {
                propObject[QString::fromStdString(prop->getName())]
                        = valueObject;
            } else {
                propObject[QString::fromStdString(prop->getName())] = QJsonValue();
            }
            // TODO use astToJson here instead.
            if (propValue->getNodeType() == AST::Int) {
                propObject[QString::fromStdString(prop->getName())]
                        = static_cast<ValueNode *>(propValue.get())->getIntValue();
            } else if (propValue->getNodeType() == AST::Real) {
                propObject[QString::fromStdString(prop->getName())]
                        = static_cast<ValueNode *>(propValue.get())->getRealValue();
            } else if (propValue->getNodeType() == AST::String) {
                propObject[QString::fromStdString(prop->getName())]
                        = QString::fromStdString(static_cast<ValueNode *>(propValue.get())->getStringValue());
            } else if (propValue->getNodeType() == AST::Expression) {
                    // TODO complete this
            } else if (propValue->getNodeType() == AST::Block) {
                QJsonObject nameObject;
                astToJson(propValue, nameObject);
                propObject[QString::fromStdString(prop->getName())] = nameObject;
            } else if (propValue->getNodeType() == AST::List) {
                QJsonArray list;
                listToJsonArray(static_pointer_cast<ListNode>(propValue), list);
                propObject[QString::fromStdString(prop->getName())] = list;
            }
        }
        newObject["ports"] = propObject;
        newObject["filename"] = QString::fromStdString(node->getFilename());
        newObject["line"] = node->getLine();
        obj["blockbundle"] = newObject;
    } else if (node->getNodeType() == AST::List) {
        QJsonArray list;
        listToJsonArray(static_pointer_cast<ListNode>(node), list);
        obj["list"] = list;
    } else if (node->getNodeType() == AST::None) {
        obj = QJsonObject(); // Null value
    } else if (node->getNodeType() == AST::PortProperty) {
        QJsonObject newObj;
        newObj["name"] = QString::fromStdString(static_cast<PortPropertyNode *>(node.get())->getName());
        newObj["portname"] = QString::fromStdString(static_cast<PortPropertyNode *>(node.get())->getPortName());
        obj["portproperty"] = newObj;
    } else if (node->getNodeType() == AST::Platform) {
        QJsonObject newObj;
        newObj["name"] = QString::fromStdString(static_cast<SystemNode *>(node.get())->platformName());
        newObj["majorVersion"] = static_cast<SystemNode *>(node.get())->majorVersion();
        newObj["minorVersion"] = static_cast<SystemNode *>(node.get())->minorVersion();
        QJsonObject platformInfo;
        platformInfo["path"] = m_platformPath;
        platformInfo["name"] = m_platformName;
        QJsonArray platformList;
        platformList.append(platformInfo);
        newObj["platforms"] = platformList;
        obj["system"] = newObj;
    } else {
        obj["type"] = QString("Unsupported");
    }
}

void PythonProject::listToJsonArray(std::shared_ptr<ListNode> listNode, QJsonArray &obj)
{
    foreach(ASTNode element, listNode->getChildren()) {
        QJsonObject jsonElement;
        astToJson(element, jsonElement);
        obj.append(jsonElement);
    }
}

void PythonProject::streamToJsonArray(std::shared_ptr<StreamNode> node, QJsonArray &array)
{
    Q_ASSERT(node->getNodeType() == AST::Stream);
    QJsonObject leftObject;
    astToJson(node->getLeft(), leftObject);
    array.append(leftObject);
    if (node->getRight()->getNodeType() == AST::Stream) {
        appendStreamToArray(node->getRight(), array);
    } else {
        QJsonObject rightObject;
        astToJson(node->getRight(), rightObject);
        array.append(rightObject);
    }
}

void PythonProject::functionToJson(std::shared_ptr<FunctionNode> node, QJsonObject &obj)
{
    obj["name"] = QString::fromStdString(node->getName());
    obj["type"] = QString("Function");
    vector<std::shared_ptr<PropertyNode>> properties = node->getProperties();
    QJsonObject propObject;
    for(auto property : properties) {
        QJsonObject propValue;
        astToJson(property->getValue(), propValue);
        if (!propValue.isEmpty()) {
            propObject[QString::fromStdString(property->getName())] = propValue;
        } else {
            propObject[QString::fromStdString(property->getName())] = QJsonValue();
        }
    }
    obj["ports"] = propObject;
    obj["rate"] = CodeValidator::getNodeRate(node);
    auto inputBlock = node->getCompilerProperty("inputBlock");
    if (inputBlock) {
        if (inputBlock->getNodeType() == AST::Block) {
            obj["inputBlock"] = QString::fromStdString(static_pointer_cast<BlockNode>(inputBlock)->getName());
        } else if (inputBlock->getNodeType() == AST::Bundle) {
            obj["inputBlock"] = QString::fromStdString(static_pointer_cast<BundleNode>(inputBlock)->getName());
        }
    }
    auto outputBlock = node->getCompilerProperty("outputBlock");
    if (outputBlock) {
        if (outputBlock->getNodeType() == AST::Block) {
            obj["outputBlock"] = QString::fromStdString(static_pointer_cast<BlockNode>(outputBlock)->getName());
        } else if (outputBlock->getNodeType() == AST::Bundle) {
            obj["outputBlock"] = QString::fromStdString(static_pointer_cast<BundleNode>(outputBlock)->getName());
        }
    }

}

void PythonProject::expressionToJson(std::shared_ptr<ExpressionNode> node, QJsonObject &obj)
{
    obj["type"] = QString::fromStdString(node->getExpressionTypeString());

    if (node->isUnary()) {
        QJsonObject value;
        astToJson(node->getValue(), value);
        if (!value.isEmpty()) {
            obj["value"] = value;
        }
    } else {
        QJsonObject left;
        astToJson(node->getLeft(), left);
        obj["left"] = left.isEmpty() ? QJsonValue() : left;
        QJsonObject right;
        astToJson(node->getRight(),right);
        obj["right"] = right.isEmpty() ? QJsonValue() : right;
    }
}

void PythonProject::appendStreamToArray(ASTNode node, QJsonArray &array)
{
    QJsonObject newObj;
    astToJson(node, newObj);
    if(!newObj["stream"].toArray().isEmpty()) {
        foreach(QJsonValue value, newObj["stream"].toArray()) {
            array.append(value);
        }
    }
}




bool PythonProject::isValid()
{
    return true;
}

void PythonProject::consoleMessage()
{
    QByteArray stdOut;
    QByteArray stdErr;
    if (m_running.load() == 1) {
        stdOut = m_runningProcess.readAllStandardOutput();
        stdErr = m_runningProcess.readAllStandardError();
    } else if (m_building.load() == 1) {
        stdOut = m_buildProcess.readAllStandardOutput();
        stdErr = m_buildProcess.readAllStandardError();
    } else {
        qDebug() << "WARNING: consoleMessage() called but nothing running";
    }
    if (!stdOut.isEmpty()) {
        m_stdOut.append(stdOut);
        emit outputText(stdOut);
    }
    if (!stdErr.isEmpty()) {
        m_stdErr.append(stdErr);
        emit errorText(stdErr);
    }
}
