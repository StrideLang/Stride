
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

#include "pythonproject.h"
#include "strideplatform.hpp"
#include "codevalidator.h"


PythonProject::PythonProject(QString platformPath,
                             QString projectDir,
                             QString pythonExecutable) :
    Builder(projectDir, platformPath),
    m_runningProcess(this),
    m_buildProcess(this)

{
    if(pythonExecutable.isEmpty()) {
        m_pythonExecutable = "python";
    } else {
        m_pythonExecutable = pythonExecutable;
    }

    connect(&m_buildProcess, SIGNAL(readyRead()), this, SLOT(consoleMessage()));
    connect(&m_runningProcess, SIGNAL(readyRead()), this, SLOT(consoleMessage()));
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

bool PythonProject::build(AST *tree)
{
    writeAST(tree);
    QStringList arguments;
    if (m_buildProcess.state() == QProcess::Running) {
        m_buildProcess.close();
        if (!m_buildProcess.waitForFinished(5000)) {
            qDebug() << "Could not stop build process. Not starting again.";
            return false;
        }
     }
    m_buildProcess.setWorkingDirectory(m_platformPath + "/../../../");
    // FIXME un hard-code library version
    arguments << "library/1.0/python/build.py" << m_projectDir << m_platformPath + "/../";
    m_buildProcess.start(m_pythonExecutable, arguments);

    m_buildProcess.waitForStarted(15000);
    m_building.store(1);
    while(m_building.load() == 1) {
        if(m_buildProcess.waitForFinished(50)) {
            m_building.store(0);
        }
        qApp->processEvents();
    }
    if (m_buildProcess.state() == QProcess::Running) {
        m_buildProcess.kill(); // Taking too long...
    }
    emit outputText("Done building.");

    if (m_buildProcess.exitStatus() == QProcess::ExitStatus::NormalExit
            && m_buildProcess.exitCode() == 0) {
        return true;
    } else {
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
           qDebug() << "Could not stop running process. Not starting again.";
           return false;
       }
    }
    m_runningProcess.setWorkingDirectory(m_platformPath + "/../../../");
    // TODO un hard-code library version
    arguments << "library/1.0/python/run.py" << m_projectDir;
    m_runningProcess.start(m_pythonExecutable, arguments);
    m_runningProcess.waitForStarted(15000);
    m_running.store(1);
    while(m_running.load() == 1) {
        if(m_runningProcess.waitForFinished(50)) {
            m_running.store(0);
        }
        qApp->processEvents();
    }
    emit outputText("Done.");
    m_runningProcess.close();

    if (m_runningProcess.exitStatus() == QProcess::ExitStatus::NormalExit
            && m_runningProcess.exitCode() == 0) {
        return true;
    } else {
        return false;
    }
}

void PythonProject::stopRunning()
{
    m_running.store(0);
    //m_runningProcess.waitForFinished();
}

void PythonProject::writeAST(AST *tree)
{

    QJsonArray treeObject;
    foreach(AST *node, tree->getChildren()) {
        QJsonObject nodeObject;
        if (node->getNodeType() == AST::Platform) {
            nodeObject["platform"] = QString::fromStdString(static_cast<PlatformNode *>(node)->platformName());
        } else if (node->getNodeType() == AST::Stream) {
            astToJson(node, nodeObject);
        } else if (node->getNodeType() == AST::Declaration) {
            astToJson(node, nodeObject);
        } else if (node->getNodeType() == AST::BundleDeclaration) {
            astToJson(node, nodeObject);
        }
        treeObject.append(nodeObject);
    }
    QFile saveFile(m_projectDir + QDir::separator() + "tree.json");

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    QJsonDocument saveDoc(treeObject);
    saveFile.write(saveDoc.toJson());
}

