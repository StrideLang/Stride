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
}

SimpleProject::~SimpleProject()
{
    delete m_runProcess;

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
    updateCodeStrings();
    setCodeSection("Basic Config", m_codeStrings[0]);
    setCodeSection("Control Globals", m_codeStrings[1]);
    setCodeSection("Shared Data", m_codeStrings[2]);
    setCodeSection("Ugen Structs", m_codeStrings[3]);
    // TODO : implement code processing for Control Input section
    setCodeSection("Control Processing", m_codeStrings[4]);
    setCodeSection("Init Ugens", m_codeStrings[6]);
    setCodeSection("Audio Processing", m_codeStrings[7]);
    // TODO implement parameter copy code generation
    setCodeSection("Parameter Copy", "");

}

void SimpleProject::updateCodeStrings()
{
    m_codeStrings.clear();
    m_codeStrings.resize(8);
    m_codeStrings[0] = getBasicConfigCode();
    m_codeStrings[1] = getControlGlobalsCode();

    m_codeStrings[4] = getControlProcessingCode();
//    m_codeStrings[5] = controlInput;

    QVector<BlockConnector *> inputConnections = m_audioOutBlock->getInputConnectors();
    foreach(BlockConnector *connection, inputConnections) {
        QVector<QPair<BaseBlock *, int> > connected = connection->getConnections();
        for (int i = 0; i < connected.size(); i++) {
            QPair<BaseBlock *, int> connDetails = connected[i];
            m_codeStrings[2] += connDetails.first->getGlobalVariablesCode();
            m_codeStrings[3] += connDetails.first->getUgenStructCode();
            m_codeStrings[6] += connDetails.first->getInitUgensCode();
            m_codeStrings[7] += "S32_T asig;\n";
            m_codeStrings[7] += connDetails.first->getAudioProcessingCode(QStringList() << "asig");
        }
    }
    m_codeStrings[7] += "out_samps[0] = asig;";
}

QString SimpleProject::getBasicConfigCode()
{
    QString text;
    text = "#define NUM_IN_CHANS 4\n#define NUM_OUT_CHANS 4\n\n#define MAX_PARAMS_PER_SAMPLE 2\n\n#define SAMPLE_RATE 44100\n";
    return text;
}

QString SimpleProject::getUgenStructsCode()
{

    QStringList ugenStructs;
    ugenStructs << "typedef struct {\nS32_T phs;\nS32_T att_incr;\nS32_T dec_incr;\nS32_T sus_lvl;\nS32_T rel_incr;\nunsigned char mode; //0 - attack 1 - decay 2- release\n} ENVDATA;";
    ugenStructs << "typedef struct {\nfloat gain;\n} GAINDATA;\n";

    return ugenStructs.join("\n");
}

QString SimpleProject::getControlGlobalsCode()
{
    QString code;

    code +=  "#define NUM_CTLS " "2" "\n";
    code += "typedef struct {\n float value;\n } ctl_t;\n";
    code += "ctl_t controls[NUM_CTLS]; // TODO should set to some default values\n";

    code += "interface control_in_if {\n    void setControl1(float val);\n  void setControl2(float val);\n};\n";
    code += "interface param_if {\n" \
            "[[clears_notification]] void setParam1(float val);\n" \
            "[[clears_notification]] void setParam2(float val);\n" \
            "[[notification]] slave void data_ready(void);\n" \
            "};\n";
    return code;
}

QString SimpleProject::getControlProcessingCode()
{
    QString code;

    code +=  "case c.setControl1(float val):\n"
            "controls[0].value = val;\n"
            "parameter_set.setParam1(val);\n"
            "break;\n"

            "case c.setControl2(float val):\n"
            "controls[1].value += val;\n"
            "parameter_set.setParam2(controls[1].value);\n"
            "break;\n";
    return code;
}

void SimpleProject::setCodeSection(QString section, QString code)
{
    QFile file(m_projectDir + "/src/main.xc");
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        qDebug() << "SimpleProject::setCodeSection Error! file not found: " << m_projectDir + "/Makefile/main.xc";
        return;
    }
    QString text(file.readAll());
    int startIndex = text.indexOf("//[[" + section + "]]\n");
    Q_ASSERT(startIndex >= 0);
    int endIndex = text.indexOf("//[[/" + section + "]]\n", startIndex);
    Q_ASSERT(endIndex >= startIndex);
    QString newText = "//[[" + section + "]]\n" + code;
    text.replace(startIndex, endIndex - startIndex, newText);
    file.close(); // close the file handle.

    file.open(QIODevice::WriteOnly);
    file.write(text.toUtf8()); // write the new text back to the file
    file.close(); // close the file handle.
}
