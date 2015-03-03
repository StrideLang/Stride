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
        None
    } ErrorType;

    ErrorType type;
    QStringList errorTokens;
    int lineNumber;
    QString getErrorText();
};

class Codegen
{
public:
    Codegen(StreamPlatform &platform, AST * tree);
    Codegen(QString platformRootDir, AST * tree);


    bool isValid();
    bool platformIsValid();

    QList<LangError> getErrors();
    QStringList getPlatformErrors();

private:
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

    void validate();
    void validateTypeNames(AST *node);
    void validateProperties(AST *node, QVector<AST *> scope);
    void validateBundleIndeces(AST *node, QVector<AST *> scope);
    void validateBundleSizes(AST *node, QVector<AST *> scope);
    void validateSymbolUniqueness(AST *node, QVector<AST *> scope);
    void validateListConsistency(AST *node, QVector<AST *> scope);
    void sortErrors();

    BlockNode *findDeclaration(QString bundleName, QVector<AST *> scope);
    int getBlockBundleDeclaredSize(BlockNode *block, QVector<AST *> scope, QList<LangError> &errors);
    int getConstBlockDataSize(BlockNode *block, QVector<AST *> scope, QList<LangError> &errors);

    PortType resolveBundleType(BundleNode *bundle, QVector<AST *> scope);
    PortType resolveNodeOutType(AST *node, QVector<AST *> scope);
    PortType resolveListType(ListNode *listnode, QVector<AST *> scope);
    PortType resolveExpressionType(ExpressionNode *exprnode, QVector<AST *> scope);

    int evaluateConstInteger(AST *node, QVector<AST *> scope, QList<LangError> &errors);
    AST *getMemberfromBlockBundle(BlockNode *node, int index, QList<LangError> &errors);
    AST *getMemberFromList(ListNode *node, int index, QList<LangError> &errors);

    QString getPortTypeName(PortType type);

    StreamPlatform m_platform;
    AST *m_tree;
    QList<LangError> m_errors;
};

#endif // CODEGEN_H
