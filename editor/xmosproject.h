#ifndef XMOSPROJECT_H
#define XMOSPROJECT_H

#include "baseproject.h"

#include <QString>
#include <QProcess>
#include <QMutex>

extern "C" {
// #include "luajit-2.0/lua.hpp"
#include "lua.hpp"
}

/* SimpleProject uses the XMOS command line tools and Makefile build system */
class XmosProject : public BaseProject
{
    Q_OBJECT
public:
    XmosProject(QString projectDir, QString platformRoot, QString xmosToolchainRoot);
    virtual ~XmosProject();

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
    QString m_xmosToolchainRoot;
//    BaseBlock *m_audioOutBlock;
//    BaseBlock *m_audioInBlock;
    QVector<QString> m_codeStrings;

    lua_State *m_lua;
};

#endif // XMOSPROJECT_H
