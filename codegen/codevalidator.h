#ifndef CODEGEN_H
#define CODEGEN_H

#include <QString>

#include "streamparser.h"
#include "streamplatform.h"

class LangError {
public:
    typedef enum {
        Syntax,
        UnknownType,
        InvalidType,
        InvalidPort,
        InvalidPortType,
        IndexMustBeInteger,
        BundleSizeMismatch,
        ArrayIndexOutOfRange,
        DuplicateSymbol,
        InconsistentList,
        StreamMemberSizeMismatch,
        UndeclaredSymbol,
        None
    } ErrorType;

    ErrorType type;
    QStringList errorTokens;
    int lineNumber;
    QString getErrorText();
};

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

    StreamPlatform getPlatform();

    static BlockNode *findDeclaration(QString objectName, QVector<AST *> scope, AST *tree);
    static PortType resolveBundleType(BundleNode *bundle, QVector<AST *> scope, AST *tree);
    static PortType resolveNameType(NameNode *name, QVector<AST *> scope, AST *tree);
    static PortType resolveNodeOutType(AST *node, QVector<AST *> scope, AST *tree);
    static PortType resolveListType(ListNode *listnode, QVector<AST *> scope, AST *tree);
    static PortType resolveExpressionType(ExpressionNode *exprnode, QVector<AST *> scope, AST *tree);

    static int evaluateConstInteger(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors);
    static double evaluateConstReal(AST *node, QVector<AST *> scope, AST *tree, QList<LangError> &errors);
    static AST *getMemberfromBlockBundle(BlockNode *block, int index, QList<LangError> &errors);
    static AST *getValueFromConstBlock(BlockNode *block);
    static AST *getMemberFromList(ListNode *node, int index, QList<LangError> &errors);

    static int largestBundleSize(StreamNode *stream, AST *tree);
    static int getBundleSize(AST *node, AST *tree);

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

    int getBlockBundleDeclaredSize(BlockNode *block, QVector<AST *> scope, QList<LangError> &errors);
    int getBlockDataSize(BlockNode *block, QVector<AST *> scope, QList<LangError> &errors);
    int getNodeSize(AST *node, QVector<AST *> &scope, QList<LangError> &errors);

    StreamPlatform m_platform;
    AST *m_tree;
    QList<LangError> m_errors;
};

#endif // CODEGEN_H
