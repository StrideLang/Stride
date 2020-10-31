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
  std::map<std::string, ConfigMap>
      hardwareConfigurations; // Configure the hardware
  std::map<std::string, ConfigMap>
      platformConfigurations; // Configure the platform

  bool testing = false;

  void readProjectConfiguration(std::string filename);
};

#endif // SYSTEMCONFIGURATION_HPP
