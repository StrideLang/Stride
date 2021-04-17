#include "systemconfiguration.hpp"

#include <fstream>
#include <iostream>

SystemConfiguration::SystemConfiguration() {
  //  resourceConfigurations["all"] = ConfigMap();
}

void SystemConfiguration::readConfiguration(std::string filename) {
  std::string configFilename = filename + ".config";
  ASTNode configFile = AST::parseFile(configFilename.c_str());
  if (configFile) {
    for (ASTNode configNode : configFile->getChildren()) {
      if (configNode->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> configDecl =
            std::static_pointer_cast<DeclarationNode>(configNode);
        if (configDecl->getObjectType() == "config") {
          std::string framework;
          auto frameworkNode = configDecl->getPropertyValue("framework");
          if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
            framework = std::static_pointer_cast<ValueNode>(frameworkNode)
                            ->getStringValue();
          }

          if (configDecl->getPropertyValue("resourceConfigValues")) {
            for (auto configOption :
                 configDecl->getPropertyValue("resourceConfigValues")
                     ->getChildren()) {
              if (configOption->getNodeType() == AST::Declaration) {
                // TODO Should we support DeclarationBundle too?
                auto configOptionDecl =
                    std::static_pointer_cast<DeclarationNode>(configOption);
                std::string optionName = configOptionDecl->getName();
                resourceConfigurations["all"].push_back(configOptionDecl);
              }
            }
          }
        } else if (configDecl->getObjectType() == "override") {
          for (auto configOptions : configDecl->getProperties()) {
            std::string optionName = configOptions->getName();
            // TODO more robust capitalization
            if (optionName[0] == '_') {
              if (optionName[1] >= 'a' && optionName[1 <= 'z']) {
                optionName[1] += 'A' - 'a';
              }
              //                            optionName[1] =
              //                            optionName[1].toUpper();
            } else {
              optionName[0] += 'A' - 'a';
            }
            ASTNode configValue = configOptions->getValue();
            if (configValue->getNodeType() == AST::String) {
              overrides["all"][optionName] = QString::fromStdString(
                  std::static_pointer_cast<ValueNode>(configValue)
                      ->getStringValue());
            } else if (configValue->getNodeType() == AST::Int) {
              overrides["all"][optionName] = QVariant::fromValue(
                  std::static_pointer_cast<ValueNode>(configValue)
                      ->getIntValue());
            }
          }
        }
      }
    }
  }
}

void SystemConfiguration::writeConfiguration(std::string filename) {
  std::ofstream configFile(filename);

  if (!configFile.good()) {
    std::cerr << "Error opening code file for writing!" << std::endl;
    ;
    //        throw;
    return;
  }
  std::string presetName = "Preset";
  for (auto frameworkConfig : resourceConfigurations) {
    configFile << "config " << presetName << "{" << std::endl;
    configFile << "    framework: \"" << frameworkConfig.first << "\""
               << std::endl;

    configFile << "    resourceConfigValues: [" << std::endl;
    for (auto option = frameworkConfig.second.cbegin();
         option != frameworkConfig.second.cend(); ++option) {
      configFile << AST::toText(*option, 4);
    }
    configFile << "    ]\n}\n" << std::endl;

    configFile << "override Test {\n" << std::endl;
    for (auto option = overrides["all"].cbegin();
         option != overrides["all"].cend(); ++option) {
      std::string optionName = option->first;
      // TODO more robust capitalization
      if (optionName[0] == '_') {
        if (optionName[1] >= 'A' && optionName[1 <= 'Z']) {
          optionName[1] -= 'A' - 'a';
        }
        //                            optionName[1] =
        //                            optionName[1].toLower();
      } else {
        optionName[0] -= 'A' - 'a';
      }
      configFile << "    " << optionName.c_str() << ": " << std::endl;
      if (option->second.type() == QVariant::String) {
        configFile << "\"" << option->second.toByteArray().constData() << "\"";
      } else {
        configFile << option->second.toString().toLocal8Bit().constData();
      }
      configFile << std::endl;
    }
    configFile << "}" << std::endl;
  }
  configFile.close();
}
