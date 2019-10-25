#include "languagesyntax.hpp"

std::string LanguageSyntax::getDeclarationType(std::string type,
                                               std::string name,
                                               SignalAccess access, int size) {
  std::string declType;
  std::string base;
  if (size > 1) {
    base = "Bundle";
  } else {
    base = "Signal";
  }
  // FIXME cover all cases
  std::string helperType = "stride::" + base + "Helper<double>";
  if ((access & ACCESS_SDRst) || (access & ACCESS_MDRst)) {
    if (access & ACCESS_SDR && access & ACCESS_SDW) {
      declType =
          "stride::" + base + "_SDRWRst<" + helperType + ", " + type + ">";
    } else if (access & ACCESS_SDR && access & ACCESS_MDW) {
      declType =
          "stride::" + base + "_SDRWRst<" + helperType + ", " + type + ">";
    } else {  // Fallback
      declType =
          "stride::" + base + "_SDRWRst<" + helperType + ", " + type + ">";
    }
  } else {  // No reset
    if (access & ACCESS_SDR && access & ACCESS_SDW) {
      declType = "stride::" + base + "_SDRW<" + helperType + ", " + type + ">";
    } else if (access & ACCESS_SDR && access & ACCESS_MDW) {
      declType = "stride::" + base + "_SDRW<" + helperType + ", " + type + ">";
    } else {  // Fallback
      declType = "stride::" + base + "_SDRW<" + helperType + ", " + type + ">";
    }
  }

  return declType;
}

std::string LanguageSyntax::getDeclarationForType(
    std::string type, std::string name, SignalAccess access, int size,
    std::vector<std::string> defaultValue) {
  std::string out;

  if (access == ACCESS_NONE && !(access & ACCESS_SDRst) &&
      !(access & ACCESS_MDRst)) {
    std::string bundleSize;
    if (size > 1) {
      bundleSize = "[" + std::to_string(size) + "]";
    }
    out += type + " " + name + bundleSize;
    if (defaultValue.size() == 1) {
      out += " = ";
      int count = size;
      if (size > 1) {
        out += "{";
      }
      while (count > 0) {
        out += defaultValue[0] + ",";
        count--;
      }
      if (defaultValue.size() > 0) {
        out = out.substr(0, out.size() - 1);
      }
      if (size > 1) {
        out += "}";
      }
    } else if (defaultValue.size() > 1) {
      out += " = {";
      int count = size;
      while (count > 0) {
        for (auto value : defaultValue) {
          out += value + ",";
          count--;
        }
      }
      if (defaultValue.size() > 0) {
        out = out.substr(0, out.size() - 1);
      }
      out += "}";
    }
    out += ";\n";
    return out;
  }
  std::string base;
  if (size > 1) {
    base = "Bundle";
  } else {
    base = "Signal";
  }
  out +=
      "using " + name + "_Helper_Type = stride::" + base + "Helper<double>;\n";

  if (size > 1) {
    out += name + "_Helper_Type " + name + "_Helper{" + std::to_string(size) +
           "};\n";
  } else {
    out += name + "_Helper_Type " + name + "_Helper{";
    for (auto value : defaultValue) {
      out += value + ",";
    }
    if (defaultValue.size() > 0) {
      out = out.substr(0, out.size() - 1);
    }
    out += "};\n";
  }
  //        out += "std::mutex ResetMutex;"

  std::string structType = getDeclarationType(type, name, access, size);

  out += "using " + name + "_Type = " + structType + ";";

  if (size > 1) {
    out += name + "_Type " + name + "{&" + name +
           "_Helper_Type::init_External, &" + name + "_Helper, " +
           std::to_string(size) + "};\n";
  } else {
    out += name + "_Type " + name + "{&" + name +
           "_Helper_Type::init_External, &" + name + "_Helper};\n";
  }
  return out;
}

string LanguageSyntax::getDeclarationForType(Instance instance) {
  return getDeclarationForType(instance.type, instance.fullName(),
                               instance.access, instance.size,
                               instance.defaultValue);
}

std::string LanguageSyntax::instance(Instance &inst, bool close) {
  std::string out;
  if (inst.type == "double") {
    if (inst.access == ACCESS_NONE) {
      out += instanceReal(inst.fullName(), inst.size, true, inst.defaultValue);
    } else {
      out = "// " + inst.fullName() + "\n";
      out += getDeclarationForType(inst);
    }
  } else if (inst.type == "bool") {
    if (inst.access == ACCESS_NONE) {
      out = LanguageSyntax::instanceBool(inst.fullName(), inst.size, true,
                                         inst.defaultValue);
    } else {
      out = "// " + inst.fullName() + "\n";
      out += getDeclarationForType(inst);
    }
  } else {
    // Do we need to look at size here?
    out = inst.type;
    if (inst.templateArgs.size() > 0) {
      out += "<";
      for (auto arg : inst.templateArgs) {
        out += arg + ",";
      }
      out = out.substr(0, out.size() - 1);
      out += ">";
    }
    out += " " + inst.fullName();
    if (inst.constructorArgs.size() > 0) {
      out += "{";
      for (auto arg : inst.constructorArgs) {
        out += arg + ",";
      }
      out = out.substr(0, out.size() - 1);
      out += "}";
    }
    if (close) {
      out += ";\n";
    }
  }
  return out;
}

