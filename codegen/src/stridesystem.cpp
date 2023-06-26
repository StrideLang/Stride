/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <cassert>
#ifndef _WIN32
#include <dlfcn.h>
#endif
#include <filesystem>
#include <memory.h>
#include <string>

#include "stride/codegen/astfunctions.hpp"
#include "stride/codegen/codeanalysis.hpp"
#include "stride/codegen/codevalidator.hpp"
#include "stride/codegen/stridesystem.hpp"
#include "stride/codegen/toolmanager.hpp"

#include "stride/codegen/astquery.hpp"
#include "stride/parser/declarationnode.h"
#include "stride/parser/propertynode.h"
//#include "pythonproject.h"

StrideSystem::StrideSystem(std::string strideRoot, std::string systemName,
                           int majorVersion, int minorVersion,
                           std::vector<std::shared_ptr<ImportNode>> importList)
    : m_strideRoot(strideRoot), m_systemName(systemName),
      m_majorVersion(majorVersion), m_minorVersion(minorVersion) {
  std::string versionString =
      std::to_string(m_majorVersion) + "." + std::to_string(m_minorVersion);

  m_systemPath = std::filesystem::path(strideRoot + "/systems/" + systemName +
                                       "/" + versionString)
                     .generic_string();
  std::string systemFile = m_systemPath + "/System.stride";

  for (auto importNode : importList) {
    m_importList[importNode->importName()] = importNode->importAlias();
  }

  if (std::filesystem::exists(systemFile)) {
    ASTNode systemTree = AST::parseFile(systemFile.c_str(), nullptr);
    if (systemTree) {
      parseSystemTree(systemTree);

      // Iterate through platforms reading them.
      // TODO Should optimize this to not reread platform if already done.

      // Load testing trees
      for (auto framework : m_frameworks) {
        auto platformPath =
            std::filesystem::path(framework->buildTestingLibPath());
        if (std::filesystem::exists(platformPath)) {
          for (const auto &file :
               std::filesystem::directory_iterator{platformPath}) {
            if (file.is_regular_file() &&
                file.path().extension() == ".stride") {
              ASTNode tree =
                  AST::parseFile(file.path().generic_string().c_str(), nullptr);
              if (tree) {
                for (auto node : tree->getChildren()) {
                  node->setCompilerProperty(
                      "framework",
                      std::make_shared<ValueNode>(
                          getFrameworkAlias(framework->getFramework()),
                          __FILE__, __LINE__));
                }
                framework->addTestingTree(file.path().stem().generic_string(),
                                          tree);
              } else {
                std::vector<LangError> errors = AST::getParseErrors();
                for (LangError error : errors) {
                  std::cerr << error.getErrorText() << std::endl;
                }
                continue;
              }
            }
          }
        }
      }
      //                m_platformPath = fullPath;
      //                m_api = PythonTools;
      //                m_types = getPlatformTypeNames();
    } else {
      std::cerr << __FILE__ << ":" << __LINE__
                << "Error parsing system tree in:" << systemFile << std::endl;
    }
  } else {
    std::cerr << __FILE__ << ":" << __LINE__
              << "System file not found:" << systemFile << std::endl;
  }

  //    if (m_api == NullPlatform) {
  //        qDebug() << "Platform not found!";
  //    }
}

StrideSystem::~StrideSystem() {}

std::vector<std::string>
StrideSystem::listAvailableSystems(std::string strideroot) {
  std::vector<std::string> outEntries;
  std::filesystem::path dir(strideroot);
  dir += "/systems";
  for (auto const &dir_entry : std::filesystem::directory_iterator{dir}) {
    std::cout << dir_entry << '\n';
    if (dir_entry.is_directory()) {
      outEntries.push_back(dir_entry.path().generic_string());
    }
  }
  return outEntries;
}

