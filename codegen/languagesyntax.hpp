#ifndef LANGUAGESYNTAX_HPP
#define LANGUAGESYNTAX_HPP

#include <string>

#include "codeentities.hpp"
#include "../parser/expressionnode.h"

class LanguageSyntax {
public:

    static std::string endStatement() {return ";\n";}

    static std::string instance(Instance &inst, bool close = true);

    static std::string instanceReal(std::string name, int size = 1, bool close = true, std::vector<std::string> defaultValue = std::vector<std::string>());

    static std::string assignment(std::string name, std::string value, bool close = true);

    static std::string dereference(std::string varName) {
        return "*" + varName;
    }

    static std::string reference(std::string varName) {
        return "&" + varName;
    }

    static std::string generateExpression(ExpressionNode::ExpressionType type, std::string left, std::string right = std::string());


    static std::string domainProcessCall(std::string domainId, std::string instanceToken, std::string outToken) {
        return functionCall(instanceToken + "." + domainId + "_process", outToken);
    }

    static std::string functionCall(std::string name, std::string arguments, bool close = true);

    static std::string functionDeclaration(std::string returnType, std::string name,
                                           std::string arguments, std::string code, std::string templateArgs = "");

    static std::string generateModuleDeclaration(std::string functionName, std::string templateArgs,
                                   std::string declarations, std::string arguments,
                                   std::string constructorCode, std::string functionCode
                                   );

    static std::string reactionConditionBegin(std::string condition) {
        return "if (" + condition + ") {\n";
    }

    static std::string reactionConditionEnd() {
        return "}\n";
    }

    static std::string getDataType(std::string strideType);
};


#endif // LANGUAGESYNTAX_HPP
