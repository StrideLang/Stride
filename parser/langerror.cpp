#include "langerror.h"

#include <sstream>

LangError::LangError()
{
  
}

std::string LangError::getErrorText() {
    std::string errorText = "In file " + filename  + ":\n    " ;
    std::stringstream lineNumString;
    lineNumString << lineNumber;
    errorText += "Line " + lineNumString.str() + " : ";
    switch(type) {
    case Syntax:
        errorText += "Syntax Error: Unexpected character '" + errorTokens[0] + "'";
        break;
    case UnknownType:
        errorText += "Unknown Type Error. Type '" + errorTokens[0] + "' not recognized.";
        break;
    case InvalidType:
        errorText += "Invalid Type: '" + errorTokens[0] + "'";
        break;
    case InvalidPort:
        errorText += "Invalid Port '" + errorTokens[1]
                + "' for type '" + errorTokens[0] + "'";
        break;
    case InvalidPortType:
        errorText += "Invalid port type Error. Port '" + errorTokens[1]
                + "' in Block '" + errorTokens[0]
                + "' expects '" + errorTokens[2] + "'";
        break;
    case IndexMustBeInteger:
        errorText += "Index to array must be integer ";
        break;
    case BundleSizeMismatch:
        errorText += "Bundle Size Mismatch Error";
        break;
    case ArrayIndexOutOfRange:
        errorText += "Array Index out of Range Error";
        break;
    case DuplicateSymbol:
        errorText += "Duplicate Symbol Error";
        break;
    case InconsistentList:
        errorText += "Inconsistent List Error";
        break;
    case StreamMemberSizeMismatch:
        errorText += "Stream size mismatch. '" + errorTokens[1]
                + "' provides " + errorTokens[0] + "outs , " + errorTokens[2] + " expected.";
        break;
    case UndeclaredSymbol:
        errorText += "Undeclared Symbol '" + errorTokens[0] + "'";
        break;
    case SystemError:
        errorText += "Undeclared Symbol '" + errorTokens[0] + "'";
        break;
    case None:
        errorText += "Unknown error";
        break;
    default:
        break;
    }
    return errorText;
}
