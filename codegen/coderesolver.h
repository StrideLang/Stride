#ifndef CODERESOLVER_H
#define CODERESOLVER_H


#include <QVector>
#include <QSharedPointer>

#include "strideplatform.hpp"
#include "stridelibrary.hpp"

#include "ast.h"
#include "streamnode.h"
#include "bundlenode.h"
#include "expressionnode.h"
#include "propertynode.h"
#include "declarationnode.h"
#include "blocknode.h"
#include "rangenode.h"
#include "valuenode.h"

class CodeResolver
{
public:
    CodeResolver(StridePlatform *platform, AST *tree);
    ~CodeResolver();

    void preProcess();

private:
    // Main processing functions
    void insertBuiltinObjects();
    void fillDefaultProperties();
    void declareModuleInternalBlocks();
    void resolveStreamSymbols();
    void expandParallel();
    void resolveConstants();
    void resolveRates();
    void processDomains();
    void analyzeConnections();
    void processStreamRateBlock();

    // Sub functions
    void resolveStreamRates(StreamNode *stream);
    void expandParallelStream(StreamNode *stream, QVector<AST *> scopeStack, AST *tree);

    void expandStreamToSizes(StreamNode *stream, QVector<int> &size, QVector<AST *> scopeStack);
    AST *expandFunctionFromProperties(FunctionNode *func, QVector<AST *> scope, AST *tree);
    void fillDefaultPropertiesForNode(AST *node);

    void insertBuiltinObjectsForNode(AST *node, QList<AST *> &objects);

    void resolveDomainsForStream(const StreamNode *func, QVector<AST *> scopeStack, QString contextDomain = "");
    string processDomainsForNode(AST *node, QVector<AST *> scopeStack, QList<AST *> &domainStack);
    void setDomainForStack(QList<AST *> domainStack, string domainName,  QVector<AST *> scopeStack);
    DeclarationNode *createDomainDeclaration(QString name);
    DeclarationNode *createSignalDeclaration(QString name, int size = 1);
    std::vector<AST *> declareUnknownName(BlockNode *name, int size, QVector<AST *> localScope, AST *tree);
    std::vector<AST *> declareUnknownBundle(BundleNode *name, int size, QVector<AST *> localScope, AST *tree);
    DeclarationNode *createConstantDeclaration(string name, AST *value);
    void declareIfMissing(string name, AST *blockList, AST *value);
    DeclarationNode *createSignalBridge(string name, AST *defaultValue, AST *inDomain, AST * outDomain, const string filename, int line, int size = 1);

    std::vector<AST *> declareUnknownExpressionSymbols(ExpressionNode *expr, int size, QVector<AST *> scopeStack, AST * tree);
    std::vector<AST *> declareUnknownFunctionSymbols(FunctionNode *func, QVector<AST *> scopeStack, AST * tree);
    ListNode *expandNameToList(BlockNode *name, int size);
    void expandNamesToBundles(StreamNode *stream, AST *tree);
    std::vector<AST *> declareUnknownStreamSymbols(const StreamNode *stream, AST *previousStreamMember, QVector<AST *> localScope, AST *tree);
    std::vector<const AST *> getModuleStreams(DeclarationNode *module);

//    QVector<AST *>  expandStream(StreamNode *stream);
//    void expandStreamMembers();
//    void sliceStreams();

//    void reduceExpressions();
    ValueNode *reduceConstExpression(ExpressionNode *expr, QVector<AST *> scope, AST *tree);
    ValueNode *resolveConstant(AST *value, QVector<AST *> scope);
    void resolveConstantsInNode(AST *node, QVector<AST *> scope);
    void resolveDomainForStreamNode(AST *node, QVector<AST *> scope);

    void checkStreamConnections(const StreamNode *stream, QVector<AST *> scopeStack, bool start = true);
    void markConnectionForNode(AST *node, QVector<AST *> scopeStack, bool start);

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
    AST * makeConnector(AST * node, string connectorName, int size, const QVector<AST *> &scopeStack);
    QVector<AST *> sliceStreamByDomain(StreamNode *stream, QVector<AST *> scopeStack);
//    StreamNode *splitStream(StreamNode *stream, AST *closingNode, AST *endNode);
    QVector<AST *> processExpression(ExpressionNode *expr, QVector<AST *> scopeStack, AST *outDOmain);

    // TODO move these four functions to CodeValidator with the rest of querying functions
    double findRateInProperties(vector<PropertyNode *> properties, QVector<AST *> scope, AST *tree);
    double getNodeRate(AST *node, QVector<AST *> scope, AST *tree);
    double getDefaultForTypeAsDouble(QString type, QString port);
    AST *getDefaultPortValueForType(QString type, QString portName);


    StridePlatform *m_platform;
    AST *m_tree;
    int m_connectorCounter;
};

#endif // CODERESOLVER_H
