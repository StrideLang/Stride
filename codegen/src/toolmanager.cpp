#include "stride/codegen/toolmanager.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "stride/codegen/astfunctions.hpp"
#include "stride/parser/blocknode.h"
#include "stride/parser/declarationnode.h"
#include "stride/parser/valuenode.h"

ToolManager::ToolManager(std::string strideRoot) {
  setStrideRoot(strideRoot);
  readTemplates();
  readLocalConfigs();
}

void ToolManager::readTemplates() {

  std::filesystem::create_directories(m_strideRoot + "/tools");
  toolTemplates.clear();
  toolSearches.clear();

  pathTemplates.clear();
  pathSearches.clear();

  std::filesystem::path rootDir(m_strideRoot + "/tools");
  for (auto file : std::filesystem::directory_iterator{rootDir}) {
    if (std::filesystem::is_regular_file(file) &&
        file.path().extension() == ".stride") {
      auto tree = AST::parseFile(file.path().generic_string().c_str());
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
                  auto toolTemplateNode =
                      decl->getPropertyValue("toolTemplate");
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
                      //                              Overwriting." <<
                      //                              std::endl;
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
                std::cerr << "ERROR in platforms port in toolSearch"
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
                std::cerr << "ERROR in platforms port in pathTemplate"
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
                  auto toolTemplateNode =
                      decl->getPropertyValue("pathTemplate");
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
                                << pathTemplate << ". Overwriting."
                                << std::endl;
                    }
                    pathSearches[pathTemplate] = std::make_shared<PathSearch>();
                    pathSearches[pathTemplate]->rootPathRegex = rootPathRegex;
                  } else {
                    std::cerr << "ERROR: missing pathTemplate in pathSearch"
                              << std::endl;
                  }
                } else {
                  std::cerr << "ERROR invalid platform in pathSearch"
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
        std::cerr << "ERROR parsing tool file: " << file.path() << std::endl;
      }
    }
  }
}

void ToolManager::readLocalConfigs() {

  if (!std::filesystem::exists(m_strideRoot + "/local/tools")) {
    std::filesystem::create_directories(m_strideRoot + "/local/tools");
  }

  localTools.clear();
  localPaths.clear();
  std::filesystem::path rootDir(m_strideRoot + "/local/tools");
  for (const auto &file : std::filesystem::directory_iterator{rootDir}) {
    if (std::filesystem::is_regular_file(file) &&
        file.path().extension() == ".stride") {
      auto tree = AST::parseFile(file.path().generic_string().c_str());
      if (tree) {
        for (const auto &child : tree->getChildren()) {
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
                          << file.path() << std::endl;
              }
            } else if (decl->getObjectType() == "pathInstance") {
              auto pathNode = decl->getPropertyValue("path");
              if (pathNode && pathNode->getNodeType() == AST::String) {
                std::string path = std::static_pointer_cast<ValueNode>(pathNode)
                                       ->getStringValue();

                if (path.find("~") != std::string::npos) {
#ifndef _WINDOWS
                  auto home = std::getenv("HOME");
#else
                  auto home = std::getenv("USERPROFILE");
#endif
                  path.replace(path.find("~"), 1, std::string(home));
                }
                localPaths[decl->getName()] = path;

              } else {
                std::cerr << "ERROR: Invalid executable port in tool instance "
                          << file.path() << std::endl;
              }
            }
          }
        }
      } else {
        std::cerr << "ERROR pasring tool instance: " << file.path()
                  << std::endl;
      }
    }
  }
}

void ToolManager::updateAllLocalConfigs() {
  if (std::filesystem::exists(m_strideRoot + "/local/tools")) {
    std::filesystem::remove_all(m_strideRoot + "/local/tools");
  }

  std::filesystem::create_directories(m_strideRoot + "/local/tools");

  for (const auto &toolTemplate : toolTemplates) {
    if (toolSearches.find(toolTemplate->strideName) == toolSearches.end()) {
      std::cerr << "No search directive for tool: " << toolTemplate->strideName
                << std::endl;
      continue;
    }
    std::string foundExecutable;
    auto search = toolSearches[toolTemplate->strideName];

    // TODO implement regex search of name
    std::filesystem::path fi(search->nameRegex);
    if (fi.is_absolute() && std::filesystem::exists(fi)) {
      // First try path as absolute
      foundExecutable = search->nameRegex;
    } else {
      if (search->rootPathRegex.size() > 0) {
        // FIXME add regex search
        if (search->rootPathRegex[0] == '~') {
#ifndef _WINDOWS
          auto home = std::getenv("HOME");
#else
          auto home = std::getenv("USERPROFILE");
#endif
          search->rootPathRegex =
              std::string(home) + search->rootPathRegex.substr(1);
        }
        if (std::filesystem::exists(search->rootPathRegex + "/" +
                                    search->nameRegex)) {
          foundExecutable = search->rootPathRegex + "/" + search->nameRegex;
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
          if (std::filesystem::exists(token + "/" + search->nameRegex)) {
            foundExecutable = token + "/" + search->nameRegex;
            break;
          }
        }
      }
    }

    if (foundExecutable.size() > 0) {
      std::ofstream f(m_strideRoot + "/local/tools/" + toolTemplate->output +
                      ".stride");
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

  for (const auto &pathTemplate : pathTemplates) {
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
      std::ofstream f(m_strideRoot + "/local/tools" + pathTemplate->output +
                      ".stride");
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