void PythonProject::astToJson(AST *node, QJsonObject &obj)
{
    if (node->getNodeType() == AST::Bundle) {
        QJsonObject newObj;
        newObj["type"] = QString("Bundle");
        newObj["name"] = QString::fromStdString(static_cast<BundleNode *>(node)->getName());
        newObj["filename"] = QString::fromStdString(node->getFilename());
        newObj["line"] = node->getLine();

        ListNode *indexList = static_cast<BundleNode *>(node)->index();
        Q_ASSERT(indexList->size() == 1);
        AST *indexNode = indexList->getChildren().at(0);
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
        newObj["rate"] = node->getRate();
        obj["bundle"] = newObj;
    } else if (node->getNodeType() == AST::Block) {
        QJsonObject newObj;
        newObj["name"] = QString::fromStdString(static_cast<BundleNode *>(node)->getName());
        newObj["rate"] = node->getRate();
        newObj["filename"] = QString::fromStdString(node->getFilename());
        newObj["line"] = node->getLine();
        obj["name"] = newObj;
    } else if (node->getNodeType() == AST::Expression) {
        QJsonObject newObj;
        newObj["rate"] = node->getRate();
        expressionToJson(static_cast<ExpressionNode *>(node), newObj);
        newObj["filename"] = QString::fromStdString(node->getFilename());
        newObj["line"] = node->getLine();
        obj["expression"] = newObj;
    } else if (node->getNodeType() == AST::Function) {
        QJsonObject newObj;
        functionToJson(static_cast<FunctionNode *>(node), newObj);
        newObj["filename"] = QString::fromStdString(node->getFilename());
        newObj["line"] = node->getLine();
        obj["function"] = newObj;
    } else if (node->getNodeType() == AST::Stream) {
        QJsonArray array;
        streamToJsonArray(static_cast<StreamNode *>(node), array);
        obj["stream"] = array;
    } else if (node->getNodeType() == AST::Int) {
        QJsonObject newObj;
        obj["value"] = static_cast<ValueNode *>(node)->getIntValue();
    } else if (node->getNodeType() == AST::Real) {
        obj["value"] = static_cast<ValueNode *>(node)->getRealValue();
    } else if (node->getNodeType() == AST::String) {
        obj["value"] = QString::fromStdString(static_cast<ValueNode *>(node)->getStringValue());
    } else if (node->getNodeType() == AST::Switch) {
        obj["value"] = static_cast<ValueNode *>(node)->getSwitchValue();
    } else if (node->getNodeType() == AST::Declaration) {
        DeclarationNode *block = static_cast<DeclarationNode *>(node);
        QJsonObject newObject;
        newObject["name"] = QString::fromStdString(block->getName());
        newObject["type"] = QString::fromStdString(block->getObjectType());
        vector<PropertyNode *> props = block->getProperties();
        foreach(PropertyNode *prop, props) {
            AST *propValue = prop->getValue();
            QJsonObject valueObject;
            astToJson(propValue, valueObject);
            if (!valueObject.isEmpty()) {
                newObject[QString::fromStdString(prop->getName())]
                        = valueObject;
            } else {
                newObject[QString::fromStdString(prop->getName())] = QJsonValue();
            }
            // TODO use astToJson here instead.
            if (propValue->getNodeType() == AST::Int) {
                newObject[QString::fromStdString(prop->getName())]
                        = static_cast<ValueNode *>(propValue)->getIntValue();
            } else if (propValue->getNodeType() == AST::Real) {
                newObject[QString::fromStdString(prop->getName())]
                        = static_cast<ValueNode *>(propValue)->getRealValue();
            } else if (propValue->getNodeType() == AST::String) {
                newObject[QString::fromStdString(prop->getName())]
                        = QString::fromStdString(static_cast<ValueNode *>(propValue)->getStringValue());
            } else if (propValue->getNodeType() == AST::Expression) {
                    // TODO complete this
            } else if (propValue->getNodeType() == AST::Block) {
                QJsonObject nameObject;
                astToJson(propValue, nameObject);
                newObject[QString::fromStdString(prop->getName())] = nameObject;
            } else if (propValue->getNodeType() == AST::List) {
                QJsonArray list;
                listToJsonArray(static_cast<ListNode *>(propValue), list);
                newObject[QString::fromStdString(prop->getName())] = list;
            }
        }
        newObject["filename"] = QString::fromStdString(node->getFilename());
        newObject["line"] = node->getLine();
        obj["block"] = newObject;
    } else if (node->getNodeType() == AST::BundleDeclaration) {
        DeclarationNode *block = static_cast<DeclarationNode *>(node);
        QJsonObject newObject;
        BundleNode *bundle = block->getBundle();
        newObject["name"] = QString::fromStdString(bundle->getName());
        newObject["type"] = QString::fromStdString(block->getObjectType());
        ListNode *indexList = bundle->index();
        Q_ASSERT(indexList->size() == 1);
        AST *bundleIndex = indexList->getChildren().at(0);
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
        vector<PropertyNode *> props = block->getProperties();
//        QJsonObject propertiesObj;
        foreach(PropertyNode *prop, props) {
            AST *propValue = prop->getValue();
            QJsonObject valueObject;
            astToJson(propValue, valueObject);
            if (!valueObject.isEmpty()) {
                newObject[QString::fromStdString(prop->getName())]
                        = valueObject;
            } else {
                newObject[QString::fromStdString(prop->getName())] = QJsonValue();
            }
            // TODO use astToJson here instead.
            if (propValue->getNodeType() == AST::Int) {
                newObject[QString::fromStdString(prop->getName())]
                        = static_cast<ValueNode *>(propValue)->getIntValue();
            } else if (propValue->getNodeType() == AST::Real) {
                newObject[QString::fromStdString(prop->getName())]
                        = static_cast<ValueNode *>(propValue)->getRealValue();
            } else if (propValue->getNodeType() == AST::String) {
                newObject[QString::fromStdString(prop->getName())]
                        = QString::fromStdString(static_cast<ValueNode *>(propValue)->getStringValue());
            } else if (propValue->getNodeType() == AST::Expression) {
                    // TODO complete this
            } else if (propValue->getNodeType() == AST::Block) {
                QJsonObject nameObject;
                astToJson(propValue, nameObject);
                newObject[QString::fromStdString(prop->getName())] = nameObject;
            } else if (propValue->getNodeType() == AST::List) {
                QJsonArray list;
                listToJsonArray(static_cast<ListNode *>(propValue), list);
                newObject[QString::fromStdString(prop->getName())] = list;
            }
        }
        newObject["filename"] = QString::fromStdString(node->getFilename());
        newObject["line"] = node->getLine();
        obj["blockbundle"] = newObject;
    } else if (node->getNodeType() == AST::List) {
        QJsonArray list;
        listToJsonArray(static_cast<ListNode *>(node), list);
        obj["list"] = list;
    } else if (node->getNodeType() == AST::None) {
        obj = QJsonObject(); // Null value
    } else {
        obj["type"] = QString("Unsupported");
    }
}

