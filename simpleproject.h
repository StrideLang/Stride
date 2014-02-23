#ifndef SIMPLEPROJECT_H
#define SIMPLEPROJECT_H

#include "baseproject.h"

#include <QString>

class SimpleProject : public BaseProject
{
    Q_OBJECT
public:
    SimpleProject(QString projectDir);
    virtual ~SimpleProject();

    void setPath(QString newProjectPath);
    void build();

private:
    QString m_templateBaseDir;
};

#endif // SIMPLEPROJECT_H