void StrideSystem::parseSystemTree(ASTNode systemTree, ASTNode configuration) {
  std::vector<std::pair<std::string, std::string>> usedPlatformNames;
  std::vector<std::map<std::string, std::string>> platformDefinitions;
  std::map<std::string, std::pair<std::string, std::string>> frameworkInherits;
  //    vector<string> platformDefinitionNames;
  std::vector<AST *>
      connectionDefinitions; // TODO process connection definitions

  for (const ASTNode &systemNode : systemTree->getChildren()) {
    if (systemNode->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> declaration =
          std::static_pointer_cast<DeclarationNode>(systemNode);
      if (declaration->getObjectType() == "platform") {
        std::map<std::string, std::string> definition;
        std::vector<std::shared_ptr<PropertyNode>> properties =
            declaration->getProperties();
        for (const std::shared_ptr<PropertyNode> &prop : properties) {
          if (prop->getValue()->getNodeType() == AST::String) {
            definition[prop->getName()] =
                static_cast<ValueNode *>(prop->getValue().get())
                    ->getStringValue();
          }
        }
        platformDefinitions.push_back(definition);
        m_platformDefinitions.push_back(
            std::static_pointer_cast<DeclarationNode>(declaration));
      } else if (declaration->getObjectType() == "connection") {
        m_connectionDefinitions.push_back(declaration);
      } else if (declaration->getObjectType() == "system") {
        ASTNode platforms = declaration->getPropertyValue("platforms");
        if (!platforms) {
          std::cerr << "ERROR: platforms port not found in system definition"
                    << std::endl;
        } else if (platforms->getNodeType() == AST::List) {
          ListNode *platformsList = static_cast<ListNode *>(platforms.get());
          for (ASTNode platformName : platformsList->getChildren()) {
            assert(platformName->getNodeType() == AST::Block);
            auto platformSpecBlock =
                std::static_pointer_cast<BlockNode>(platformName);
            auto platformSpec = ASTQuery::findDeclarationByName(
                platformSpecBlock->getName(), {}, systemTree);
            if (platformSpec) {
              auto platformBlock = platformSpec->getPropertyValue("framework");
              auto rootNamespaceBlock =
                  platformSpec->getPropertyValue("rootNamespace");
              if (rootNamespaceBlock) {
                if (platformBlock->getNodeType() == AST::String &&
                    rootNamespaceBlock->getNodeType() == AST::String) {
                  usedPlatformNames.push_back(
                      {std::static_pointer_cast<ValueNode>(platformBlock)
                           ->getStringValue(),
                       std::static_pointer_cast<ValueNode>(rootNamespaceBlock)
                           ->getStringValue()});
                } else {
                  std::cerr << "ERROR: Unexpected types for platform spec "
                            << platformSpecBlock->getName() << std::endl;
                }
              } else if (platformBlock->getNodeType() == AST::String) {
                usedPlatformNames.push_back(
                    {std::static_pointer_cast<ValueNode>(platformBlock)
                         ->getStringValue(),
                     ""});
              }
            } else {
              std::cerr << "ERROR: could not find platform spec "
                        << platformSpecBlock->getName() << std::endl;
            }
          }
        } else {
          std::cerr << "ERROR: Expected list for platforms in system definition"
                    << std::endl;
        }
      } /*else {
          std::cerr << "ERROR: Unknown system declaration type";
      }*/
    }
  }

  // Resolve platforms in connections to avoid having to look them up later
  for (auto connection : m_connectionDefinitions) {
    auto sourcePlatform = connection->getPropertyValue("sourceFramework");
    auto destPlatform = connection->getPropertyValue("destinationFramework");
    // TODO currently only looking in the system tree. Should we allow other
    // locations?
    for (auto systemNode : systemTree->getChildren()) {
      if (systemNode->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> declaration =
            std::static_pointer_cast<DeclarationNode>(systemNode);
        if (declaration->getObjectType() == "platform") {
          if (sourcePlatform->getNodeType() == AST::Block) {
            if (declaration->getName() ==
                std::static_pointer_cast<BlockNode>(sourcePlatform)
                    ->getName()) {
              connection->replacePropertyValue(
                  "sourceFramework",
                  declaration->getPropertyValue("framework"));
              assert(
                  declaration->getPropertyValue("framework")->getNodeType() ==
                  AST::String);
            }
          } else if (sourcePlatform->getNodeType() == AST::String) {
            std::cerr << "ERROR: expenting block or string in source platform "
                         "for connection "
                      << connection->getName() << std::endl;
          }
          if (destPlatform->getNodeType() == AST::Block) {
            if (declaration->getName() ==
                std::static_pointer_cast<BlockNode>(destPlatform)->getName()) {
              connection->replacePropertyValue(
                  "destinationFramework",
                  declaration->getPropertyValue("framework"));
              assert(
                  declaration->getPropertyValue("framework")->getNodeType() ==
                  AST::String);
            }
          } else if (destPlatform->getNodeType() == AST::String) {
            std::cerr << "ERROR: expenting block or string in source platform "
                         "for connection "
                      << connection->getName() << std::endl;
          }
        }
      }
    }
  }

  // Now connect platforms referenced in system with defined platforms
  for (auto usedPlatformName : usedPlatformNames) {
    for (size_t i = 0; i < platformDefinitions.size(); i++) {
      std::map<std::string, std::string> &definition =
          platformDefinitions.at(i);
      //            string &name = platformDefinitionNames.at(i);
      if (definition["framework"] == usedPlatformName.first) {
        std::string framework, framworkVersion, hardware, hardwareVersion,
            rootNamespace;
        framework = definition["framework"];

        std::shared_ptr<StrideFramework> newPlatform =
            std::make_shared<StrideFramework>(
                m_strideRoot, definition["framework"],
                definition["frameworkVersion"], definition["hardware"],
                definition["hardwareVersion"], definition["rootNamespace"],
                frameworkInherits[definition["framework"]].first,
                frameworkInherits[definition["framework"]].second);

        m_frameworks.push_back(newPlatform);
      } else {
        // TODO add error
      }
    }
  }

  // Initialize root namespace
  m_library.initializeLibrary(m_strideRoot);

  for (auto libmembers : m_library.getLibraryMembers()) {
    if (m_systemNodes.find(libmembers.first) == m_systemNodes.end()) {
      m_systemNodes[libmembers.first] = libmembers.second;
    } else {
      m_systemNodes[libmembers.first].insert(
          m_systemNodes[libmembers.first].end(), libmembers.second.begin(),
          libmembers.second.end());
    }
  }

  for (auto fw : m_frameworks) {
    auto fwNodes = fw->getFrameworkMembers();

    for (auto members : fwNodes) {
      if (m_systemNodes.find(members.first) == m_systemNodes.end()) {
        m_systemNodes[members.first] = members.second;
      } else {
        m_systemNodes[members.first].insert(m_systemNodes[members.first].end(),
                                            members.second.begin(),
                                            members.second.end());
      }
    }
  }
}

const std::string &StrideSystem::systemName() const { return m_systemName; }

std::string StrideSystem::getStrideRoot() const { return m_strideRoot; }

void StrideSystem::setStrideRoot(const std::string &strideRoot) {
  m_strideRoot = strideRoot;
}

std::vector<std::string> StrideSystem::listAvailableImports() {
  std::filesystem::path dir(m_strideRoot);
  dir += "/library/1.0";
  std::vector<std::string> outEntries;
  for (auto entry : std::filesystem::directory_iterator{dir}) {
    if (entry.is_directory()) {
      outEntries.push_back(entry.path().generic_string());
    }
  }
  for (auto fw : m_frameworks) {
    dir = fw->buildPlatformLibPath();

    for (auto entry : std::filesystem::directory_iterator{dir}) {

      if (entry.is_directory()) {
        if (entry.path().string().size() > 0 &&
            ::isupper(entry.path().string()[0])) {

          if (std::find(outEntries.begin(), outEntries.end(), entry.path()) ==
              outEntries.end()) {
            outEntries.push_back(entry.path().generic_string());
          }
        }
      }
    }
  }
  return outEntries;
}

std::vector<std::string> StrideSystem::getErrors() { return m_errors; }

std::vector<std::string> StrideSystem::getWarnings() { return m_warnings; }

