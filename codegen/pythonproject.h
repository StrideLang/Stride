#ifndef PYTHONPROJECT_H
#define PYTHONPROJECT_H

#include <QObject>
#include <QString>
#include <QAtomicInt>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

#include "builder.h"

#include "ast.h"
#include "platformnode.h"
#include "bundlenode.h"
#include "declarationnode.h"
#include "streamnode.h"
#include "valuenode.h"
#include "functionnode.h"
#include "expressionnode.h"

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
    virtual bool build(AST *tree) override;
    virtual bool flash() override { return true;}
    virtual bool run(bool pressed = true) override;
    virtual bool isValid() override;

    void consoleMessage();

    void stopRunning();

private:
    void writeAST(AST *tree);
    void astToJson(AST *node, QJsonObject &obj);
    void listToJsonArray(ListNode *node, QJsonArray &obj);
    void streamToJsonArray(StreamNode *node, QJsonArray &array);
    void functionToJson(FunctionNode *node, QJsonObject &obj);
    void expressionToJson(ExpressionNode *node, QJsonObject &obj);
    void appendStreamToArray(AST *node, QJsonArray &array);

    QString m_pythonExecutable;
    QAtomicInt m_running;
    QProcess m_runningProcess;
    QAtomicInt m_building;
    QProcess m_buildProcess;
};

#endif // PYTHONPROJECT_H
