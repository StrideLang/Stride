#include "stride/codegen/languagesyntax.hpp"
#include "stride/codegen/stridesystem.hpp"

#include <iostream>

std::string LanguageSyntax::getDeclarationType(std::string type,
                                               std::string name,
                                               SignalAccess access,
                                               uint64_t size) {
  std::string declType;
  std::string base;
  if (size > 1) {
    base = "Bundle";
  } else {
    base = "Signal";
  }
  // FIXME cover all cases
  std::string helperType = "stride::" + base + "Helper<double>";
  if ((access.access & ACCESS_SDRst) || (access.access & ACCESS_MDRst)) {
    if (access.access & ACCESS_SDR && access.access & ACCESS_SDW) {
      declType =
          "stride::" + base + "_SDRWRst<" + helperType + ", " + type + ">";
    } else if (access.access & ACCESS_SDR && access.access & ACCESS_MDW) {
      declType =
          "stride::" + base + "_SDRWRst<" + helperType + ", " + type + ">";
    } else { // Fallback
      declType =
          "stride::" + base + "_SDRWRst<" + helperType + ", " + type + ">";
    }
  } else { // No reset
    if (access.access & ACCESS_SDR && access.access & ACCESS_SDW) {
      declType = "stride::" + base + "_SDRW<" + helperType + ", " + type + ">";
    } else if (access.access & ACCESS_SDR && access.access & ACCESS_MDW) {
      declType = "stride::" + base + "_SDRW<" + helperType + ", " + type + ">";
    } else { // Fallback
      declType = "stride::" + base + "_SDRW<" + helperType + ", " + type + ">";
    }
  }

  return declType;
}

