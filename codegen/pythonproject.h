#ifndef PYTHONPROJECT_H
#define PYTHONPROJECT_H

#include <QObject>
#include <QString>
#include <QAtomicInteger>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "streamplatform.h"
#include "ast.h"
#include "platformnode.h"
#include "bundlenode.h"
#include "blocknode.h"
#include "streamnode.h"
#include "valuenode.h"
#include "functionnode.h"

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
    void run();
    void stopRunning();

private:
    void writeAST();
    void astToJson(AST *node, QJsonObject &obj);
    void streamToJsonArray(StreamNode *node);
    void functionToJson(FunctionNode *node, QJsonObject &obj);
    void addNodeToStreamArray(AST *node);

    AST * m_tree;
    QJsonArray m_curStreamArray;
    StreamPlatform m_platform;
    QString m_projectDir;
    QString m_pythonExecutable;
    QAtomicInteger<short> m_running;
    QProcess m_runningProcess;
};

#endif // PYTHONPROJECT_H
