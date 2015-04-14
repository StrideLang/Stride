#ifndef BASEPROJECT_H
#define BASEPROJECT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QMutex>

#ifdef USE_LUA
extern "C" {
// #include "luajit-2.0/lua.hpp"
#include "lua.hpp"
}
#endif

#include "streamplatform.h"

class BaseProject : public QObject
{
    Q_OBJECT
public:
    BaseProject(QString projectDir);
    virtual ~BaseProject();

//    QString getType() {return m_platform->getPlatformName();}
    QString getTarget() {return m_target;}
    QString getBoardId() {return m_target;}
    void setTarget(QString target) {m_target = target;}
    void setBoardId(QString id) {m_board_id = id;}
    void setCode(QString code) {m_code = code;}

    virtual void setProjectName(QString name) {}
    virtual void save() {}

public slots:
    virtual void build();
    virtual void flash() {}
    virtual void run(bool pressed) {Q_UNUSED(pressed);}
    virtual QStringList listTargets() {return QStringList();}
    virtual QStringList listDevices() {return QStringList();}

protected:
    QString m_projectDir;
    QString m_target;
    QString m_board_id;
    QString m_code;

    StreamPlatform *m_platform;

    QProcess *m_runProcess;
    QMutex m_codeMutex;

#ifdef USE_LUA
    lua_State *m_lua;
#endif

signals:
    void outputText(QString text);
    void errorText(QString text);
    void programStopped();
};

#endif // BASEPROJECT_H
