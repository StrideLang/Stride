#ifndef LANGERROR_H
#define LANGERROR_H

#include <string>
#include <vector>

class LangError {
public:
    LangError();

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
        SystemError,
        None
    } ErrorType;

    ErrorType type;
    std::vector<std::string> errorTokens;
    int lineNumber;
    std::string getErrorText();
};

#endif // LANGERROR_H
