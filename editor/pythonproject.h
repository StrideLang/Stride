#ifndef PYTHONPROJECT_H
#define PYTHONPROJECT_H

#include <QObject>
#include <QString>

#include "ast.h"

class PythonProject : public QObject
{
    Q_OBJECT
public:
    explicit PythonProject(QObject *parent = 0,
                           AST *m_tree,
                           QString pythonExecutable);
    ~PythonProject();

signals:

public slots:
    void build();

private:
    QString m_pythonExecutable;
};

#endif // PYTHONPROJECT_H
