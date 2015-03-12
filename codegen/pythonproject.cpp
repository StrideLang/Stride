
#include <QProcess>
#include <QDebug>
#include <QDir>

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

