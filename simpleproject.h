#ifndef SIMPLEPROJECT_H
#define SIMPLEPROJECT_H

#include "baseproject.h"
#include "blocks/baseblock.h"

#include <QString>
#include <QProcess>

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
    QString getBasicConfigCode();
    QString getUgenStructsCode();
    QString getControlGlobalsCode();
    QString getControlProcessingCode();
    void setCodeSection(QString section, QString code);

private:
    QString m_templateBaseDir;
    QString m_toolPath;
    QProcess *m_runProcess;
    BaseBlock *m_audioOutBlock;
    BaseBlock *m_audioInBlock;
    QVector<QString> m_codeStrings;
};

#endif // SIMPLEPROJECT_H