std::vector<std::string> StrideSystem::getPlatformTypeNames() {
  std::vector<std::string> typeNames;
  auto libObjects = getImportTrees();
  for (auto libNamespace : libObjects) {
    for (auto node : libNamespace.second) {
      if (node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> block =
            std::static_pointer_cast<DeclarationNode>(node);
        if (block->getObjectType() == "type" ||
            block->getObjectType() == "platformModule" ||
            block->getObjectType() == "platformBlock") {
          ValueNode *name = static_cast<ValueNode *>(
              block->getPropertyValue("typeName").get());
          if (name && name->getNodeType() == AST::String) {
            typeNames.push_back(name->getStringValue());
          }
        }
      }
    }
  }
  return typeNames;
}

std::vector<std::string> StrideSystem::getFunctionNames() {
  std::vector<std::string> funcNames;

  std::map<std::string, std::vector<ASTNode>> refObjects = getImportTrees();

  for (auto it = refObjects.begin(); it != refObjects.end(); it++) {

    for (ASTNode node : it->second) {
      if (node->getNodeType() == AST::Declaration) {
        DeclarationNode *block = static_cast<DeclarationNode *>(node.get());
        if (block->getObjectType() == "module") {
          if (it->first.size() > 0) {
            funcNames.push_back(it->first + ":" + block->getName());
          } else {
            funcNames.push_back(block->getName());
          }
        }
      }
    }
  }
  return funcNames;
}

std::vector<Builder *> StrideSystem::createBuilders(std::string fileName,
                                                    ASTNode tree) {

  std::vector<Builder *> builders;
  std::string projectDir = makeProject(fileName);
  if (projectDir.size() == 0) {
    std::cerr << "Error creating project path" << std::endl;
    return builders;
  }
  for (auto platform : m_frameworks) {
    auto usedFrameworks = CodeAnalysis::getUsedFrameworks(tree);
    if ((usedFrameworks.size() == 0) ||
        (std::find(usedFrameworks.begin(), usedFrameworks.end(),
                   getFrameworkAlias(platform->getFramework())) !=
         usedFrameworks.end())) {
      if (platform->getAPI() == StrideFramework::PythonTools) {
        std::string pythonExec = "python";
        throw std::exception();
        //        Builder *builder = new PythonProject(
        //            platform->getFramework(), platform->buildPlatformPath(),
        //            m_strideRoot, projectDir, pythonExec);
        //        if (builder) {
        //          if (builder->isValid()) {
        //            builders.push_back(builder);
        //          } else {
        //            delete builder;
        //          }
        //        }
      } else if (platform->getAPI() == StrideFramework::PluginPlatform) {
        auto pluginPath = std::filesystem::path(m_strideRoot + "/plugins");
        // FIXME we should copy plugins to framework directories
        //                auto pluginList =
        //                QDir(QString::fromStdString(platformEntry.second->buildPlatformPath(m_strideRoot.toStdString())
        //                + "/plugins")).entryList(QDir::NoDotAndDotDot |
        //                QDir::Files);

        // TODO get plugin name from configuration or platform files.
        std::string pluginName = "pufferfish";
        int pluginMajorVersion = 1;
        int pluginMinorVersion = 0;

        if (!platform->getPluginDetails(pluginName, pluginMajorVersion,
                                        pluginMinorVersion)) {
          //          qDebug() << "Error getting plugin details";
        }
        std::vector<std::string> validDomains;

        if (std::filesystem::exists(pluginPath)) {
          for (auto plugin : std::filesystem::directory_iterator{pluginPath}) {
            //                    qDebug() << plugin;
            //          QLibrary
            //          pluginLibrary(QString::fromStdString(m_strideRoot) +
            //                                 "/plugins/" + plugin);
#ifndef _WIN32
            auto *lib = dlopen(plugin.path().c_str(), RTLD_NOW);
            if (lib) {
              char name[STRIDE_PLUGIN_MAX_STR_LEN];
              int versionMajor = -1;
              int versionMinor = -1;

              auto nameFunc = (platform_name_t)dlsym(lib, "platform_name");
              if (nameFunc) {
                nameFunc(name);
              }
              if (strncmp(name, pluginName.c_str(),
                          STRIDE_PLUGIN_MAX_STR_LEN) == 0) {

                auto versionMajorFunc = (platform_version_major_t)dlsym(
                    lib, "platform_version_major");
                if (versionMajorFunc) {
                  versionMajor = versionMajorFunc();
                }
                auto versionMinorFunc = (platform_version_minor_t)dlsym(
                    lib, "platform_version_minor");
                if (versionMinorFunc) {
                  versionMinor = versionMinorFunc();
                }
                if (pluginMajorVersion == versionMajor &&
                    pluginMinorVersion == versionMinor) {
                  //                                qDebug() << "Code generator
                  //                                plugin found! " <<
                  //                                QString::fromStdString(pluginName);

                  auto create = (create_object_t)dlsym(lib, "create_object");
                  if (create) {
                    Builder *builder = create(platform->buildPlatformPath(),
                                              m_strideRoot, projectDir);
                    if (builder) {
                      builder->m_frameworkName = platform->getFramework();
                      builders.push_back(builder);
                    }
                  }
                }
              }
            } else {
              std::cerr << dlerror() << std::endl;
            }
#endif
          }
        }
        //                pluginLibrary.unload();
      }
    }
  }
  return builders;
}

ASTNode StrideSystem::getPlatformDomain(std::string namespaceName) {
  ASTNode platformDomain;
  std::map<std::string, std::vector<ASTNode>> libObjects = getImportTrees();
  if (libObjects.find(namespaceName) == libObjects.end()) { // Invalid namespace
    assert(0 == 1);
    return platformDomain;
  }
  for (ASTNode object : libObjects[namespaceName]) {
    if (object->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          std::static_pointer_cast<DeclarationNode>(object);
      if (decl->getName() == "PlatformDomain") {
        if (decl->getObjectType() == "alias") {
          platformDomain = decl->getPropertyValue("block");
          break;
        }
      }
    }
  }
  return platformDomain;
}

