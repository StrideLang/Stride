#ifndef PYTHONPROJECT_H
#define PYTHONPROJECT_H

#include <QObject>
#include <QString>

#include "ast.h"
#include "streamplatform.h"

class PythonProject : public QObject
{
    Q_OBJECT
public:
    explicit PythonProject(QObject *parent = 0,
                           AST *tree = NULL,
                           StreamPlatform platform = StreamPlatform(""),
                           QString projectDir = QString(),
                           QString pythonExecutable = QString());
    ~PythonProject();

signals:

public slots:
    void build();

private:
    AST * m_tree;
    StreamPlatform m_platform;
    QString m_projectDir;
    QString m_pythonExecutable;

};

#endif // PYTHONPROJECT_H
