#ifndef SIMPLEPROJECT_H
#define SIMPLEPROJECT_H

#include "baseproject.h"
//#include "blocks/baseblock.h"

#include <QString>
#include <QProcess>
#include <QMutex>

extern "C" {
#include "luajit-2.0/lua.hpp"
}

/* SimpleProject uses the XMOS command line tools and Makefile build system */
class SimpleProject : public BaseProject
{
    Q_OBJECT
public:
    SimpleProject(QString projectDir);
    virtual ~SimpleProject();

    void setPath(QString newProjectPath);
    virtual void setProjectName(QString name);
    virtual void save();

public slots:
    virtual void build();
    virtual void flash();
    virtual void run(bool pressed);
    virtual QStringList listTargets();
    virtual QStringList listDevices();

    void runStateChanged(QProcess::ProcessState newState);

protected:
    QStringList getBuildEnvironment();
    void setMakefileOption(QString option, QString value);
    QString getMakefileOption(QString option);
    void generateCode();
    void updateCodeStrings();
    QString getEditorCode();

private:
    QString m_templateBaseDir;
    QString m_luaScriptsDir;
    QString m_toolPath;
    QProcess *m_runProcess;
//    BaseBlock *m_audioOutBlock;
//    BaseBlock *m_audioInBlock;
    QVector<QString> m_codeStrings;
    QMutex m_codeMutex;

    lua_State *m_lua;
};

#endif // SIMPLEPROJECT_H