std::string LanguageSyntax::instanceReal(
    std::string name, int size, bool close,
    std::vector<std::string> defaultValue) {
  std::string decl = "double " + name;
  if (size > 1) {
    decl += "[" + std::to_string(size) + "] ";
    if (defaultValue.size() > 0) {
      decl += " = {";
      for (auto v : defaultValue) {
        decl += " " + v + ",";
      }
      decl.resize(decl.size() - 1);  // Chop off last comma
      decl += "}";
    }
  } else {
    if (defaultValue.size() == 1) {
      decl += " = " + defaultValue[0];
    }
  }
  if (close) {
    decl += endStatement();
  }
  return decl;
}

std::string LanguageSyntax::instanceBool(
    std::string name, int size, bool close,
    std::vector<std::string> defaultValue) {
  std::string decl = "bool " + name;
  if (size > 1) {
    decl += "[" + std::to_string(size) + "] ";
    if (defaultValue.size() > 0) {
      decl += " = {";
      for (auto v : defaultValue) {
        decl += " " + v + ",";
      }
      decl.resize(decl.size() - 1);  // Chop off last comma
      decl += "}";
    }
  } else {
    if (defaultValue.size() == 1) {
      decl += " = " + defaultValue[0];
    }
  }
  if (close) {
    decl += endStatement();
  }
  return decl;
}

std::string LanguageSyntax::assignment(std::string name, std::string value,
                                       bool close) {
  std::string out = name + " = " + value;
  if (close) {
    out += endStatement();
  }
  return out;
}

std::string LanguageSyntax::generateExpression(
    ExpressionNode::ExpressionType type, std::string left, std::string right) {
  switch (type) {
    case ExpressionNode::Multiply:
      return left + " * " + right;
    case ExpressionNode::Divide:
      return left + " / " + right;
    case ExpressionNode::Add:
      return left + " + " + right;
    case ExpressionNode::Subtract:
      return left + " - " + right;
    case ExpressionNode::And:
      return left + " & " + right;
    case ExpressionNode::Or:
      return left + " | " + right;
    case ExpressionNode::UnaryMinus:
      return " - " + left;
    case ExpressionNode::LogicalNot:
      return " ~ " + left;
    default:
      break;
  }
  return "";
}

std::string LanguageSyntax::functionCall(std::string name,
                                         std::string arguments, bool close) {
  std::string text = name + "(" + arguments + ")";
  if (close) {
    text += ";\n";
  }
  return text;
}

std::string LanguageSyntax::functionDeclaration(std::string returnType,
                                                std::string name,
                                                std::string arguments,
                                                std::string code,
                                                std::string templateArgs) {
  std::string text;
  if (templateArgs.size() > 0) {
    text += "template<" + templateArgs + ">\n";
  }
  text += returnType + " " + name + "(" + arguments + ") {\n";
  text += code;
  text += "}\n";
  return text;
}

std::string LanguageSyntax::generateModuleDeclaration(
    std::string functionName, std::string templateArgs,
    std::string declarations, std::string arguments,
    std::string constructorCode, std::string functionCode) {
  // Module Class Declaration --------------------------------------------
  // FIXME make sure there are no name clashes by using namespaces

  // If port properties used, pass them as constructor and template arguments
  //    std::string declarations;
  //    std::string arguments;
  //    std::string constructorCode;
  std::string moduleCode;
  if (templateArgs.size() > 0) {
    moduleCode += "template<" + templateArgs + ">\n";
  }
  moduleCode += "class " + functionName + " {\npublic:\n";
  moduleCode += declarations;
  if (arguments.size() > 0 && constructorCode.size() > 0) {
    moduleCode += functionName + "(" + arguments + ")";
    moduleCode += "{" + constructorCode + "}\n";
  }

  moduleCode += functionCode;

  moduleCode += "};\n\n";
  return moduleCode;
  //        std::cout<< moduleCode <<std::endl;
}

std::string LanguageSyntax::getDataType(std::string strideType) {
  if (strideType == "signal") {
    // FIXME we need to
    return "double";
  } else if (strideType == "switch") {
    return "bool";
  }
  return "";
}
