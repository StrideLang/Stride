#include <QFile>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>

#include "simpleproject.h"

SimpleProject::SimpleProject(QString projectDir):
    BaseProject(projectDir)
{
    m_projectType = "SIMPLE";

    m_templateBaseDir = "/home/andres/Documents/src/XMOS/Odo/OdoEdit/templates";

    if (!QFile::exists(projectDir)) {
        QDir().mkpath(projectDir);
        QDir templateDir(m_templateBaseDir + "/simple");
        QStringList fileList = templateDir.entryList(QDir::NoDotAndDotDot | QDir::Files);

        QStringList dirList = templateDir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs);


        foreach(QString entry, fileList) {
            QFile::copy(m_templateBaseDir + "/simple/" + entry,
                        projectDir + "/" + entry);
        }

        foreach(QString dir, dirList) { // TODO template copying is not recursive
            QDir subdir(templateDir.absolutePath() + "/" + dir);
            subdir.mkpath(projectDir + "/" + dir);
            foreach(QString entry, subdir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
                QFile::copy(m_templateBaseDir + "/simple/" + dir + "/" + entry,
                            projectDir + "/" + dir + "/" + entry);
            }
        }
    }
}

SimpleProject::~SimpleProject()
{

}

void SimpleProject::setPath(QString newProjectPath)
{
    Q_ASSERT(!m_projectDir.isEmpty());

}

void SimpleProject::build()
{
    QProcess p(this);
    QStringList env = QProcess::systemEnvironment();

    QString toolPath = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";
    QString xmosHome = QDir::homePath() +  "/.xmos";

    env << "XMOS_TOOL_PATH=" + toolPath;

    // The following comes from the XMOS SetEnv script
    env << "installpath=" + toolPath;
    env << "XMOS_HOME=" + xmosHome;
    env << "XCC_C_INCLUDE_PATH=" + toolPath + "/target/include:" + toolPath + "/target/include/gcc";
    env << "XCC_XC_INCLUDE_PATH=" + toolPath + "/target/include/xc:" + toolPath + "/target/include:" + toolPath + "/target/include/gcc";
    env << "XCC_CPLUS_INCLUDE_PATH=$XCC_C_INCLUDE_PATH:" + toolPath + "/target/include/c++/4.2.1";
    env << "XCC_CPLUS_INCLUDE_PATH=$XCC_CPLUS_INCLUDE_PATH:" + toolPath + "/target/include/c++/4.2.1/xcore-xmos-elf";
    env << "XCC_ASSEMBLER_INCLUDE_PATH=" + toolPath + "/target/include:" + toolPath + "/target/include/gcc";
    env << "XCC_LIBRARY_PATH=" + toolPath + "/target/lib";
    env << "XCC_DEVICE_PATH=" + toolPath + "/configs:" + toolPath + "/configs/.deprecated";
    env << "XCC_TARGET_PATH=" + xmosHome + "/targets:" + toolPath + "/targets:" + toolPath + "/targets/.deprecated";
    env << "XCC_EXEC_PREFIX=" + toolPath + "/libexec/";
    env << "XMOS_DOC_PATH=" + toolPath + "/doc";
    env << "PYTHON_HOME=\"" + toolPath + "/lib/jython\"";
    env << "PYTHON_VERBOSE=\"warning\"";
    env << "XMOS_CACHE_PATH=" + xmosHome + "/cache";
    env << "XMOS_REPO_PATH=" + xmosHome + "/repos";
    env << "XMOS_MAKE_PATH="+ toolPath +"/build";
    // Up to here

    env << "XMOS_MODULE_PATH=" + xmosHome + "/repos/:/home/andres/workspace";
    env << "INCLUDE_DIRS=src" ;
//    env << "XCC_FLAGS=-I"+ xmosHome + "/targets/include -I" + toolPath + "/targets/include";

    int pathIndex = env.indexOf(QRegExp("^PATH=.*"));
    env[pathIndex].append(":" + toolPath + "/bin/:" + toolPath + "/xtimecomposer_bin");

    int libpathIndex = env.indexOf(QRegExp("^LD_LIBRARY_PATH=.*"));
    env[libpathIndex].append(":" + toolPath + "/lib");

    p.setEnvironment(env);
    p.setWorkingDirectory(m_projectDir);
    p.start(toolPath + "/bin/xmake",  QStringList() /*<< "CONFIG=Default"*/ << "all");
    while (!p.waitForFinished(1000)) {
        QString out = QString(p.readAllStandardOutput());
        QString error = QString(p.readAllStandardError());
        if (!out.isEmpty()) {
            emit outputText(out);
        }
        if (!error.isEmpty()) {
            emit errorText(error);
        }
        qApp->processEvents();
    }
    QString out = QString(p.readAllStandardOutput());
    QString error = QString(p.readAllStandardError());
    if (!out.isEmpty()) {
        emit outputText(out);
    }
    if (!error.isEmpty()) {
        emit errorText(error);
    }
}
