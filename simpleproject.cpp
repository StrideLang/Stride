#include <QFile>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QStringList>
#include <QMessageBox>

#include "simpleproject.h"

#include "blocks/outputblock.h"
#include "blocks/oscblock.h"

SimpleProject::SimpleProject(QString projectDir):
    BaseProject(projectDir)
{
    m_projectType = "SIMPLE";

    m_templateBaseDir = "/home/andres/Documents/src/XMOS/Odo/OdoEdit/templates";

    m_luaScriptsDir = "/home/andres/Documents/src/XMOS/Odo/OdoEdit/lua_scripts/";
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

    m_runProcess = new QProcess(this);

    connect(m_runProcess, SIGNAL(stateChanged(QProcess::ProcessState)),
            this, SLOT(runStateChanged(QProcess::ProcessState)));


    //FIXME: Remove these test cases
    m_audioOutBlock = new OutputBlock("out", this);
    OscBlock *oscBlock = new OscBlock("osc1", this);
    m_audioOutBlock->connectToInput(0, oscBlock, 0);



    m_lua = lua_open();
}

SimpleProject::~SimpleProject()
{
    lua_close(m_lua);
    delete m_runProcess;

}

void SimpleProject::setPath(QString newProjectPath)
{
    Q_ASSERT(!m_projectDir.isEmpty());

}

void SimpleProject::setProjectName(QString name)
{

}

void SimpleProject::save()
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

void SimpleProject::runStateChanged(QProcess::ProcessState newState)
{
    Q_ASSERT(newState == QProcess::NotRunning);
    emit programStopped();
}

void SimpleProject::build()
{
    generateCode();
    QProcess p(this);
    QStringList env = getBuildEnvironment();

    p.setEnvironment(env);
    p.setWorkingDirectory(m_projectDir);
    // FIXME always clean before building
//    p.execute(m_toolPath + "/bin/xmake",  QStringList() << "clean");
    p.start(m_toolPath + "/bin/xmake",  QStringList() << "all");
    // FIXME this loops endlessly if build fails
    while (p.waitForReadyRead(1000) || !p.waitForFinished(1)) {
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
    flags << QString("--adapter-id") << m_board_id
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

void SimpleProject::run(bool pressed)
{
    if (pressed) {
        flash();
        QStringList env = getBuildEnvironment();

        m_runProcess->setEnvironment(env);
        m_runProcess->setWorkingDirectory(m_projectDir);

        QStringList flags;
        flags << QString("--adapter-id") << m_board_id
              << QString("--io")
              << m_projectDir + "/bin/Release/"
                 + m_projectDir.mid(m_projectDir.lastIndexOf("/") + 1) + ".xe";

        m_runProcess->execute(m_toolPath + "/bin/xrun",  flags);
        m_runProcess->waitForStarted();
        if (m_runProcess->state() != QProcess::NotRunning) {
            qDebug() << "Couldn't start xrun";
        }
    } else {
        if (m_runProcess->state() != QProcess::NotRunning) {
            m_runProcess->kill();
        }
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

void SimpleProject::generateCode()
{
    qDebug() << "SimpleProject::generateCode()";
    QDir::setCurrent(m_luaScriptsDir);
    if (luaL_loadfile(m_lua, QString(m_luaScriptsDir
                                      + "build_project.lua").toLocal8Bit()) != 0) {
        qDebug() << "Error in luaL_loadfile: " <<  m_luaScriptsDir
                    + "build_project.lua";
    }
    luaL_openlibs(m_lua); // Load standard libraries

    if (lua_pcall(m_lua,0, LUA_MULTRET, 0)) {
      qDebug() << "Something went wrong during execution";
      qDebug() << lua_tostring(m_lua, -1);
      lua_pop(m_lua,1);
    }

    lua_getglobal(m_lua, "process");
    if(!lua_isfunction(m_lua,-1)) {
        lua_pop(m_lua,1);
        qDebug() << "Error in lua script.";
        return;
    }
    lua_pushstring(m_lua, m_projectDir.toLocal8Bit().constData());   /* push 1st argument */

    if (lua_pcall(m_lua, 1, 1, 0) != 0) {
        printf("error running function `f': %s\n",lua_tostring(m_lua, -1));
        return;
    }

//    /* retrieve result */
//    if (!lua_isnumber(m_lua, -1)) {
//        printf("function `f' must return a number\n");
//        return;
//    }
//    int z = lua_tointeger(m_lua, -1);
//    printf("Result: %i\n",z);
    lua_pop(m_lua, 1);
    qDebug() << "Done!";
//    updateCodeStrings();
//    setCodeSection("Basic Config", m_codeStrings[0]);
//    setCodeSection("Control Globals", m_codeStrings[1]);
//    setCodeSection("Shared Data", m_codeStrings[2]);
//    setCodeSection("Ugen Structs", m_codeStrings[3]);
//    // TODO : implement code processing for Control Input section
//    setCodeSection("Control Processing", m_codeStrings[4]);
//    setCodeSection("Init Ugens", m_codeStrings[6]);
//    setCodeSection("Audio Processing", m_codeStrings[7]);
//     TODO implement parameter copy code generation
//    setCodeSection("Parameter Copy", "");

}

void SimpleProject::updateCodeStrings()
{
    m_codeStrings.clear();

//    m_codeStrings[0] = getBasicConfigCode();
//    m_codeStrings[1] = getControlGlobalsCode();

//    m_codeStrings[4] = getControlProcessingCode();
////    m_codeStrings[5] = controlInput;

//    QVector<BlockConnector *> inputConnections = m_audioOutBlock->getInputConnectors();
//    foreach(BlockConnector *connection, inputConnections) {
//        QVector<QPair<BaseBlock *, int> > connected = connection->getConnections();
//        for (int i = 0; i < connected.size(); i++) {
//            QPair<BaseBlock *, int> connDetails = connected[i];
//            m_codeStrings[2] += connDetails.first->getGlobalVariablesCode();
//            m_codeStrings[3] += connDetails.first->getUgenStructCode();
//            m_codeStrings[6] += connDetails.first->getInitUgensCode();
//            m_codeStrings[7] += "S32_T asig;\n";
//            m_codeStrings[7] += connDetails.first->getAudioProcessingCode(QStringList() << "asig");
//        }
//    }
    //    m_codeStrings[7] += "out_samps[0] = asig;";
}