std::map<std::string, std::string>
StrideSystem::getFrameworkTools(std::string namespaceName) {
  std::map<std::string, std::string> tools;

  ToolManager toolManager(m_strideRoot);

  std::map<std::string, std::vector<ASTNode>> libObjects = getImportTrees();
  if (libObjects.find(namespaceName) == libObjects.end()) { // Invalid namespace
    assert(0 == 1);
    return std::map<std::string, std::string>();
  }
  //  namespaceName = ""; // Hack! Everything is currently being put into
  //  root...
  for (ASTNode object : libObjects[namespaceName]) {
    if (object->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          std::static_pointer_cast<DeclarationNode>(object);
      if (decl->getObjectType() == "toolRequirement") {
        auto toolInstanceNode = decl->getPropertyValue("toolInstance");
        if (toolInstanceNode && toolInstanceNode->getNodeType() == AST::Block) {
          auto toolInstance =
              std::static_pointer_cast<BlockNode>(toolInstanceNode)->getName();
          if (toolManager.localTools.find(toolInstance) !=
              toolManager.localTools.end()) {

            tools[toolInstance] = toolManager.localTools[toolInstance];
          } else {
            std::cerr << "ERROR tool instance not found in local config"
                      << std::endl;
          }
        } else {
          std::cerr << "ERROR expected toolInstance port in toolRequirement "
                    << decl->getName() << std::endl;
        }
      }
    }
  }
  return tools;
}

std::map<std::string, std::string>
StrideSystem::getFrameworkPaths(std::string namespaceName) {
  std::map<std::string, std::string> paths;

  ToolManager toolManager(m_strideRoot);

  std::map<std::string, std::vector<ASTNode>> libObjects = getImportTrees();
  if (libObjects.find(namespaceName) == libObjects.end()) { // Invalid namespace
    assert(0 == 1);
    return std::map<std::string, std::string>();
  }
  for (ASTNode object : libObjects[namespaceName]) {
    if (object->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          std::static_pointer_cast<DeclarationNode>(object);
      if (decl->getObjectType() == "pathRequirement") {
        auto toolInstanceNode = decl->getPropertyValue("pathInstance");
        if (toolInstanceNode && toolInstanceNode->getNodeType() == AST::Block) {
          auto pathInstance =
              std::static_pointer_cast<BlockNode>(toolInstanceNode)->getName();
          if (toolManager.localPaths.find(pathInstance) !=
              toolManager.localPaths.end()) {

            paths[pathInstance] = toolManager.localPaths[pathInstance];
          } else {
            std::cerr << "ERROR tool instance not found in local config"
                      << std::endl;
          }
        } else {
          std::cerr << "ERROR unexpected pathInstance port in pathRequirement "
                    << decl->getName() << std::endl;
        }
      }
    }
  }
  return paths;
}

std::string StrideSystem::substituteTokens(std::string namespaceName,
                                           std::string text) {
  auto paths = getFrameworkPaths(namespaceName);

  for (auto mapEntry : paths) {
    size_t index = 0;
    while (true) {
      index = text.find("%" + mapEntry.first + "%", index);
      if (index == std::string::npos) {
        break;
      }
      text.replace(index, mapEntry.first.size() + 2, mapEntry.second);
      index += mapEntry.first.size() + 2;
    }
  }
  size_t index = 0;
  while (index < text.size() && text.find("%", index) != std::string::npos) {
    index = text.find("%", index);
    size_t endIndex = text.find("%", index + 1);
    if (endIndex != std::string::npos) {
      auto tokenName = text.substr(index + 1, endIndex - index - 1);
      for (auto frameworkConfig : m_systemConfig.resourceConfigurations) {
        for (auto resourceConfig : frameworkConfig.second) {
          if (resourceConfig->getName() == tokenName) {
            text.replace(
                index, endIndex - index + 1,
                AST::toText(resourceConfig->getPropertyValue("value")));
            break;
          }
        }
      }

      index = endIndex + 1;
    } else {
      index = endIndex;
    }
  }

  return text;
}

std::vector<std::string> StrideSystem::getFrameworkNames() {
  std::vector<std::string> names;
  for (auto platform : m_frameworks) {
    names.push_back(platform->getFramework());
  }
  return names;
}

std::vector<ASTNode> StrideSystem::getOptionTrees(std::string systemPath) {
  std::vector<ASTNode> optionTrees;
  auto platformPath = std::filesystem::path(systemPath + "/options");
  for (auto file : std::filesystem::directory_iterator{platformPath}) {

    if (file.is_regular_file() && file.path().extension() == ".stride") {
      ASTNode optionTree =
          AST::parseFile(file.path().generic_string().c_str(), nullptr);
      if (optionTree) {
        ASTNode finalTree = std::make_shared<AST>();
        for (auto child : optionTree->getChildren()) {
          bool osMatch = true;
          if (child->getNodeType() == AST::Declaration) {
            auto decl = std::static_pointer_cast<DeclarationNode>(child);
            auto osNode = decl->getPropertyValue("buildPlatforms");
            if (osNode && osNode->getNodeType() == AST::List) {
              osMatch = false;
              for (auto validOSNode : osNode->getChildren()) {
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
                    osMatch = true;
                    break;
                  }
                }
              }
            }
          }
          if (osMatch) {
            finalTree->addChild(child);
          }
        }
        optionTrees.push_back(finalTree);
      } else {
        std::cerr << "Error parsing option file: " << file.path().c_str()
                  << std::endl;
      }
    }
  }
  return optionTrees;
}

std::vector<ASTNode> StrideSystem::loadImportTree(std::string importName,
                                                  std::string importAs,
                                                  std::string frameworkName) {
  auto importedNodes = m_library.loadImport(importName, importAs);
  for (auto fw : m_frameworks) {
    if ((frameworkName.size() == 0) || (frameworkName == fw->getFramework())) {
      auto fwnodes = fw->loadImport(importName, importAs);
      importedNodes.insert(importedNodes.begin(), fwnodes.begin(),
                           fwnodes.end());
    }
  }
  if (m_systemNodes.find(importAs) == m_systemNodes.end()) {
    m_systemNodes[importAs] = importedNodes;
  } else {
    m_systemNodes[importAs].insert(m_systemNodes[importAs].end(),
                                   importedNodes.begin(), importedNodes.end());
  }
  return importedNodes;
}

