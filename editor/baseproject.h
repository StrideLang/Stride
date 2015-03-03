#ifndef BASEPROJECT_H
#define BASEPROJECT_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "ugeninterface.h"
#include "streamplatform.h"

class BaseProject : public QObject
{
    Q_OBJECT
public:
    BaseProject(QString dir);
    virtual ~BaseProject();

//    QString getType() {return m_platform->getPlatformName();}
    QString getTarget() {return m_target;}
    QString getBoardId() {return m_target;}
    void setTarget(QString target) {m_target = target;}
    void setBoardId(QString id) {m_board_id = id;}
    void setCode(QString code) {m_code = code;}

    UgenInterface *getUgens() {return &m_ugens;}

    virtual void setProjectName(QString name) = 0;
    virtual void save() = 0;

public slots:
    virtual void build() {}
    virtual void flash() {}
    virtual void run(bool pressed) {Q_UNUSED(pressed);}
    virtual QStringList listTargets() {return QStringList();}
    virtual QStringList listDevices() {return QStringList();}

protected:
    QString m_projectDir;
    QString m_target;
    QString m_board_id;
    QString m_code;

    UgenInterface m_ugens; // TODO move UgenInterface to Platform
    StreamPlatform *m_platform;

signals:
    void outputText(QString text);
    void errorText(QString text);
    void programStopped();
};

#endif // BASEPROJECT_H
