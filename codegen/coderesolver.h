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
#include "namenode.h"
#include "rangenode.h"

class CodeResolver
{
public:
    CodeResolver(StreamPlatform &platform, AST *tree);
    ~CodeResolver();

    void preProcess();

private:
    void resolveRates();
    void resolveStreamRates(StreamNode *stream);
    void fillDefaultProperties(); // This should be called when required, but not generally. Otherwise it's hard to tell which are default and which are set by the user.

    double findRateInProperties(vector<PropertyNode *> properties, QVector<AST *> scope, AST *tree);
    double getNodeRate(AST *node, QVector<AST *> scope, AST *tree);
    void insertBuiltinObjects();
    double createSignalDeclaration(QString name, int size, AST *tree);
    void declareUnknownStreamSymbols(StreamNode *stream, AST *previousStreamMember, AST *tree);
    void resolveStreamSymbols();
    void resolveConstants();
    void expandStreamMembers();
    void sliceStreams();

//    void reduceExpressions();
    bool reduceConstExpression(ExpressionNode *expr, QVector<AST *> scope, AST *tree, double &expressionResult);
    void resolveConstantInProperty(PropertyNode *property, QVector<AST *> scope);
    void resolveConstantsInNode(AST *node, QVector<AST *> scope);
    double getDefaultForTypeAsDouble(QString type, QString port);

    QVector<AST *> expandStreamNode(StreamNode *stream);
    AST *expandStream(AST *node, int index, int rightNumInputs = 1, int leftNumOutputs = 1);
    QVector<AST *> sliceStream(StreamNode *stream);
    StreamNode *splitStream(StreamNode *stream, AST *closingNode, AST *endNode);

    StreamPlatform m_platform;
    AST *m_tree;
    int m_connectorCounter;
};

#endif // CODERESOLVER_H
