#ifndef PYTHONPROJECT_H
#define PYTHONPROJECT_H

#include <QObject>
#include <QString>
#include <QAtomicInteger>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "streamplatform.h"
#include "ast.h"
#include "platformnode.h"
#include "bundlenode.h"
#include "streamnode.h"
#include "valuenode.h"

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
    QJsonArray streamToJson(StreamNode *node, QJsonArray &array);
    void addNodeToStreamArray(AST *node, QJsonArray &array);

    AST * m_tree;
    StreamPlatform m_platform;
    QString m_projectDir;
    QString m_pythonExecutable;
    QAtomicInteger<short> m_running;
};

#endif // PYTHONPROJECT_H
