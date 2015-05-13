#ifndef BASEPROJECT_H
#define BASEPROJECT_H

#include <QObject>
#include <QString>

#include "streamplatform.h"

class BaseProject : public QObject
{
    Q_OBJECT
public:
    BaseProject(QString projectDir);
    virtual ~BaseProject();

//    QString getTarget() {return m_target;}
//    QString getBoardId() {return m_target;}
//    void setTarget(QString target) {m_target = target;}
//    void setBoardId(QString id) {m_board_id = id;}
//    void setCode(QString code) {m_code = code;}

public slots:
    virtual void build() = 0;
    virtual void flash() = 0;
    virtual void run(bool pressed) = 0;
//    virtual QStringList listTargets() {return QStringList();}
//    virtual QStringList listDevices() {return QStringList();}

protected:

signals:
    void outputText(QString text);
    void errorText(QString text);
    void programStopped();
};


#endif // BASEPROJECT_H

