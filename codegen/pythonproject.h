#ifndef PYTHONPROJECT_H
#define PYTHONPROJECT_H

#include <QObject>
#include <QString>
#include <QAtomicInteger>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "baseproject.h"

#include "streamplatform.h"
#include "ast.h"
#include "platformnode.h"
#include "bundlenode.h"
#include "blocknode.h"
#include "streamnode.h"
#include "valuenode.h"
#include "functionnode.h"

class PythonProject : public BaseProject
{
    Q_OBJECT
public:
    explicit PythonProject(QObject *parent = 0,
                           AST *tree = NULL,
                           StreamPlatform *platform = NULL,
                           QString projectDir = QString(),
                           QString pythonExecutable = QString());
    virtual ~PythonProject();

signals:

public slots:
    virtual void build();
    virtual void flash() {}
    virtual void run(bool pressed = true);

    void stopRunning();

private:
    void writeAST();
    void astToJson(AST *node, QJsonObject &obj);
    void streamToJsonArray(StreamNode *node);
    void functionToJson(FunctionNode *node, QJsonObject &obj);
    void addNodeToStreamArray(AST *node);

    AST * m_tree;
    QJsonArray m_curStreamArray;
    QString m_pythonExecutable;
    QAtomicInteger<short> m_running;
    QProcess m_runningProcess;
};

#endif // PYTHONPROJECT_H
