#ifndef SIMPLEPROJECT_H
#define SIMPLEPROJECT_H

#include "baseproject.h"

#include <QString>

/* SimpleProject uses the XMOS command line tools and Makefile build system */
class SimpleProject : public BaseProject
{
    Q_OBJECT
public:
    SimpleProject(QString projectDir);
    virtual ~SimpleProject();

    void setPath(QString newProjectPath);
    virtual void setProjectName(QString name);

public slots:
    virtual void build();
    virtual void flash();
    virtual QStringList listTargets();
    virtual QStringList listDevices();

protected:
    QStringList getBuildEnvironment();
    void setMakefileOption(QString option, QString value);
    QString getMakefileOption(QString option);

private:
    QString m_templateBaseDir;
    QString m_toolPath;
};

#endif // SIMPLEPROJECT_H
