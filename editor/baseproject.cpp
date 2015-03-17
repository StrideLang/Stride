#include <QDir>

#include "baseproject.h"

//extern int lua_build(const char* template_path, const char* destination_path,
//                     const char* ugen_graph, const char* configuration);

BaseProject::BaseProject(QString projectDir) :
    QObject(), m_projectDir(projectDir)
{
    m_runProcess = new QProcess(this);
    m_lua = lua_open();

    QDir::setCurrent("/home/andres/Documents/src/XMOS/Odo/Streamstack/platforms/Gamma/1.0/scripts");
    m_lua = lua_open();
    luaL_openlibs(m_lua); // Load standard libraries
}

BaseProject::~BaseProject()
{
    delete m_runProcess;

    lua_close(m_lua);
}

void BaseProject::build()
{
    QMutexLocker mutexLocker(&m_codeMutex);

    char const * fileName = m_code.toLocal8Bit().constData();
//    lua_build("","","","");
//    parse(fileName);
//    generateCode();
    QProcess p(this);
//    QStringList env = getBuildEnvironment();

//    p.setEnvironment(env);
//    p.setWorkingDirectory(m_projectDir);
//    // FIXME always clean before building
////    p.execute(m_platform->getToolchainPath() + "/bin/xmake",  QStringList() << "clean");
//    p.start(m_platform->getToolchainPath() + "/bin/xmake",  QStringList() << "all");
//    while (p.state() == QProcess::Running) {
//        p.waitForReadyRead(100);
//        QString out = QString(p.readAllStandardOutput());
//        QString error = QString(p.readAllStandardError());
//        if (!out.isEmpty()) {
//            emit outputText(out);
//        }
//        if (!error.isEmpty()) {
//            emit errorText(error);
//        }
//        qApp->processEvents();
//    }
//    QString out = QString(p.readAllStandardOutput());
//    QString error = QString(p.readAllStandardError());
//    if (!out.isEmpty()) {
//        emit outputText(out);
//    }
//    if (!error.isEmpty()) {
//        emit errorText(error);
//    }

}