std::map<std::string, std::vector<ASTNode>> StrideSystem::getImportTrees() {
  return m_systemNodes;
}

std::vector<std::shared_ptr<DeclarationNode>>
StrideSystem::getFrameworkSynchronization(std::string frameworkName) {
  std::vector<std::shared_ptr<DeclarationNode>> syncNodes;
  for (auto platform : m_frameworks) {
    if ((frameworkName == platform->getRootNamespace()) ||
        frameworkName == platform->getFramework()) {
      std::string platformPath = platform->buildPlatformLibPath();
      for (auto file : std::filesystem::directory_iterator{platformPath}) {

        if (file.is_regular_file() && file.path().extension() == ".stride") {
          auto newTree =
              AST::parseFile(file.path().generic_string().c_str(), nullptr);
          if (newTree) {
            for (ASTNode node : newTree->getChildren()) {
              if (node->getNodeType() == AST::Declaration) {
                auto decl = std::static_pointer_cast<DeclarationNode>(node);
                if (decl->getObjectType() == "synchronization") {

                  syncNodes.push_back(decl);
                }
              }
            }
          }
        }
      }
    }
  }
  return syncNodes;
}

std::shared_ptr<DeclarationNode>
StrideSystem::getFrameworkDataType(std::string frameworkName,
                                   std::string strideDataType) {
  //  QStringList nameFilters;
  //  nameFilters.push_back("*.stride");
  std::pair<std::string, std::string> inheritedFramework;
  for (auto platform : m_frameworks) {
    if ((frameworkName == platform->getRootNamespace()) ||
        frameworkName == platform->getFramework()) {
      std::string platformPath = platform->buildPlatformLibPath();
      auto dataTypeDecl = findDataTypeInPath(platformPath, strideDataType);
      if (dataTypeDecl) {
        return dataTypeDecl;
      }
    }
  }

  return nullptr;
}

std::string StrideSystem::getFrameworkDefaultDataType(std::string frameworkName,
                                                      std::string strideType) {
  for (auto platform : m_frameworks) {
    if ((frameworkName == platform->getRootNamespace()) ||
        frameworkName == platform->getFramework()) {
      std::string platformPath = platform->buildPlatformLibPath();
      auto defaultType = findDefaultDataTypeInPath(platformPath, strideType);
      return defaultType;
    }
  }
  return std::string();
}

std::vector<std::shared_ptr<DeclarationNode>>
StrideSystem::getFrameworkOperators(std::string frameworkName) {
  std::vector<std::shared_ptr<DeclarationNode>> operators;
  std::map<std::string, std::vector<ASTNode>> libObjects = getImportTrees();
  if (libObjects.find(frameworkName) != libObjects.end()) {
    for (auto node : libObjects[frameworkName]) {
      if (node->getNodeType() == AST::Declaration) {
        auto decl = std::static_pointer_cast<DeclarationNode>(node);
        if (decl->getObjectType() == "platformOperator") {
          operators.push_back(decl);
        }
      }
    }
  }
  return operators;
}

std::vector<std::string>
StrideSystem::getFrameworkAliasInherits(std::string frameworkAlias) {
  for (auto fw : m_frameworks) {
    if (fw->getRootNamespace() == frameworkAlias) {
      return fw->getInheritedList();
    }
  }
  return std::vector<std::string>{};
}

std::string StrideSystem::getDataType(ASTNode node, ASTNode tree) {
  for (auto child : tree->getChildren()) {
    if (child->getNodeType() == AST::Block) {
      std::string domainId = CodeAnalysis::getDomainIdentifier(node, {}, tree);
      auto frameworkName = CodeAnalysis::getFrameworkForDomain(domainId, tree);
      frameworkName = getFrameworkAlias(frameworkName);
      auto decl = ASTQuery::findDeclarationByName(
          ASTQuery::getNodeName(child), {}, tree, child->getNamespaceList());

      if (decl) {
        return CodeAnalysis::getDataTypeForDeclaration(decl, tree);
      } else {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ERROR, could not find declaration for node" << std::endl;
      }
    } else if (child->getNodeType() == AST::Declaration) {
      auto nodeDecl = std::static_pointer_cast<DeclarationNode>(child);
      return CodeAnalysis::getDataTypeForDeclaration(nodeDecl, tree);
    }
  }
  return std::string();
}

std::string StrideSystem::getFrameworkAlias(std::string frameworkName) {
  for (auto platform : m_frameworks) {
    if (frameworkName == platform->getFramework()) {
      return platform->getRootNamespace();
    }
  }
  return frameworkName;
}

std::string StrideSystem::getFrameworkFromAlias(std::string frameworkAlias) {
  for (auto platform : m_frameworks) {
    if (frameworkAlias == platform->getRootNamespace()) {
      return platform->getFramework();
    }
  }
  return frameworkAlias;
}

