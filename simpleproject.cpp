#include <QFile>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QApplication>
#include <QStringList>

#include "simpleproject.h"

SimpleProject::SimpleProject(QString projectDir):
    BaseProject(projectDir)
{
    m_projectType = "SIMPLE";

    m_templateBaseDir = "/home/andres/Documents/src/XMOS/Odo/OdoEdit/templates";
    m_toolPath = "/home/andres/Documents/src/XMOS/xTIMEcomposer/Community_13.0.2";

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
    m_target = getMakefileOption("TARGET");
    setMakefileOption("APP_NAME", m_projectDir.mid(m_projectDir.lastIndexOf("/") + 1));
}

SimpleProject::~SimpleProject()
{

}

void SimpleProject::setPath(QString newProjectPath)
{
    Q_ASSERT(!m_projectDir.isEmpty());

}

void SimpleProject::setProjectName(QString name)
{

}


QStringList SimpleProject::listTargets()
{
    QStringList targetList;
    QProcess p(this);
    QStringList env = getBuildEnvironment();

    p.setEnvironment(env);
    p.setWorkingDirectory(m_projectDir);
    p.start(m_toolPath + "/bin/xcc",  QStringList() << "-print-targets");

    if (!p.waitForFinished(30000)) {
        qDebug() << "SimpleProject::listTargets Timeout getting device list.";
    } else {
        targetList = QString(p.readAllStandardOutput()).split("\n");
    }
    return targetList;
}

QStringList SimpleProject::listDevices()
{
    QStringList targetList;
    QProcess p(this);
    QStringList env = getBuildEnvironment();

    p.setEnvironment(env);
    p.setWorkingDirectory(m_projectDir);
    p.start(m_toolPath + "/bin/xrun",  QStringList() << "-l");

    if (!p.waitForFinished(30000)) {
        qDebug() << "SimpleProject::listDevices Timeout getting device list.";
    }

    QString out = QString(p.readAllStandardOutput());
    emit outputText(out);

    QStringList outLines = out.split("\n");
    while (!outLines.isEmpty() && !outLines.front().startsWith("  ID")) {
        outLines.removeFirst();
    }
    if (!outLines.isEmpty()) {
        outLines.removeFirst();
        outLines.removeFirst();
        foreach(QString line, outLines) {
            if (!line.isEmpty()) {
                targetList << line.simplified();
            }
        }
    }
    return targetList;
}

