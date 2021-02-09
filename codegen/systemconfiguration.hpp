#ifndef SYSTEMCONFIGURATION_HPP
#define SYSTEMCONFIGURATION_HPP

#include "ast.h"
#include "declarationnode.h"
#include "valuenode.h"

// TODO remove QVariant, use std
#include <QVariant>
#include <map>
#include <string>

typedef std::map<std::string, QVariant> ConfigMap;

class SystemConfiguration {
public:
  SystemConfiguration();
  std::map<std::string, ConfigMap>
      overrides; // Override existing values in Stride Code
  std::map<std::string, ConfigMap>
      substitutions; // Substitute strings in generated code
  std::map<std::string, std::vector<std::shared_ptr<DeclarationNode>>>
      resourceConfigurations; // Configure resources
  std::map<std::string, ConfigMap>
      frameworkConfigurations; // Configure frameworks

  bool testing = false;

  void readConfiguration(std::string filename);
  void writeConfiguration(std::string filename);
};

#endif // SYSTEMCONFIGURATION_HPP