std::string LanguageSyntax::getDeclarationForType(
    std::string type, std::string name, SignalAccess access, uint64_t size,
    std::vector<std::string> defaultValue,
    std::vector<std::string> constructorArgs) {
  std::string out;
  if (size == -1) {
    out += "// FIXME Size unresolved\n";
    size = defaultValue.size();
  }

  if (access.access == ACCESS_NONE || access.access == ACCESS_SDRst) {
    std::string bundleSize;
    if (size > 1) {
      bundleSize = "[" + std::to_string(size) + "]";
    }
    std::string constructorInitList;
    if (constructorArgs.size() > 0) {
      constructorInitList = "{";
      for (auto &arg : constructorArgs) {
        constructorInitList += arg + ",";
      }
      constructorInitList.back() = '}';
    }
    out += type + " " + name + bundleSize + constructorInitList;
    if (defaultValue.size() == 1) {
      out += " = ";
      uint64_t count = size;
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
      uint64_t count = size;
      while (count > 0) {
        for (const auto &value : defaultValue) {
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
    for (const auto &value : defaultValue) {
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

std::string LanguageSyntax::getDeclarationForType(Instance &instance) {
  return getDeclarationForType(instance.type, instance.fullName(),
                               instance.access, instance.size,
                               instance.defaultValue, instance.constructorArgs);
}

// std::string LanguageSyntax::instance(Instance &inst, bool close) {
//  std::string out;
//  if (inst.type == "double") {
//    if (inst.access == ACCESS_NONE) {
//      out += instanceReal(inst.fullName(), inst.size, true,
//      inst.defaultValue);
//    } else {
//      out = "// " + inst.fullName() + "\n";
//      out += getDeclarationForType(inst);
//    }
//  } else if (inst.type == "bool") {
//    if (inst.access == ACCESS_NONE) {
//      out = LanguageSyntax::instanceBool(inst.fullName(), inst.size, true,
//                                         inst.defaultValue);
//    } else {
//      out = "// " + inst.fullName() + "\n";
//      out += getDeclarationForType(inst);
//    }
//  } else {
//    // Do we need to look at size here?
//    out = inst.type;
//    if (inst.templateArgs.size() > 0) {
//      out += "<";
//      for (auto arg : inst.templateArgs) {
//        out += arg + ",";
//      }
//      out = out.substr(0, out.size() - 1);
//      out += ">";
//    }
//    out += " " + inst.fullName();
//    if (inst.constructorArgs.size() > 0) {
//      out += "{";
//      for (auto arg : inst.constructorArgs) {
//        out += arg + ",";
//      }
//      out = out.substr(0, out.size() - 1);
//      out += "}";
//    }
//    if (close) {
//      out += ";\n";
//    }
//  }
//  return out;
//}

std::string LanguageSyntax::include(std::string includeName) {
  std::string includetext;
  if (includeName.at(0) == '<') {
    includetext += "#include " + includeName + "\n";
  } else {
    includetext += "#include \"" + includeName + "\"\n";
  }
  return includetext;
}

// std::string
// LanguageSyntax::instanceReal(std::string name, int size, bool close,
//                             std::vector<std::string> defaultValue) {
//  std::string decl = "double " + name;
//  if (size > 1) {
//    decl += "[" + std::to_string(size) + "] ";
//    if (defaultValue.size() > 0) {
//      decl += " = {";
//      for (auto v : defaultValue) {
//        decl += " " + v + ",";
//      }
//      decl.resize(decl.size() - 1); // Chop off last comma
//      decl += "}";
//    }
//  } else {
//    if (defaultValue.size() == 1) {
//      decl += " = " + defaultValue[0];
//    }
//  }
//  if (close) {
//    decl += endStatement();
//  }
//  return decl;
//}

// std::string
// LanguageSyntax::instanceBool(std::string name, int size, bool close,
//                             std::vector<std::string> defaultValue) {
//  std::string decl = "bool " + name;
//  if (size > 1) {
//    decl += "[" + std::to_string(size) + "] ";
//    if (defaultValue.size() > 0) {
//      decl += " = {";
//      for (auto v : defaultValue) {
//        decl += " " + v + ",";
//      }
//      decl.resize(decl.size() - 1); // Chop off last comma
//      decl += "}";
//    }
//  } else {
//    if (defaultValue.size() == 1) {
//      decl += " = " + defaultValue[0];
//    }
//  }
//  if (close) {
//    decl += endStatement();
//  }
//  return decl;
//}

std::string LanguageSyntax::assignment(std::string name, std::string value,
                                       bool close) {
  std::string out = name + " = " + value;
  if (close) {
    out += endStatement();
  }
  return out;
}

std::string
LanguageSyntax::generateExpression(ExpressionNode::ExpressionType type,
                                   std::string left, std::string right) {
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
                                         std::string arguments,
                                         std::string templateParams,
                                         bool close) {
  std::string text = name;
  if (templateParams.size() > 0) {
    text += "<" + templateParams + ">";
  }

  text += "(" + arguments + ")";
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

std::string LanguageSyntax::getDataType(std::shared_ptr<DeclarationNode> decl,
                                        StrideSystem *system) {
  std::string dataType;
  std::string frameworkName;
  auto frameworkNode = decl->getCompilerProperty("framework");
  if (frameworkNode) {
    frameworkName =
        std::static_pointer_cast<ValueNode>(frameworkNode)->getStringValue();
  }
  if (decl->getObjectType() == "switch" ||
      decl->getObjectType() == "reaction") {
    // TODO get this from platform defintions
    return "bool /*FIXME languagesyntax.cpp*/";
  }
  // FIXME need to get type from input block if module or loop

  auto declaredType = decl->getPropertyValue("type");
  if (declaredType && declaredType->getNodeType() == AST::Block) {
    // FIXME we need to determine framework
    auto dataTypeDecl = system->getFrameworkDataType(
        "", std::static_pointer_cast<BlockNode>(declaredType)->getName());
    if (dataTypeDecl) {
      auto frameworkTypeNode = dataTypeDecl->getPropertyValue("frameworkName");

      if (frameworkTypeNode &&
          frameworkTypeNode->getNodeType() == AST::String) {
        return std::static_pointer_cast<ValueNode>(frameworkTypeNode)
            ->getStringValue();
      }
    }
  }

  auto defaultDataType =
      system->getFrameworkDefaultDataType(frameworkName, decl->getObjectType());

  auto dataTypeNode =
      system->getFrameworkDataType(frameworkName, defaultDataType);

  if (!dataTypeNode || dataTypeNode->getNodeType() == AST::None) {
    // Fallback, but should never get here...
    std::cerr << "ERROR, undefined data type for " << decl->getObjectType()
              << std::endl;
    defaultDataType = system->getFrameworkDefaultDataType(frameworkName, "");
    dataTypeNode = system->getFrameworkDataType(frameworkName, defaultDataType);
  }

  if (dataTypeNode) {
    auto dataTypeNameNode = dataTypeNode->getPropertyValue("frameworkName");
    if (dataTypeNameNode && dataTypeNameNode->getNodeType() == AST::String) {
      dataType = std::static_pointer_cast<ValueNode>(dataTypeNameNode)
                     ->getStringValue();
    }
  } else {
    std::cerr << "ERROR: no datatype provided" << std::endl;
  }
  return dataType;
}