void PythonProject::listToJsonArray(ListNode *listNode, QJsonArray &obj)
{
    foreach(AST * element, listNode->getChildren()) {
        QJsonObject jsonElement;
        astToJson(element, jsonElement);
        obj.append(jsonElement);
    }
}

void PythonProject::streamToJsonArray(StreamNode *node, QJsonArray &array)
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

void PythonProject::functionToJson(FunctionNode *node, QJsonObject &obj)
{
    obj["name"] = QString::fromStdString(node->getName());
    obj["type"] = QString("Function");
    vector<PropertyNode *> properties = node->getProperties();
    QJsonObject propObject;
    foreach(PropertyNode *property, properties) {
        QJsonObject propValue;
        astToJson(property->getValue(), propValue);
        if (!propValue.isEmpty()) {
            propObject[QString::fromStdString(property->getName())] = propValue;
        } else {
            propObject[QString::fromStdString(property->getName())] = QJsonValue();
        }
    }
    obj["ports"] = propObject;
    obj["rate"] = node->getRate();
}

void PythonProject::expressionToJson(ExpressionNode *node, QJsonObject &obj)
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

void PythonProject::appendStreamToArray(AST *node, QJsonArray &array)
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
        emit outputText(stdOut);
    }
    if (!stdErr.isEmpty()) {
        emit errorText(stdErr);
    }
}
