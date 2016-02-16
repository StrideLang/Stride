#ifndef CODEGEN_H
#define CODEGEN_H

#include <QString>

#include "strideparser.h"
#include "streamplatform.h"

class CodeValidator
{
public:
    CodeValidator(StreamPlatform &platform, AST * tree);
    CodeValidator(QString platformRootDir, AST * tree);

    typedef enum {
        Audio,
        ControlReal,
        ControlInt,
        ControlBoolean,
        ControlString,
        ConstReal,
        ConstInt,
        ConstBoolean,
        ConstString,
        None,
        Invalid
    } PortType;

    bool isValid();
    bool platformIsValid();

    QList<LangError> getErrors();
    QStringList getPlatformErrors();

    StreamPlatform *getPlatform();

    static BlockNode *findDeclaration(QString objectName, QVector<AST *> scope, AST *tree);
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

    /// Number of parallel streams that a single stream can be broken up into
    static int numParallelStreams(StreamNode *stream, StreamPlatform &platform, QVector<AST *> &scope, AST *tree, QList<LangError> &errors);

    /// Get the number of parallel nodes implicit in node. i.e. into how many parallel streams
    /// can the node be broken up.
    static int getNodeSize(AST *node, AST *tree);


    static int getNodeNumOutputs(AST *node, StreamPlatform &platform, QVector<AST *> &scope, AST *tree, QList<LangError> &errors);
    static int getNodeNumInputs(AST *node, StreamPlatform &platform, QVector<AST *> &scope, AST *tree, QList<LangError> &errors);

    static int getBlockDeclaredSize(BlockNode *block, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    static int getLargestPropertySize(vector<PropertyNode *> &properties, QVector<AST *> scope, AST *tree, QList<LangError> &errors);

    static QString getPortTypeName(PortType type);

private:

    QVector<PlatformNode *> getPlatformNodes();

    void validate();
    void validateTypeNames(AST *node);
    void validateProperties(AST *node, QVector<AST *> scope);
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

    StreamPlatform m_platform;
    AST *m_tree;
    QList<LangError> m_errors;
};

#endif // CODEGEN_H
