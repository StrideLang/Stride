#ifndef CODEGEN_H
#define CODEGEN_H

#include <QString>

#include "strideparser.h"
#include "streamplatform.h"
#include "stridelibrary.hpp"

class CodeValidator
{
public:
    CodeValidator(QString platformRootDir, AST * tree);
    ~CodeValidator();

    bool isValid();
    bool platformIsValid();

    QList<LangError> getErrors();
    QStringList getPlatformErrors();

    StreamPlatform *getPlatform();

    static BlockNode *findDeclaration(QString objectName, QVector<AST *> scopeStack, AST *tree);
    static PortType resolveBundleType(BundleNode *bundle, QVector<AST *> scope, AST *tree);
    static PortType resolveNameType(NameNode *name, QVector<AST *> scope, AST *tree);
    static PortType resolveNodeOutType(AST *node, QVector<AST *> scope, AST *tree);
    static PortType resolveListType(ListNode *listnode, QVector<AST *> scope, AST *tree);
    static PortType resolveExpressionType(ExpressionNode *exprnode, QVector<AST *> scope, AST *tree);
    static PortType resolveRangeType(RangeNode *rangenode, QVector<AST *> scope, AST *tree);

    static int evaluateConstInteger(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors);
    static double evaluateConstReal(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors);
    static AST *getMemberfromBlockBundle(BlockNode *block, int index, QList<LangError> &errors);
    static AST *getValueFromConstBlock(BlockNode *block);
    static AST *getMemberFromList(ListNode *node, int index, QList<LangError> &errors);
    static PropertyNode *findPropertyByName(vector<PropertyNode *> properties, QString propertyName);
    static ListNode *validTypesForPort(BlockNode *typeDeclaration, QString portName, QVector<AST *> scope, AST *tree);
    static BlockNode *findTypeDeclarationByName(QString typeName, QVector<AST *> scopeStack, AST *tree, QList<LangError> &errors);
    static BlockNode *findTypeDeclaration(BlockNode *block, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    static QVector<AST *> getPortsForTypeBlock(BlockNode *block, QVector<AST *> scope, AST *tree);
    static QVector<AST *> getPortsForType(QString typeName, QVector<AST *> scope, AST *tree);

    /// Number of parallel streams that a single stream can be broken up into
    static int numParallelStreams(StreamNode *stream, StreamPlatform &platform, QVector<AST *> &scope, AST *tree, QList<LangError> &errors);

    /// Get the number of parallel nodes implicit in node. i.e. into how many parallel streams
    /// can the node be broken up.
    static int getNodeSize(AST *node, AST *tree);

    static int getNodeNumOutputs(AST *node, const QVector<AST *> &scope, AST *tree, QList<LangError> &errors);
    static int getNodeNumInputs(AST *node, const QVector<AST *> &scope, AST *tree, QList<LangError> &errors);
    static int getTypeNumOutputs(BlockNode *blockDeclaration, const QVector<AST *> &scope, AST *tree, QList<LangError> &errors);
    static int getTypeNumInputs(BlockNode *blockDeclaration, const QVector<AST *> &scope, AST *tree, QList<LangError> &errors);

    static int getBlockDeclaredSize(BlockNode *block, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    static int getLargestPropertySize(vector<PropertyNode *> &properties, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    static AST *getBlockSubScope(BlockNode *block);

    static QString getPortTypeName(PortType type);

    void validate();

private:

    QVector<PlatformNode *> getPlatformNodes();
    QVector<AST *> getBlocksInScope(AST *root);

    void validateTypes(AST *node, QVector<AST *> scope);
    void validateStreamMembers(StreamNode *node, QVector<AST *> scopeStack);
    void validateBundleIndeces(AST *node, QVector<AST *> scope);
    void validateBundleSizes(AST *node, QVector<AST *> scope);
    void validateSymbolUniqueness(AST *node, QVector<AST *> scope);
    void validateListTypeConsistency(AST *node, QVector<AST *> scope);
    void validateStreamSizes(AST *tree, QVector<AST *> scope);

    void sortErrors();

    void validateStreamInputSize(StreamNode *stream, QVector<AST *> scope, QList<LangError> &errors);

    int getBlockDataSize(BlockNode *block, QVector<AST *> scope, QList<LangError> &errors);
    static int getBundleSize(BundleNode *bundle, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    QString getNodeText(AST *node);

    StreamPlatform *m_platform;
    AST *m_tree;
    QList<LangError> m_errors;
};

#endif // CODEGEN_H
