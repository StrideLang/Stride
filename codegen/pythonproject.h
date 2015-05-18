#ifndef PYTHONPROJECT_H
#define PYTHONPROJECT_H

#include <QObject>
#include <QString>
#include <QAtomicInteger>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "builder.h"

#include "streamplatform.h"
#include "ast.h"
#include "platformnode.h"
#include "bundlenode.h"
#include "blocknode.h"
#include "streamnode.h"
#include "valuenode.h"
#include "functionnode.h"

class PythonProject : public Builder
{
    Q_OBJECT
public:
    explicit PythonProject(QString platformPath,
                           QString projectDir = QString(),
                           QString pythonExecutable = QString());
    virtual ~PythonProject();

signals:

public slots:
    virtual void build(AST *tree);
    virtual void flash() {}
    virtual void run(bool pressed = true);
    virtual bool isValid();

    void stopRunning();

private:
    void writeAST(AST *tree);
    void astToJson(AST *node, QJsonObject &obj);
    void streamToJsonArray(StreamNode *node);
    void functionToJson(FunctionNode *node, QJsonObject &obj);
    void addNodeToStreamArray(AST *node);

    QJsonArray m_curStreamArray;
    QString m_pythonExecutable;
    QAtomicInteger<short> m_running;
    QProcess m_runningProcess;
};

#endif // PYTHONPROJECT_H
