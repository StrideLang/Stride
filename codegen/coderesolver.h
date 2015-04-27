#ifndef CODERESOLVER_H
#define CODERESOLVER_H

#include <QVector>

#include "streamplatform.h"
#include "ast.h"
#include "streamnode.h"
#include "bundlenode.h"
#include "expressionnode.h"
#include "propertynode.h"
#include "blocknode.h"

class CodeResolver
{
public:
    CodeResolver(StreamPlatform &platform, AST *tree);
    ~CodeResolver();

    void process();

private:
    void resolveRates();
    void resolveStreamRates(StreamNode *stream);
    void fillDefaultProperties(); // This should be called when required, but not generally. Otherwise it's hard to tell which are default and which are set by the user.

    double findRateInProperties(vector<PropertyNode *> properties, QVector<AST *> scope, AST *tree);
    double getNodeRate(AST *node, QVector<AST *> scope, AST *tree);
    void insertBuiltinObjects();
    double createSignalDeclaration(QString name, StreamNode *parentStream, AST *tree);
    void declareUnknownStreamSymbols(StreamNode *stream, AST *tree);
    void resolveStreamSymbols();
    void resolveConstants();
    void expandStreamBundles();
    void expandTypeBundles();

//    void reduceExpressions();
    bool reduceConstExpression(ExpressionNode *expr, QVector<AST *> scope, AST *tree, double &expressionResult);
    void resolveConstantInProperty(PropertyNode *property, QVector<AST *> scope);
    void resolveConstantsInNode(AST *node, QVector<AST *> scope);
    double getDefaultForTypeAsDouble(QString type, QString port);

    QVector<StreamNode *> expandBundleStream(StreamNode *stream, int size = -1);

    AST *expandStreamMember(AST *node, int i);
    StreamPlatform m_platform;
    AST *m_tree;
};

#endif // CODERESOLVER_H