void StrideSystem::generateDomainConnections(ASTNode tree) {
  std::vector<ASTNode> newChildren;
  for (const auto &node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      auto stream = std::static_pointer_cast<StreamNode>(node);
      std::vector<std::shared_ptr<StreamNode>> newStreams;
      ASTNode previousNode;
      while (stream) {
        auto node = stream->getLeft();
        previousNode = node;
        auto previousDomainId = CodeAnalysis::getDomainIdentifier(
            CodeAnalysis::getNodeDomain(previousNode, {}, tree), {}, tree);

        ASTNode next;
        if (stream->getRight()->getNodeType() == AST::Stream) {
          next = std::static_pointer_cast<StreamNode>(stream->getRight())
                     ->getLeft();
        } else {
          next = stream->getRight();
        }
        auto nextDomainId = CodeAnalysis::getDomainIdentifier(
            CodeAnalysis::getNodeDomain(next, {}, tree), {}, tree);
        if (nextDomainId != previousDomainId) {
          std::cerr << "Domain change: " << previousDomainId << " -> "
                    << nextDomainId << std::endl;
          auto domainChangeNodes =
              getDomainChangeStreams(previousDomainId, nextDomainId);

          //
          auto nextInstance = CodeAnalysis::getInstance(next, {}, tree);
          if (nextInstance) {
            auto writes = nextInstance->getCompilerProperty("writes");
            if (writes) {
              auto newWrites = std::make_shared<ListNode>(__FILE__, __LINE__);
              for (auto w : writes->getChildren()) {
                auto writeDomain =
                    std::static_pointer_cast<BlockNode>(w)->getName();
                if (w->getScopeLevels() > 0) {
                  writeDomain = w->getScopeAt(0) + "::" + writeDomain;
                }
                if (writeDomain != previousDomainId) {
                  newWrites->addChild(w);
                }
              }
              nextInstance->setCompilerProperty("writes", newWrites);
            }
          }

          // FIXME currently only simple two member connector streams
          // supported
          if (domainChangeNodes.sourceStreams &&
              domainChangeNodes.destStreams) {
            stream->setRight(domainChangeNodes.sourceStreams->getChildren()[0]
                                 ->getChildren()[1]);
            stream->getRight()->setCompilerProperty("inputBlock",
                                                    stream->getLeft());
            auto previousFramework =
                CodeAnalysis::getFrameworkForDomain(previousDomainId, tree);
            // We should validate with provided connection framework
            auto nodeDecl = ASTQuery::findDeclarationByName(
                ASTQuery::getNodeName(stream->getRight()), {},
                domainChangeNodes.sourceImports, {},
                getFrameworkAlias(previousFramework));
            stream->getRight()->setCompilerProperty("declaration", nodeDecl);
            newStreams.push_back(stream);

            auto connectionNode =
                domainChangeNodes.destStreams->getChildren()[0]
                    ->getChildren()[0];
            connectionNode->setCompilerProperty("outputBlock", next);
            auto destFramework =
                CodeAnalysis::getFrameworkForDomain(nextDomainId, tree);
            // We should validate with provided connection framework
            nodeDecl = ASTQuery::findDeclarationByName(
                ASTQuery::getNodeName(connectionNode), {},
                domainChangeNodes.destImports, {},
                getFrameworkAlias(destFramework));
            connectionNode->setCompilerProperty("declaration", nodeDecl);

            // We need to remove the read domain from the reads metadata in
            // "next"

            auto connectionDomain =
                CodeAnalysis::getNodeDomain(connectionNode, {}, tree);
            nextInstance->appendToPropertyValue("writes", connectionDomain);

            std::shared_ptr<StreamNode> newStream =
                std::make_shared<StreamNode>(connectionNode, next, __FILE__,
                                             __LINE__);
            newStreams.push_back(newStream);
            auto srcImportNodes =
                domainChangeNodes.sourceImports->getChildren();
            newChildren.insert(newChildren.end(), srcImportNodes.begin(),
                               srcImportNodes.end());
            // FIXME a lot of redundant nodes are inserted here. Only insert if
            // not present
            auto destImportNodes = domainChangeNodes.destImports->getChildren();
            newChildren.insert(newChildren.end(), destImportNodes.begin(),
                               destImportNodes.end());
          } else {
          }
        }
        if (stream->getRight()->getNodeType() == AST::Stream) {
          stream = std::static_pointer_cast<StreamNode>(stream->getRight());
        } else {
          stream = nullptr;
        }
      }
      if (newStreams.size() == 0) { // no domain changes, keep stream as is
        newChildren.push_back(node);
      } else {
        newChildren.insert(newChildren.end(), newStreams.begin(),
                           newStreams.end());
      }

    } else {
      newChildren.push_back(node);
    }
  }
  tree->setChildren(newChildren);
}

void StrideSystem::injectResourceConfiguration(ASTNode tree) {
  std::vector<ASTNode> newChildren = tree->getChildren();

  std::vector<std::shared_ptr<DeclarationNode>> resourceConfigs;

  // Insert default configuration if not provided.
  for (auto node : tree->getChildren()) {
    if (node->getNodeType() == AST::Declaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "resourceConfiguration") {
        std::string framework;
        auto frameworkNode = decl->getCompilerProperty("framework");
        if (frameworkNode && frameworkNode->getNodeType() == AST::String) {
          framework = std::static_pointer_cast<ValueNode>(frameworkNode)
                          ->getStringValue();
        }
        bool configFound = false;
        for (auto config : m_systemConfig.resourceConfigurations[framework]) {
        }
        if (!configFound) {
          ASTNode propertiesList =
              std::make_shared<ListNode>(__FILE__, __LINE__);
          propertiesList->addChild(std::make_shared<PropertyNode>(
              "value", decl->getPropertyValue("default"), __FILE__, __LINE__));
          auto constDecl = std::make_shared<DeclarationNode>(
              decl->getName(), "constant", propertiesList, __FILE__, __LINE__);
          if (decl->getCompilerProperty("framework")) {
            constDecl->setCompilerProperty(
                "framework", decl->getCompilerProperty("framework"));
          }
          m_systemConfig.resourceConfigurations[framework].push_back(constDecl);
        }
      }
    }
  }

  for (auto frameworkConfig : m_systemConfig.resourceConfigurations) {
    for (auto configEntry : frameworkConfig.second) {
      // FIXME verify framework
      for (size_t i = 0; i < newChildren.size(); i++) {
        if (ASTQuery::getNodeName(newChildren[i]) == configEntry->getName()) {
          newChildren[i] = configEntry;
          break;
        }
      }
    }
  }
  tree->setChildren(newChildren);
}

