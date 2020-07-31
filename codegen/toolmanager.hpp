#ifndef TOOLMANAGER_HPP
#define TOOLMANAGER_HPP

#include <iostream>
#include <string>

#include <QDir>

#include "ast.h"

struct ToolTemplate {
  std::string strideName;
  std::string displayName;
  std::string output;
  std::string meta;
};

struct ToolSearch {
  std::string rootPathRegex;
  std::string nameRegex;
};

struct PathTemplate {
  std::string strideName;
  std::string displayName;
  std::string output;
  std::string meta;
};

struct PathSearch {
  std::string rootPathRegex;
};

class ToolManager {
public:
  ToolManager() {}
  ToolManager(std::string strideRoot);

  void setStrideRoot(std::string strideRoot) { m_strideRoot = strideRoot; }

  void readTemplates();

  void readLocalConfigs();

  void updateAllLocalConfigs();

  std::map<std::string, std::string> localTools;

  std::vector<std::shared_ptr<ToolTemplate>> toolTemplates;
  std::map<std::string, std::shared_ptr<ToolSearch>> toolSearches;

  std::map<std::string, std::string> localPaths;

  std::vector<std::shared_ptr<PathTemplate>> pathTemplates;
  std::map<std::string, std::shared_ptr<PathSearch>> pathSearches;

private:
  std::string m_strideRoot;
};

#endif // TOOLMANAGER_HPP