void SimpleProject::build()
{
    QProcess p(this);
    QStringList env = getBuildEnvironment();

    p.setEnvironment(env);
    p.setWorkingDirectory(m_projectDir);
    // FIXME always clean before building
//    p.execute(m_toolPath + "/bin/xmake",  QStringList() << "clean");
    p.start(m_toolPath + "/bin/xmake",  QStringList() << "all");
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

void SimpleProject::flash()
{
    build();

    QProcess p(this);
    QStringList env = getBuildEnvironment();

    p.setEnvironment(env);
    p.setWorkingDirectory(m_projectDir);

    QStringList flags;
    flags << QString("--adapter-id") << QString("0ontZocni8POZ")
          << m_projectDir + "/bin/Release/"
             + m_projectDir.mid(m_projectDir.lastIndexOf("/") + 1) + ".xe";

    p.start(m_toolPath + "/bin/xflash",  flags);
    if (!p.waitForFinished(15000)) {
        qDebug() << "Flashing timeout.";
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

QStringList SimpleProject::getBuildEnvironment()
{
    QStringList env = QProcess::systemEnvironment();

    QString xmosHome = QDir::homePath() +  "/.xmos";

    env << "XMOS_TOOL_PATH=" + m_toolPath;

    // The following comes from the XMOS SetEnv script
    env << "installpath=" + m_toolPath;
    env << "XMOS_HOME=" + xmosHome;
    env << "XCC_C_INCLUDE_PATH=" + m_toolPath + "/target/include:" + m_toolPath + "/target/include/gcc";
    env << "XCC_XC_INCLUDE_PATH=" + m_toolPath + "/target/include/xc:" + m_toolPath + "/target/include:" + m_toolPath + "/target/include/gcc";
    env << "XCC_CPLUS_INCLUDE_PATH=$XCC_C_INCLUDE_PATH:" + m_toolPath + "/target/include/c++/4.2.1";
    env << "XCC_CPLUS_INCLUDE_PATH=$XCC_CPLUS_INCLUDE_PATH:" + m_toolPath + "/target/include/c++/4.2.1/xcore-xmos-elf";
    env << "XCC_ASSEMBLER_INCLUDE_PATH=" + m_toolPath + "/target/include:" + m_toolPath + "/target/include/gcc";
    env << "XCC_LIBRARY_PATH=" + m_toolPath + "/target/lib";
    env << "XCC_DEVICE_PATH=" + m_toolPath + "/configs:" + m_toolPath + "/configs/.deprecated";
    env << "XCC_TARGET_PATH=" + xmosHome + "/targets:" + m_toolPath + "/targets:" + m_toolPath + "/targets/.deprecated";
    env << "XCC_EXEC_PREFIX=" + m_toolPath + "/libexec/";
    env << "XMOS_DOC_PATH=" + m_toolPath + "/doc";
    env << "PYTHON_HOME=\"" + m_toolPath + "/lib/jython\"";
    env << "PYTHON_VERBOSE=\"warning\"";
    env << "XMOS_CACHE_PATH=" + xmosHome + "/cache";
    env << "XMOS_REPO_PATH=" + xmosHome + "/repos";
    env << "XMOS_MAKE_PATH="+ m_toolPath +"/build";
    // Up to here

    env << "XMOS_MODULE_PATH=" + xmosHome + "/repos/:/home/andres/workspace";
    env << "INCLUDE_DIRS=src" ;
//    env << "XCC_FLAGS=-I"+ xmosHome + "/targets/include -I" + toolPath + "/targets/include";

    int pathIndex = env.indexOf(QRegExp("^PATH=.*"));
    env[pathIndex].append(":" + m_toolPath + "/bin/:" + m_toolPath + "/xtimecomposer_bin");

    int libpathIndex = env.indexOf(QRegExp("^LD_LIBRARY_PATH=.*"));
    env[libpathIndex].append(":" + m_toolPath + "/lib");

    return env;
}

void SimpleProject::setMakefileOption(QString option, QString value)
{
    QFile file(m_projectDir + "/Makefile");
    file.open(QIODevice::ReadWrite);
    if (!file.isOpen()) {
        qDebug() << "SimpleProject::setMakefileOption Error! Makefile not found.";
    }
    QString text(file.readAll());
    int index = text.indexOf(QRegExp("\\n" + option + "\\s+="));
    if (index >= 0) {
        index++;
        int endindex = text.indexOf("\n", index);
        QString oldOption = text.mid(index, endindex - index);
        QString newOption = option + " = " + value;

        text.replace(oldOption, newOption); // replace text in string

        file.seek(0); // go to the beginning of the file
        file.write(text.toUtf8()); // write the new text back to the file
        file.close(); // close the file handle.
    } else {
        qDebug() << "SimpleProject::setMakefileOption Option not found!";
    }
}

QString SimpleProject::getMakefileOption(QString option)
{
    QString value;
    QFile file(m_projectDir + "/Makefile");
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        qDebug() << "SimpleProject::setMakefileOption Error! Makefile not found.";
    }
    QString text(file.readAll());
    int index = text.indexOf(QRegExp("\\n" + option + "\\s+="));
    if (index >= 0) {
        index++; //start after newline
        int endindex = text.indexOf("\n", index);
        index = text.indexOf("=", index) + 1;
        value = text.mid(index, endindex - index);
        value = value.trimmed();
    } else {
        qDebug() << "SimpleProject::getMakefileOption Option not found!";
    }
    return value;
}