ConnectionNodes
StrideSystem::getDomainChangeStreams(std::string previousDomainId,
                                     std::string nextDomainId) {
  ConnectionNodes domainChangeNodes;

  for (auto connector : m_connectionDefinitions) {
    auto sourceDomainsList = connector->getPropertyValue("sourceDomains");
    auto destDomainsList = connector->getPropertyValue("destinationDomains");
    auto sourceFramework = connector->getPropertyValue("sourceFramework");
    auto destFramework = connector->getPropertyValue("destinationFramework");
    std::string previousDomainFramework;
    auto separatorIndex = previousDomainId.find("::");
    if (separatorIndex != std::string::npos) {
      previousDomainFramework = previousDomainId.substr(0, separatorIndex);
      previousDomainId = previousDomainId.substr(separatorIndex + 2);
    }
    // TODO implement for domain instances (with an additional ":" separator at
    // the end
    std::string nextDomainFramework;
    separatorIndex = nextDomainId.find("::");
    if (separatorIndex != std::string::npos) {
      nextDomainFramework = nextDomainId.substr(0, separatorIndex);
      nextDomainId = nextDomainId.substr(separatorIndex + 2);
    }

    if (sourceDomainsList && destDomainsList) {
      bool sourceDomainsMatch = false;
      for (auto sourceDomain : sourceDomainsList->getChildren()) {
        if (sourceDomain->getNodeType() == AST::String) {
          if (std::static_pointer_cast<ValueNode>(sourceDomain)
                  ->getStringValue() == previousDomainId) {
            sourceDomainsMatch = true;
            break;
          }
        }
      }
      bool destDomainsMatch = false;
      for (auto destDomain : destDomainsList->getChildren()) {
        if (destDomain->getNodeType() == AST::String) {
          if (std::static_pointer_cast<ValueNode>(destDomain)
                  ->getStringValue() == nextDomainId) {
            destDomainsMatch = true;
            break;
          }
        }
      }
      if (destDomainsMatch && sourceDomainsMatch) {
        auto sourceStreams = connector->getPropertyValue("sourceStreams");
        auto destStreams = connector->getPropertyValue("destinationStreams");
        auto sourceImports = connector->getPropertyValue("sourceImports");
        auto destImports = connector->getPropertyValue("destinationImports");

        domainChangeNodes.sourceStreams = sourceStreams;

        domainChangeNodes.destStreams = destStreams;

        // Process source
        if (sourceImports) {
          for (auto import : sourceImports->getChildren()) {
            if (import->getNodeType() == AST::String) {
              auto importName =
                  std::static_pointer_cast<ValueNode>(import)->getStringValue();
              auto platformName =
                  connector->getPropertyValue("sourceFramework");
              assert(platformName->getNodeType() == AST::String);
              auto importTree = loadImportTree(
                  importName, "",
                  std::static_pointer_cast<ValueNode>(platformName)
                      ->getStringValue());

              auto builtinObjects = getImportTrees();
              ASTNode newTree = std::make_shared<AST>();

              for (auto node : importTree) {
                newTree->addChild(node);
              }
              for (auto node : importTree) {
                ASTFunctions::fillDefaultPropertiesForNode(node, importTree);
                ASTFunctions::insertRequiredObjectsForNode(node, builtinObjects,
                                                           newTree);
              }

              injectResourceConfiguration(newTree);
              domainChangeNodes.sourceImports = newTree;
            }
          }
        }

        //                    processStreamNode(static_pointer_cast<StreamNode>(sourceStreams->getChildren()[0])->getRight(),
        //                            previousNode, scopeStack, domainCode);

        //                    scopeStack.pop_back();

        // Process destination
        if (destImports) {
          for (auto import : destImports->getChildren()) {
            if (import->getNodeType() == AST::String) {
              auto importName =
                  std::static_pointer_cast<ValueNode>(import)->getStringValue();
              auto platformName =
                  connector->getPropertyValue("destinationFramework");
              assert(platformName->getNodeType() == AST::String);
              auto importTree = loadImportTree(
                  importName, "",
                  std::static_pointer_cast<ValueNode>(platformName)
                      ->getStringValue());
              auto builtinObjects = getImportTrees();
              ASTNode newTree = std::make_shared<AST>();

              for (auto node : importTree) {
                newTree->addChild(node);
              }
              for (auto child : importTree) {
                ASTFunctions::fillDefaultPropertiesForNode(child, importTree);
                ASTFunctions::insertRequiredObjectsForNode(
                    child, builtinObjects, newTree);
              }
              injectResourceConfiguration(newTree);
              domainChangeNodes.destImports = newTree;
              //                                    scopeStack.push_back({"",
              //                                    importTree->getChildren()});
            }
          }
        }

        //                    processStreamNode(static_pointer_cast<StreamNode>(destStreams->getChildren()[0])->getLeft(),
        //                            nullptr, scopeStack, domainCode);
        //                    previousNode =
        //                    static_pointer_cast<StreamNode>(destStreams->getChildren()[0])->getLeft();
      }
    }
  }
  return domainChangeNodes;
}

std::string StrideSystem::getCommonAncestorDomain(std::string domainId1,
                                                  std::string domainId2,
                                                  ASTNode tree) {

  std::function<std::vector<std::string>(std::string, std::string)> getParents =
      [&](std::string domainId,
          std::string framework) -> std::vector<std::string> {
    std::vector<std::string> parents;
    if (framework.size() == 0) {
      framework = CodeAnalysis::getFrameworkForDomain(domainId, tree);
    }
    auto domainDeclaration =
        CodeAnalysis::findDomainDeclaration(domainId, tree);
    parents.push_back(domainId);
    if (domainDeclaration) {
      auto parentDomain = domainDeclaration->getPropertyValue("parentDomain");
      if (parentDomain && parentDomain->getNodeType() != AST::None) {
        auto parentDomainCopy = parentDomain->deepCopy();
        parentDomainCopy->setRootScope(framework);
        //      auto parentDomainName =
        //      CodeValidator::streamMemberName(parentDomain); auto
        //      parentDomainDecl = CodeAnalysis::findDomainDeclaration(
        //          parentDomainName, framework, tree);
        auto parentDomainId =
            CodeAnalysis::getDomainIdentifier(parentDomainCopy, {}, tree);
        auto newParentDomains = getParents(parentDomainId, framework);
        parents.insert(parents.end(), newParentDomains.begin(),
                       newParentDomains.end());
      }
    }
    return parents;
  };

  auto parents1 = getParents(domainId1, "");
  auto parents2 = getParents(domainId2, "");

  for (auto d1 : parents1) {
    for (auto d2 : parents2) {
      if (d1 == d2) {
        return d1;
      }
    }
  }

  return "";
}

