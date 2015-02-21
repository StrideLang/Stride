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
        InvalidType
    } ErrorType;

    ErrorType type;
    QString errorToken;
    int lineNumber;
};

class Codegen
{
public:
    Codegen(StreamPlatform &platform, AST * tree);

    bool isValid();

    QList<LangError> getErrors();

private:
    void validate();
    void checkTypeNames(AST *node);

    StreamPlatform m_platform;
    AST *m_tree;
    QList<LangError> m_errors;
};

#endif // CODEGEN_H
