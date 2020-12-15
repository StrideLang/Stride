#include "toolmanager.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>

#include "blocknode.h"
#include "declarationnode.h"
#include "valuenode.h"

ToolManager::ToolManager(std::string strideRoot) {
  setStrideRoot(strideRoot);
  readTemplates();
  readLocalConfigs();
}

void ToolManager::readTemplates() {
  QDir rootDir(QString::fromStdString(m_strideRoot));
  if (!rootDir.exists("tools")) {
    rootDir.mkdir("tools");
  }
  rootDir.cd("tools");

  toolTemplates.clear();
  toolSearches.clear();

  pathTemplates.clear();
  pathSearches.clear();

  auto files = rootDir.entryList(QStringList() << "*.stride");
  for (auto file : files) {
    auto tree = AST::parseFile(
        (rootDir.path() + QDir::separator() + file).toLocal8Bit().constData());
    if (tree) {
      for (auto child : tree->getChildren()) {
        if (child->getNodeType() == AST::Declaration) {
          auto decl = std::static_pointer_cast<DeclarationNode>(child);
          if (decl->getObjectType() == "toolTemplate") { // ----------------
            auto platformsNode = decl->getPropertyValue("platforms");
            if (platformsNode && platformsNode->getNodeType() == AST::List) {
              bool validPlatform = false;
              for (auto validOSNode : platformsNode->getChildren()) {
                if (validOSNode->getNodeType() == AST::String) {
#ifdef Q_OS_LINUX
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "Linux") {
#elif defined(Q_OS_MACOS)
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "macOS") {
#elif defined(Q_OS_WINDOWS)
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "Windows") {
#else
                  if (false) {
#endif
                    validPlatform = true;
                    break;
                  }
                }
              }
              if (validPlatform) {
                std::string displayName;
                auto displayNameNode = decl->getPropertyValue("displayName");
                if (displayNameNode &&
                    displayNameNode->getNodeType() == AST::String) {
                  displayName =
                      std::static_pointer_cast<ValueNode>(displayNameNode)
                          ->getStringValue();
                }
                std::string output;
                auto outputNode = decl->getPropertyValue("output");
                if (outputNode && outputNode->getNodeType() == AST::Block) {
                  output = std::static_pointer_cast<BlockNode>(outputNode)
                               ->getName();
                }
                std::string meta;
                auto metaNode = decl->getPropertyValue("meta");
                if (metaNode && metaNode->getNodeType() == AST::String) {
                  meta = std::static_pointer_cast<ValueNode>(metaNode)
                             ->getStringValue();
                }

                std::string name = decl->getName();
                auto newTool = std::make_shared<ToolTemplate>();
                newTool->meta = meta;
                newTool->output = output;
                newTool->strideName = decl->getName();
                newTool->displayName = displayName;
                toolTemplates.push_back(newTool);
              }

            } else {
              std::cerr << "ERROR in platforms port in toolTemplate"
                        << std::endl;
            }
          } else if (decl->getObjectType() == "toolSearch") { // -----------
            auto platformsNode = decl->getPropertyValue("platforms");
            if (platformsNode && platformsNode->getNodeType() == AST::List) {
              bool validPlatform = false;
              for (auto validOSNode : platformsNode->getChildren()) {
                if (validOSNode->getNodeType() == AST::String) {
#ifdef Q_OS_LINUX
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "Linux") {
#elif defined(Q_OS_MACOS)
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "macOS") {
#elif defined(Q_OS_WINDOWS)
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "Windows") {
#else
                  if (false) {
#endif
                    validPlatform = true;
                    break;
                  }
                }
              }
              if (validPlatform) {
                std::string toolTemplate;
                auto toolTemplateNode = decl->getPropertyValue("toolTemplate");
                if (toolTemplateNode &&
                    toolTemplateNode->getNodeType() == AST::Block) {
                  toolTemplate =
                      std::static_pointer_cast<BlockNode>(toolTemplateNode)
                          ->getName();
                }
                std::string rootPathRegex;
                auto rootPathRegexNode =
                    decl->getPropertyValue("rootPathRegex");
                if (rootPathRegexNode &&
                    rootPathRegexNode->getNodeType() == AST::String) {
                  rootPathRegex =
                      std::static_pointer_cast<ValueNode>(rootPathRegexNode)
                          ->getStringValue();
                }
                std::string nameRegex;
                auto binaryNode = decl->getPropertyValue("nameRegex");
                if (binaryNode && binaryNode->getNodeType() == AST::String) {
                  nameRegex = std::static_pointer_cast<ValueNode>(binaryNode)
                                  ->getStringValue();
                }

                if (toolTemplate.size() > 0) {
                  if (toolSearches.find(toolTemplate) != toolSearches.end()) {
                    //                    std::cout << "Warning: tool searches
                    //                    already contain tool "
                    //                                 "template: "
                    //                              << toolTemplate << ".
                    //                              Overwriting." << std::endl;
                    // FIXME allow multiple tool searches.
                  }
                  toolSearches[toolTemplate] = std::make_shared<ToolSearch>();
                  toolSearches[toolTemplate]->rootPathRegex = rootPathRegex;
                  toolSearches[toolTemplate]->nameRegex = nameRegex;

                } else {
                  std::cerr << "ERROR: missing toolTemplate in toolSearch"
                            << std::endl;
                }
              }

            } else {
              std::cerr << "ERROR in platforms port in toolTemplate"
                        << std::endl;
            }
          } else if (decl->getObjectType() ==
                     "pathTemplate") { // ----------------
            auto platformsNode = decl->getPropertyValue("platforms");
            if (platformsNode && platformsNode->getNodeType() == AST::List) {
              bool validPlatform = false;
              for (auto validOSNode : platformsNode->getChildren()) {
                if (validOSNode->getNodeType() == AST::String) {
#ifdef Q_OS_LINUX
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "Linux") {
#elif defined(Q_OS_MACOS)
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "macOS") {
#elif defined(Q_OS_WINDOWS)
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "Windows") {
#else
                  if (false) {
#endif
                    validPlatform = true;
                    break;
                  }
                }
              }
              if (validPlatform) {
                std::string displayName;
                auto displayNameNode = decl->getPropertyValue("displayName");
                if (displayNameNode &&
                    displayNameNode->getNodeType() == AST::String) {
                  displayName =
                      std::static_pointer_cast<ValueNode>(displayNameNode)
                          ->getStringValue();
                }
                std::string output;
                auto outputNode = decl->getPropertyValue("output");
                if (outputNode && outputNode->getNodeType() == AST::Block) {
                  output = std::static_pointer_cast<BlockNode>(outputNode)
                               ->getName();
                }
                std::string meta;
                auto metaNode = decl->getPropertyValue("meta");
                if (metaNode && metaNode->getNodeType() == AST::String) {
                  meta = std::static_pointer_cast<ValueNode>(metaNode)
                             ->getStringValue();
                }

                std::string name = decl->getName();
                auto newTool = std::make_shared<PathTemplate>();
                newTool->meta = meta;
                newTool->output = output;
                newTool->strideName = decl->getName();
                newTool->displayName = displayName;
                pathTemplates.push_back(newTool);
              }

            } else {
              std::cerr << "ERROR in platforms port in toolTemplate"
                        << std::endl;
            }
          } else if (decl->getObjectType() == "pathSearch") { // -----------
            auto platformsNode = decl->getPropertyValue("platforms");
            if (platformsNode && platformsNode->getNodeType() == AST::List) {
              bool validPlatform = false;
              for (auto validOSNode : platformsNode->getChildren()) {
                if (validOSNode->getNodeType() == AST::String) {
#ifdef Q_OS_LINUX
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "Linux") {
#elif defined(Q_OS_MACOS)
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "macOS") {
#elif defined(Q_OS_WINDOWS)
                  if (std::static_pointer_cast<ValueNode>(validOSNode)
                          ->getStringValue() == "Windows") {
#else
                  if (false) {
#endif
                    validPlatform = true;
                    break;
                  }
                }
              }
              if (validPlatform) {
                std::string pathTemplate;
                auto toolTemplateNode = decl->getPropertyValue("pathTemplate");
                if (toolTemplateNode &&
                    toolTemplateNode->getNodeType() == AST::Block) {
                  pathTemplate =
                      std::static_pointer_cast<BlockNode>(toolTemplateNode)
                          ->getName();
                }
                std::string rootPathRegex;
                auto rootPathRegexNode =
                    decl->getPropertyValue("rootPathRegex");
                if (rootPathRegexNode &&
                    rootPathRegexNode->getNodeType() == AST::String) {
                  rootPathRegex =
                      std::static_pointer_cast<ValueNode>(rootPathRegexNode)
                          ->getStringValue();
                }

                if (pathTemplate.size() > 0) {
                  if (toolSearches.find(pathTemplate) != toolSearches.end()) {
                    std::cout << "Wanring: tool searces already contain tool "
                                 "template: "
                              << pathTemplate << ". Overwriting." << std::endl;
                  }
                  pathSearches[pathTemplate] = std::make_shared<PathSearch>();
                  pathSearches[pathTemplate]->rootPathRegex = rootPathRegex;
                } else {
                  std::cerr << "ERROR: missing toolTemplate in toolSearch"
                            << std::endl;
                }
              } else {
                std::cerr << "ERROR in platforms port in toolTemplate"
                          << std::endl;
              }
            }
          } else {
            std::cerr << "Unsupported declaration in tool file." << std::endl;
          }
        } else {
          std::cerr << "Unexpected node in tool file." << std::endl;
        }
      }
    } else {
      std::cerr << "ERROR parsing tool file: " << file.toStdString()
                << std::endl;
    }
  }
}

void ToolManager::readLocalConfigs() {
  QDir rootDir(QString::fromStdString(m_strideRoot) + QDir::separator() +
               "local/tools");

  localTools.clear();
  localPaths.clear();
  auto files = rootDir.entryList(QStringList() << "*.stride");
  for (auto file : files) {
    auto tree = AST::parseFile(
        (rootDir.path() + QDir::separator() + file).toLocal8Bit().constData());
    if (tree) {
      for (auto child : tree->getChildren()) {
        if (child->getNodeType() == AST::Declaration) {
          auto decl = std::static_pointer_cast<DeclarationNode>(child);
          if (decl->getObjectType() == "toolInstance") {
            auto executableNode = decl->getPropertyValue("executable");
            if (executableNode &&
                executableNode->getNodeType() == AST::String) {
              std::string executable =
                  std::static_pointer_cast<ValueNode>(executableNode)
                      ->getStringValue();
              localTools[decl->getName()] = executable;

            } else {
              std::cerr << "ERROR: Invalid executable port in tool instance "
                        << file.toStdString() << std::endl;
            }
          } else if (decl->getObjectType() == "pathInstance") {
            auto pathNode = decl->getPropertyValue("path");
            if (pathNode && pathNode->getNodeType() == AST::String) {
              std::string path = std::static_pointer_cast<ValueNode>(pathNode)
                                     ->getStringValue();

              if (path.find("~") != std::string::npos) {
                path.replace(path.find("~"), 1, QDir::homePath().toStdString());
              }
              localPaths[decl->getName()] = path;

            } else {
              std::cerr << "ERROR: Invalid executable port in tool instance "
                        << file.toStdString() << std::endl;
            }
          }
        }
      }
    } else {
      std::cerr << "ERROR pasring tool instance: " << file.toStdString()
                << std::endl;
    }
  }
}

void ToolManager::updateAllLocalConfigs() {
  QDir rootDir(QString::fromStdString(m_strideRoot));

  if (!rootDir.exists("local")) {
    rootDir.mkdir("local");
  }
  rootDir.cd("local");
  if (rootDir.exists("tools")) {
    rootDir.cd("tools");
    rootDir.removeRecursively();
    rootDir.cdUp();
  }
  rootDir.mkdir("tools");
  rootDir.cd("tools");
  for (auto toolTemplate : toolTemplates) {
    if (toolSearches.find(toolTemplate->strideName) == toolSearches.end()) {
      std::cerr << "No search directive for tool: " << toolTemplate->strideName
                << std::endl;
      continue;
    }
    std::string foundExecutable;
    auto search = toolSearches[toolTemplate->strideName];

    // TODO implement regex search of name
    QFileInfo fi(QString::fromStdString(search->nameRegex));
    if (fi.isAbsolute() && fi.exists()) {
      // First try path as absolute
      foundExecutable = search->nameRegex;
    } else {
      if (search->rootPathRegex.size() > 0) {
        // FIXME add regex search
        if (search->rootPathRegex[0] == '~') {
          search->rootPathRegex =
              QDir::homePath().toStdString() + search->rootPathRegex.substr(1);
        }
        if (QFile::exists(QString::fromStdString(search->rootPathRegex) +
                          QDir::separator() +
                          QString::fromStdString(search->nameRegex))) {
          foundExecutable = search->rootPathRegex +
                            QDir::separator().toLatin1() + search->nameRegex;
        }
      }
      // Then check if in system path
      if (foundExecutable.size() == 0) {
        auto pathEnv = getenv("PATH");
#ifdef Q_OS_WINDOWS
        const char delim = ';';
#else
        const char delim = ':';
#endif
        std::stringstream ss(pathEnv);
        std::string token;
        while (std::getline(ss, token, delim)) {
          if (QFile::exists(QString::fromStdString(token) + QDir::separator() +
                            QString::fromStdString(search->nameRegex))) {
            foundExecutable =
                token + QDir::separator().toLatin1() + search->nameRegex;
            break;
          }
        }
      }
    }

    if (foundExecutable.size() > 0) {
      std::ofstream f((rootDir.path() + QDir::separator()).toStdString() +
                      toolTemplate->output + ".stride");
      if (!f.good()) {
        std::cerr << "ERROR creating tool config file:"
                  << toolTemplate->strideName << std::endl;
        continue;
      }
      std::string fileContents =
          "toolInstance " + toolTemplate->output + " {\n";
      fileContents += "  toolTemplate: " + toolTemplate->strideName + "\n";
      fileContents += "  executable: '" + foundExecutable + "'\n";
      fileContents += "}\n";

      f << fileContents;
      f.close();
    }
  }

  for (auto pathTemplate : pathTemplates) {
    std::string foundPath;
    auto search = pathSearches[pathTemplate->strideName];

    // TODO implement regex search of name
    foundPath = search->rootPathRegex;
    //    QDir d(QString::fromStdString(search->rootPathRegex));
    //    if (d.isAbsolute() && d.exists()) {
    //      // First try path as absolute
    //    } else {
    //      // TODO how to handle relative paths?
    //    }

    if (foundPath.size() > 0) {
      std::ofstream f((rootDir.path() + QDir::separator()).toStdString() +
                      pathTemplate->output + ".stride");
      if (!f.good()) {
        std::cerr << "ERROR creating path instance file:"
                  << pathTemplate->strideName << std::endl;
        continue;
      }
      std::string fileContents =
          "pathInstance " + pathTemplate->output + " {\n";
      fileContents += "  pathTemplate: " + pathTemplate->strideName + "\n";
      fileContents += "  path: '" + foundPath + "'\n";
      fileContents += "}\n";

      f << fileContents;
      f.close();
    }
  }
}