std::string StrideSystem::getParentDomain(std::string domainId, ASTNode tree) {
  std::string parentDomainId;
  std::string framework = CodeAnalysis::getFrameworkForDomain(domainId, tree);

  auto domainDeclaration = CodeAnalysis::findDomainDeclaration(domainId, tree);
  if (domainDeclaration) {
    auto parentDomain = domainDeclaration->getPropertyValue("parentDomain");
    if (parentDomain && parentDomain->getNodeType() != AST::None) {
      auto parentDomainCopy = parentDomain->deepCopy();
      parentDomainCopy->setRootScope(framework);
      parentDomainId =
          CodeAnalysis::getDomainIdentifier(parentDomainCopy, {}, tree);
    }
  }
  return parentDomainId;
}

std::shared_ptr<DeclarationNode>
StrideSystem::getFrameworkOperator(std::string platform, std::string leftType,
                                   std::string rightType, std::string type) {

  return nullptr;
}

void StrideSystem::installFramework(std::string frameworkName) {
  for (auto framework : m_frameworks) {
    if (framework->getFramework() == frameworkName) {
      auto configPath =
          framework->buildPlatformLibPath() + "/" + "Configuration.stride";
      ASTNode tree = AST::parseFile(configPath.c_str(), nullptr);
      if (tree) {
        for (auto node : tree->getChildren()) {
          if (node->getNodeType() == AST::Declaration) {
            auto decl = std::static_pointer_cast<DeclarationNode>(node);
            if (decl->getObjectType() == "_frameworkConfiguration") {
              auto installActions = decl->getPropertyValue("installActions");
              if (installActions) {
                for (auto action : installActions->getChildren()) {
                  if (action->getNodeType() == AST::Declaration) {
                    auto commandNode =
                        std::static_pointer_cast<DeclarationNode>(action)
                            ->getPropertyValue("command");
                    if (commandNode &&
                        commandNode->getNodeType() == AST::String) {
                      auto command =
                          std::static_pointer_cast<ValueNode>(commandNode)
                              ->getStringValue();
                      auto ret = system(command.c_str());
                      if (ret != 0) {
                        std::cerr << "Error executing " << command << std::endl;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

std::string StrideSystem::makeProject(std::string fileName) {
  auto dirName = std::filesystem::canonical(fileName).string() + "_Products";
  if (!std::filesystem::exists(dirName)) {
    if (!std::filesystem::create_directories(dirName)) {
      return std::string();
    }
  }
  return dirName;
}

std::shared_ptr<DeclarationNode>
StrideSystem::findDataTypeInPath(std::string path, std::string strideDataType) {
  auto nodes = ASTFunctions::loadAllInDirectory(path);
  for (ASTNode node : nodes) {
    if (node->getNodeType() == AST::Declaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "frameworkDataType") {
        if (decl->getName() == strideDataType) {
          return decl;
        }
      } else if (decl->getObjectType() == "_frameworkDescription") {
        auto importNameNode = decl->getPropertyValue("inherits");
        auto importVersionNode = decl->getPropertyValue("inheritsVersion");
        if (importNameNode && importVersionNode &&
            importNameNode->getNodeType() == AST::String &&
            importVersionNode->getNodeType() == AST::String) {
          auto importName = std::static_pointer_cast<ValueNode>(importNameNode)
                                ->getStringValue();
          auto importVersion =
              std::static_pointer_cast<ValueNode>(importVersionNode)
                  ->getStringValue();
          auto inheritedPath = m_strideRoot + "/frameworks/" + importName +
                               "/" + importVersion + "/platformlib";
          auto dataTypeDecl = findDataTypeInPath(inheritedPath, strideDataType);
          if (dataTypeDecl) {
            return dataTypeDecl;
          }
        }
      }
    }
  }
  return nullptr;
}

std::string StrideSystem::findDefaultDataTypeInPath(std::string path,
                                                    std::string strideType) {
  auto nodes = ASTFunctions::loadAllInDirectory(path);
  for (ASTNode node : nodes) {
    if (node->getNodeType() == AST::Declaration) {
      auto decl = std::static_pointer_cast<DeclarationNode>(node);
      if (decl->getObjectType() == "_frameworkDescription" &&
          strideType.size() == 0) {
        // If no stride type provided get default data type
        auto defaultNode = decl->getPropertyValue("defaultDataType");
        if (defaultNode) {
          if (defaultNode->getNodeType() == AST::String) {
            return std::static_pointer_cast<ValueNode>(defaultNode)
                ->getStringValue();
          } else if (defaultNode->getNodeType() == AST::Block) {
            return ASTQuery::getNodeName(defaultNode);
          }
        }
      } else if (decl->getObjectType() == "strideTypeDefaultDataType") {
        auto strideTypeNode = decl->getPropertyValue("strideType");
        if (strideTypeNode && strideTypeNode->getNodeType() == AST::String) {
          auto strideTypeName =
              std::static_pointer_cast<ValueNode>(strideTypeNode)
                  ->getStringValue();
          if (strideTypeName == strideType) {
            auto frameworkTypeNode = decl->getPropertyValue("frameworkType");
            if (frameworkTypeNode) {
              return ASTQuery::getNodeName(frameworkTypeNode);
            }
          }
        }
      } else if (decl->getObjectType() == "_frameworkDescription") {
        auto importNameNode = decl->getPropertyValue("inherits");
        auto importVersionNode = decl->getPropertyValue("inheritsVersion");
        if (importNameNode && importVersionNode &&
            importNameNode->getNodeType() == AST::String &&
            importVersionNode->getNodeType() == AST::String) {
          auto importName = std::static_pointer_cast<ValueNode>(importNameNode)
                                ->getStringValue();
          auto importVersion =
              std::static_pointer_cast<ValueNode>(importVersionNode)
                  ->getStringValue();
          auto inheritedPath = m_strideRoot + "/frameworks/" + importName +
                               "/" + importVersion + "/platformlib";
          auto dataTypeDecl =
              findDefaultDataTypeInPath(inheritedPath, strideType);
          if (dataTypeDecl.size() > 0) {
            return dataTypeDecl;
          }
        }
      }
    }
  }
  return std::string();
}
