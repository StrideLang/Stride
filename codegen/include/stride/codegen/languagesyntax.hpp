#ifndef LANGUAGESYNTAX_HPP
#define LANGUAGESYNTAX_HPP

#include <string>

#include "codeentities.hpp"
#include "stride/parser/expressionnode.h"
#include "stride/parser/valuenode.h"
#include "stridesystem.hpp"

class LanguageSyntax {
public:
  static std::string endStatement() { return ";\n"; }

  static std::string getDeclarationType(std::string type, std::string name,
                                        SignalAccess access, uint64_t size);
  static std::string
  getDeclarationForType(std::string type, std::string name, SignalAccess access,
                        uint64_t size, std::vector<std::string> defaultValue,
                        std::vector<std::string> constructorArgs = {});
  static std::string getDeclarationForType(Instance &instance);

  //  static std::string instance(Instance &inst, bool close = true);

  static std::string include(std::string includeToken);

  //  static std::string instanceReal(
  //      std::string name, int size = 1, bool close = true,
  //      std::vector<std::string> defaultValue = std::vector<std::string>());
  //  static std::string instanceBool(
  //      std::string name, int size = 1, bool close = true,
  //      std::vector<std::string> defaultValue = std::vector<std::string>());

  static std::string assignment(std::string name, std::string value,
                                bool close = true);

  static std::string dereference(std::string varName) { return "*" + varName; }

  static std::string reference(std::string varName) { return "&" + varName; }

  static std::string generateExpression(ExpressionNode::ExpressionType type,
                                        std::string left,
                                        std::string right = std::string());

  static std::string domainProcessCall(std::string domainId,
                                       std::string instanceToken,
                                       std::string outToken) {
    return functionCall(instanceToken + "." + domainId + "_process", outToken);
  }

  static std::string functionCall(std::string name, std::string arguments,
                                  std::string templateParams = "",
                                  bool close = true);

  static std::string functionDeclaration(std::string returnType,
                                         std::string name,
                                         std::string arguments,
                                         std::string code,
                                         std::string templateArgs = "");

  static std::string generateModuleDeclaration(std::string functionName,
                                               std::string templateArgs,
                                               std::string declarations,
                                               std::string arguments,
                                               std::string constructorCode,
                                               std::string functionCode);

  static std::string reactionConditionBegin(std::string condition) {
    return "if (" + condition + ") {\n";
  }

  static std::string reactionConditionEnd() { return "}\n"; }
  static std::string loopConditionBegin(std::string condition) {
    return "while (!(" + condition + ")) {\n";
  }

  static std::string loopConditionEnd() { return "}\n"; }

  static std::string triggerBegin(std::string condition) {
    return "if (" + condition + ") {\n";
  }

  static std::string triggerEnd() { return "}\n"; }

  static std::string trueKeyword() { return "true"; }

  static std::string falseKeyword() { return "false"; }

  static std::string numberValue(std::shared_ptr<ValueNode> valueNode) {
    std::string literalString;
    if (valueNode->getNodeType() == AST::Int) {
      literalString = valueNode->toString();
    } else if (valueNode->getNodeType() == AST::Real) {
      literalString = valueNode->toString();
      if (literalString.find('.') == std::string::npos) {
        literalString += ".";
      }
      literalString += "f";
    }

    return literalString;
  }

  //  static std::string getDataType(std::string strideType);

  static std::string getDataType(std::shared_ptr<DeclarationNode> decl,
                                 StrideSystem *system);
};

#endif // LANGUAGESYNTAX_HPP
