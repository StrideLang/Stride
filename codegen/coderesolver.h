#ifndef CODERESOLVER_H
#define CODERESOLVER_H


#include <QVector>
#include <QSharedPointer>

#include "streamplatform.h"
#include "stridelibrary.hpp"

#include "ast.h"
#include "streamnode.h"
#include "bundlenode.h"
#include "expressionnode.h"
#include "propertynode.h"
#include "blocknode.h"
#include "namenode.h"
#include "rangenode.h"
#include "valuenode.h"

class CodeResolver
{
public:
    CodeResolver(StreamPlatform *platform, AST *tree);
    ~CodeResolver();

    void preProcess();

private:
    void resolveRates();
    void resolveStreamRates(StreamNode *stream);
    void fillDefaultProperties();
    void expandParallelFunctions();


    void expandStreamToSize(StreamNode *stream, int size);
    void fillDefaultPropertiesForNode(AST *node);

    // TODO move these four functions to CodeValidator with the rest of querying functions
    double findRateInProperties(vector<PropertyNode *> properties, QVector<AST *> scope, AST *tree);
    double getNodeRate(AST *node, QVector<AST *> scope, AST *tree);
    double getDefaultForTypeAsDouble(QString type, QString port);
    AST *getDefaultPortValueForType(QString type, QString portName);


    void insertBuiltinObjects();
    double createSignalDeclaration(QString name, int size, AST *tree);
    void declareUnknownName(NameNode *name, int size, AST *tree);
    void declareUnknownExpressionSymbols(ExpressionNode *expr, int size, AST * tree);
    ListNode *expandNameToList(NameNode *name, int size);
    void expandNamesToBundles(StreamNode *stream, AST *tree);
    void declareUnknownStreamSymbols(StreamNode *stream, AST *previousStreamMember, AST *tree);
    void resolveStreamSymbols();
    void resolveConstants();
    QVector<AST *>  expandStream(StreamNode *stream);
//    void expandStreamMembers();
//    void sliceStreams();

//    void reduceExpressions();
    ValueNode *reduceConstExpression(ExpressionNode *expr, QVector<AST *> scope, AST *tree);
    ValueNode *resolveConstant(AST *value, QVector<AST *> scope);
    void resolveConstantsInNode(AST *node, QVector<AST *> scope);

    // Operators
    ValueNode *multiply(ValueNode *left, ValueNode *right);
    ValueNode *divide(ValueNode *left, ValueNode *right);
    ValueNode *add(ValueNode *left, ValueNode *right);
    ValueNode *subtract(ValueNode *left, ValueNode *right);
    ValueNode *unaryMinus(ValueNode *value);
    ValueNode *logicalAnd(ValueNode *left, ValueNode *right);
    ValueNode *logicalOr(ValueNode *left, ValueNode *right);
    ValueNode *logicalNot(ValueNode *left);


//    QVector<AST *> expandStreamNode(StreamNode *stream);
//    AST *expandStream(AST *node, int index, int rightNumInputs, int leftNumOutputs);
//    QVector<AST *> sliceStream(StreamNode *stream);
    StreamNode *splitStream(StreamNode *stream, AST *closingNode, AST *endNode);

    StreamPlatform *m_platform;
    AST *m_tree;
    int m_connectorCounter;
};

#endif // CODERESOLVER_H
