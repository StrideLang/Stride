
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

#include "pythonproject.h"


PythonProject::PythonProject(QObject *parent,
                             AST *tree,
                             StreamPlatform platform,
                             QString projectDir,
                             QString pythonExecutable) :
    QObject(parent), m_tree(tree), m_platform(platform), m_projectDir(projectDir)

{
    if(pythonExecutable.isEmpty()) {
        m_pythonExecutable = "python";
    } else {
        m_pythonExecutable = pythonExecutable;
    }
}

PythonProject::~PythonProject()
{

}

void PythonProject::build()
{
    writeAST();
    QProcess pythonProcess(this);
    QStringList arguments;
    pythonProcess.setWorkingDirectory(m_platform.getPlatformPath() + QDir::separator() + "scripts");
    arguments << "build.py" << m_projectDir;
    pythonProcess.start(m_pythonExecutable, arguments);
    if(!pythonProcess.waitForFinished()) {

    }
    QByteArray stdOut = pythonProcess.readAllStandardOutput();
    QByteArray stdErr = pythonProcess.readAllStandardError();
    qDebug() << stdOut;
    qDebug() << stdErr;
}

void PythonProject::run()
{
    QProcess pythonProcess(this);
    QStringList arguments;
    pythonProcess.setWorkingDirectory(m_platform.getPlatformPath() + QDir::separator() + "scripts");
    arguments << "run.py" << m_projectDir;
    pythonProcess.start(m_pythonExecutable, arguments);
    m_running.store(1);
    while(m_running.load() == 1) {
        if(pythonProcess.waitForFinished(50)) {
            m_running.store(0);
            pythonProcess.close();
        }
        qApp->processEvents();
    }
    qDebug() << pythonProcess.readAllStandardOutput();
    qDebug() << pythonProcess.readAllStandardError();
}

void PythonProject::stopRunning()
{
    m_running.store(0);
}

void PythonProject::writeAST()
{
    QFile saveFile(m_projectDir + QDir::separator() + "tree.json");

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonArray treeObject;
    foreach(AST *node, m_tree->getChildren()) {
        QJsonObject nodeObject;
        if (node->getNodeType() == AST::Platform) {
            nodeObject["platform"] = QString::fromStdString(static_cast<PlatformNode *>(node)->platformName());
        } else if (node->getNodeType() == AST::Stream) {
            QJsonArray streamArray;
            streamToJson(static_cast<StreamNode *>(node), streamArray);
            nodeObject["stream"] = streamArray;
        }
        treeObject.append(nodeObject);
    }
    QJsonDocument saveDoc(treeObject);
    saveFile.write(saveDoc.toJson());
}

QJsonArray PythonProject::streamToJson(StreamNode *node, QJsonArray &array)
{
    AST *left = node->getLeft();
    AST *right = node->getRight();

    addNodeToStreamArray(left, array);
    addNodeToStreamArray(right, array);

    return array;
}

void PythonProject::addNodeToStreamArray(AST *node, QJsonArray &array)
{
    if (node->getNodeType() == AST::Bundle) {
        QJsonObject member;
        member["type"] = "Bundle";
        member["name"] = QString::fromStdString(static_cast<BundleNode *>(node)->getName());
        AST *indexNode = static_cast<BundleNode *>(node)->index();
        if (indexNode->getNodeType() == AST::Int) {
            member["index"] = static_cast<ValueNode *>(indexNode)->getIntValue();
        }
        array.append(member);
    } else if (node->getNodeType() == AST::Name) {
        QJsonObject member;
        member["type"] = "Name";
        member["name"] = QString::fromStdString(static_cast<BundleNode *>(node)->getName());
        array.append(member);
    }
}

