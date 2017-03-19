#ifndef CODEMODEL_HPP
#define CODEMODEL_HPP

#include <QObject>
#include <QMutex>

#include "ast.h"
#include "strideplatform.hpp"

class CodeModel : public QObject
{
    Q_OBJECT
public:
    explicit CodeModel(QObject *parent = 0);

    ~CodeModel();

    QString getHtmlDocumentation(QString symbol);
    QString getTooltipText(QString symbol);
    QPair<QString, int> getSymbolLocation(QString symbol);

    // Caller owns and must free the returned AST tree
    AST *getOptimizedTree();

    Builder *createBuilder(QString projectDir);

    QStringList getTypes();
    QStringList getFunctions();
    QStringList getObjectNames();
    QString getFunctionSyntax(QString symbol);
    QList<LangError> getErrors();
    void updateCodeAnalysis(QString code, QString platformRootPath);

signals:

public slots:

private:
//    QList<AST *> m_platformObjects;
    StridePlatform *m_platform;
    QStringList m_types;
    QStringList m_funcs;
    QStringList m_objectNames;
    QList<LangError> m_errors;
    QMutex m_validTreeLock;
    AST *m_lastValidTree;
};

#endif // CODEMODEL_HPP
