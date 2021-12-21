#ifndef SYSTEMCONFIGURATION_HPP
#define SYSTEMCONFIGURATION_HPP

#include "ast.h"
#include "declarationnode.h"
#include "valuenode.h"

#include <map>
#include <string>
#include <variant>

using ConfigMap =
    std::map<std::string, std::variant<int64_t, double, std::string>>;

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
