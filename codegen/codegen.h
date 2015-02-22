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
        InvalidProperty,
        InvalidPropertyType
    } ErrorType;

    ErrorType type;
    QStringList errorTokens;
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
    void checkProperties(AST *node);

    StreamPlatform m_platform;
    AST *m_tree;
    QList<LangError> m_errors;
};

#endif // CODEGEN_H
