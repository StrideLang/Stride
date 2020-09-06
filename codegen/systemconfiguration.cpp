#include "systemconfiguration.hpp"

SystemConfiguration::SystemConfiguration() {
  platformConfigurations["all"] = ConfigMap();
}

void SystemConfiguration::readProjectConfiguration(std::string filename) {
  std::string configFilename = filename + ".config";
  ASTNode configFile = AST::parseFile(configFilename.c_str());
  if (configFile) {
    for (ASTNode configNode : configFile->getChildren()) {
      if (configNode->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> configDecl =
            std::static_pointer_cast<DeclarationNode>(configNode);
        if (configDecl->getObjectType() == "config") {
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
              platformConfigurations["all"][optionName] =
                  QString::fromStdString(
                      std::static_pointer_cast<ValueNode>(configValue)
                          ->getStringValue());
            } else if (configValue->getNodeType() == AST::Int) {
              platformConfigurations["all"][optionName] =
                  std::static_pointer_cast<ValueNode>(configValue)
                      ->getIntValue();
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
              overrides["all"][optionName] =
                  std::static_pointer_cast<ValueNode>(configValue)
                      ->getIntValue();
            }
          }
        }
      }
    }
  }
}
