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

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "stridesystem.hpp"

#include "coderesolver.h"
#include "codevalidator.h"
#include "declarationnode.h"
#include "propertynode.h"
#include "pythonproject.h"
#include "toolmanager.hpp"

StrideSystem::StrideSystem(QString strideRoot, QString systemName,
                           int majorVersion, int minorVersion,
                           std::vector<std::shared_ptr<ImportNode>> importList)
    : m_strideRoot(strideRoot), m_systemName(systemName),
      m_majorVersion(majorVersion), m_minorVersion(minorVersion) {
  QString versionString =
      QString("%1.%2").arg(m_majorVersion).arg(m_minorVersion);
  m_systemPath =
      QDir(strideRoot + QDir::separator() + "systems" + QDir::separator() +
           systemName + QDir::separator() + versionString)
          .absolutePath();
  QString systemFile = m_systemPath + QDir::separator() + "System.stride";

  m_library.initializeLibrary(strideRoot);

  for (auto importNode : importList) {
    //    m_library.loadImportTree(QString::fromStdString(importNode->importName()),
    //                             QString::fromStdString(importNode->importAlias()),
    //                             {});
    m_importList[QString::fromStdString(importNode->importName())] =
        QString::fromStdString(importNode->importAlias());
  }

  if (QFile::exists(systemFile)) {
    ASTNode systemTree =
        AST::parseFile(systemFile.toStdString().c_str(), nullptr);
    if (systemTree) {
      parseSystemTree(systemTree);

      // Iterate through platforms reading them.
      // TODO Should optimize this to not reread platform if already done.

      // Load testing trees
      for (auto framework : m_frameworks) {
        QStringList nameFilters;
        nameFilters.push_back("*.stride");
        string platformPath =
            framework->buildTestingLibPath(m_strideRoot.toStdString());
        QFileInfoList libraryFiles = QDir(QString::fromStdString(platformPath))
                                         .entryInfoList(nameFilters);
        for (auto fileInfo : libraryFiles) {
          ASTNode tree = AST::parseFile(
              fileInfo.absoluteFilePath().toLocal8Bit().data(), nullptr);
          if (tree) {
            for (auto node : tree->getChildren()) {
              node->setCompilerProperty(
                  "framework", std::make_shared<ValueNode>(
                                   getFrameworkAlias(framework->getFramework()),
                                   __FILE__, __LINE__));
            }
            framework->addTestingTree(fileInfo.baseName().toStdString(), tree);
          } else {
            vector<LangError> errors = AST::getParseErrors();
            foreach (LangError error, errors) {
              qDebug() << QString::fromStdString(error.getErrorText());
            }
            continue;
          }
        }
      }
      //                m_platformPath = fullPath;
      //                m_api = PythonTools;
      //                m_types = getPlatformTypeNames();
    } else {
      qDebug() << "Error parsing system tree in:" << systemFile;
    }
  } else {
    qDebug() << "System file not found:" << systemFile;
  }

  //    if (m_api == NullPlatform) {
  //        qDebug() << "Platform not found!";
  //    }
}

StrideSystem::~StrideSystem() {}

void StrideSystem::parseSystemTree(ASTNode systemTree, ASTNode configuration) {
  vector<pair<string, string>> usedPlatformNames;
  vector<map<string, string>> platformDefinitions;
  //    vector<string> platformDefinitionNames;
  vector<AST *> connectionDefinitions; // TODO process connection definitions

  for (ASTNode systemNode : systemTree->getChildren()) {
    if (systemNode->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> declaration =
          static_pointer_cast<DeclarationNode>(systemNode);
      if (declaration->getObjectType() == "platform") {
        map<string, string> definition;
        vector<std::shared_ptr<PropertyNode>> properties =
            declaration->getProperties();
        for (std::shared_ptr<PropertyNode> prop : properties) {
          if (prop->getValue()->getNodeType() == AST::String) {
            definition[prop->getName()] =
                static_cast<ValueNode *>(prop->getValue().get())
                    ->getStringValue();
          }
        }
        platformDefinitions.push_back(definition);
        //                platformDefinitionNames.push_back(declaration->getName());
        m_platformDefinitions.push_back(
            static_pointer_cast<DeclarationNode>(declaration));
      } else if (declaration->getObjectType() == "connection") {
        m_connectionDefinitions.push_back(declaration);
      } else if (declaration->getObjectType() == "system") {
        ASTNode platforms = declaration->getPropertyValue("platforms");
        if (!platforms) {
          qDebug() << "ERROR: platforms port not found in system definition";
        } else if (platforms->getNodeType() == AST::List) {
          ListNode *platformsList = static_cast<ListNode *>(platforms.get());
          for (ASTNode platformName : platformsList->getChildren()) {
            Q_ASSERT(platformName->getNodeType() == AST::Block);
            auto platformSpecBlock =
                static_pointer_cast<BlockNode>(platformName);
            auto platformSpec = CodeValidator::findDeclaration(
                platformSpecBlock->getName(), {}, systemTree);
            if (platformSpec) {
              auto platformBlock = platformSpec->getPropertyValue("framework");
              auto rootNamespaceBlock =
                  platformSpec->getPropertyValue("rootNamespace");
              if (rootNamespaceBlock) {
                if (platformBlock->getNodeType() == AST::String &&
                    rootNamespaceBlock->getNodeType() == AST::String) {
                  usedPlatformNames.push_back(
                      {static_pointer_cast<ValueNode>(platformBlock)
                           ->getStringValue(),
                       static_pointer_cast<ValueNode>(rootNamespaceBlock)
                           ->getStringValue()});
                } else {
                  qDebug() << "ERROR: Unexpected types for platform spec "
                           << QString::fromStdString(
                                  platformSpecBlock->getName());
                }
              } else if (platformBlock->getNodeType() == AST::String) {
                usedPlatformNames.push_back(
                    {static_pointer_cast<ValueNode>(platformBlock)
                         ->getStringValue(),
                     ""});
              }

            } else {
              qDebug() << "ERROR: could not find platform spec "
                       << QString::fromStdString(platformSpecBlock->getName());
            }
          }
        } else {
          qDebug() << "ERROR: Expected list for platforms in system definition";
        }
      } /*else {
          qDebug() << "ERROR: Unknown system declaration type";
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
            static_pointer_cast<DeclarationNode>(systemNode);
        if (declaration->getObjectType() == "platform") {
          if (sourcePlatform->getNodeType() == AST::Block) {
            if (declaration->getName() ==
                static_pointer_cast<BlockNode>(sourcePlatform)->getName()) {
              connection->replacePropertyValue(
                  "sourceFramework",
                  declaration->getPropertyValue("framework"));
              Q_ASSERT(
                  declaration->getPropertyValue("framework")->getNodeType() ==
                  AST::String);
            }
          } else if (sourcePlatform->getNodeType() == AST::String) {
            qDebug() << "ERROR: expenting block or string in source platform "
                        "for connection "
                     << QString::fromStdString(connection->getName());
          }
          if (destPlatform->getNodeType() == AST::Block) {
            if (declaration->getName() ==
                static_pointer_cast<BlockNode>(destPlatform)->getName()) {
              connection->replacePropertyValue(
                  "destinationFramework",
                  declaration->getPropertyValue("framework"));
              Q_ASSERT(
                  declaration->getPropertyValue("framework")->getNodeType() ==
                  AST::String);
            }
          } else if (destPlatform->getNodeType() == AST::String) {
            qDebug() << "ERROR: expenting block or string in source platform "
                        "for connection "
                     << QString::fromStdString(connection->getName());
          }
        }
      }
    }
  }

  // Now connect platforms referenced in system with defined platforms
  for (auto usedPlatformName : usedPlatformNames) {
    for (size_t i = 0; i < platformDefinitions.size(); i++) {
      map<string, string> &definition = platformDefinitions.at(i);
      //            string &name = platformDefinitionNames.at(i);
      if (definition["framework"] == usedPlatformName.first) {
        string framework, framworkVersion, hardware, hardwareVersion,
            rootNamespace;
        framework = definition["framework"];

        std::shared_ptr<StrideFramework> newPlatform =
            std::make_shared<StrideFramework>(
                m_strideRoot.toStdString(), definition["framework"],
                definition["frameworkVersion"], definition["hardware"],
                definition["hardwareVersion"], definition["rootNamespace"]);
        m_frameworks.push_back(newPlatform);
      } else {
        // TODO add error
      }
    }
  }
}

QStringList StrideSystem::getErrors() { return m_errors; }

QStringList StrideSystem::getWarnings() { return m_warnings; }

QStringList StrideSystem::getPlatformTypeNames() {
  QStringList typeNames;
  auto libObjects = getBuiltinObjectsReference();
  for (auto libNamespace : libObjects) {
    for (auto node : libNamespace.second) {
      if (node->getNodeType() == AST::Declaration) {
        std::shared_ptr<DeclarationNode> block =
            static_pointer_cast<DeclarationNode>(node);
        if (block->getObjectType() == "type" ||
            block->getObjectType() == "platformModule" ||
            block->getObjectType() == "platformBlock") {
          ValueNode *name = static_cast<ValueNode *>(
              block->getPropertyValue("typeName").get());
          if (name && name->getNodeType() == AST::String) {
            typeNames << QString::fromStdString(name->getStringValue());
          }
        }
      }
    }
  }
  return typeNames;
}

QStringList StrideSystem::getFunctionNames() {
  QStringList funcNames;

  map<string, vector<ASTNode>> refObjects = getBuiltinObjectsReference();

  for (auto it = refObjects.begin(); it != refObjects.end(); it++) {

    for (ASTNode node : it->second) {
      if (node->getNodeType() == AST::Declaration) {
        DeclarationNode *block = static_cast<DeclarationNode *>(node.get());
        if (block->getObjectType() == "module") {
          if (it->first.size() > 0) {
            funcNames << QString::fromStdString(it->first) + ":" +
                             QString::fromStdString(block->getName());
          } else {
            funcNames << QString::fromStdString(block->getName());
          }
        }
      }
    }
  }
  return funcNames;
}

vector<Builder *> StrideSystem::createBuilders(QString fileName, ASTNode tree) {

  vector<Builder *> builders;
  QString projectDir = makeProject(fileName);
  if (projectDir.isEmpty()) {
    qDebug() << "Error creating project path";
    return builders;
  }
  for (auto platform : m_frameworks) {
    auto usedFrameworks = CodeValidator::getUsedFrameworks(tree);
    if ((usedFrameworks.size() == 0) ||
        (std::find(usedFrameworks.begin(), usedFrameworks.end(),
                   getFrameworkAlias(platform->getFramework())) !=
         usedFrameworks.end())) {
      if (platform->getAPI() == StrideFramework::PythonTools) {
        QString pythonExec = "python";
        Builder *builder = new PythonProject(
            QString::fromStdString(platform->getFramework()),
            QString::fromStdString(
                platform->buildPlatformPath(m_strideRoot.toStdString())),
            m_strideRoot, projectDir, pythonExec);
        if (builder) {
          if (builder->isValid()) {
            builders.push_back(builder);
          } else {
            delete builder;
          }
        }
      } else if (platform->getAPI() == StrideFramework::PluginPlatform) {
        auto pluginList = QDir(m_strideRoot + "/plugins")
                              .entryList(QDir::NoDotAndDotDot | QDir::Files);
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
          qDebug() << "Error getting plugin details";
        }
        vector<string> validDomains;

        for (auto plugin : pluginList) {
          //                    qDebug() << plugin;

          QLibrary pluginLibrary(m_strideRoot + "/plugins/" + plugin);
          if (pluginLibrary.load()) {
            char name[STRIDE_PLUGIN_MAX_STR_LEN];
            int versionMajor = -1;
            int versionMinor = -1;
            auto nameFunc =
                (platform_name_t)pluginLibrary.resolve("platform_name");
            if (nameFunc) {
              nameFunc(name);
            }
            if (strncmp(name, pluginName.c_str(), STRIDE_PLUGIN_MAX_STR_LEN) ==
                0) {
              auto versionMajorFunc =
                  (platform_version_major_t)pluginLibrary.resolve(
                      "platform_version_major");
              if (versionMajorFunc) {
                versionMajor = versionMajorFunc();
              }
              auto versionMinorFunc =
                  (platform_version_minor_t)pluginLibrary.resolve(
                      "platform_version_minor");
              if (versionMinorFunc) {
                versionMinor = versionMinorFunc();
              }
              if (pluginMajorVersion == versionMajor &&
                  pluginMinorVersion == versionMinor) {
                //                                qDebug() << "Code generator
                //                                plugin found! " <<
                //                                QString::fromStdString(pluginName);
                create_object_t create =
                    (create_object_t)pluginLibrary.resolve("create_object");
                if (create) {
                  Builder *builder =
                      create(QString::fromStdString(platform->buildPlatformPath(
                                 m_strideRoot.toStdString())),
                             m_strideRoot, projectDir);
                  if (builder) {
                    builder->m_frameworkName =
                        QString::fromStdString(platform->getFramework());
                    builders.push_back(builder);
                  }
                }
              }
            }
          }
        }

        //                pluginLibrary.unload();
      }
    }
  }
  return builders;
}

ASTNode StrideSystem::getPlatformDomain(string namespaceName) {
  ASTNode platformDomain;
  map<string, vector<ASTNode>> libObjects = getBuiltinObjectsReference();
  if (libObjects.find(namespaceName) == libObjects.end()) { // Invalid namespace
    assert(0 == 1);
    return platformDomain;
  }
  for (ASTNode object : libObjects[namespaceName]) {
    if (object->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          static_pointer_cast<DeclarationNode>(object);
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

QMap<QString, QString>
StrideSystem::getFrameworkTools(std::string namespaceName) {
  QMap<QString, QString> tools;

  ToolManager toolManager(m_strideRoot.toStdString());

  map<string, vector<ASTNode>> libObjects = getBuiltinObjectsReference();
  if (libObjects.find(namespaceName) == libObjects.end()) { // Invalid namespace
    assert(0 == 1);
    return QMap<QString, QString>();
  }
  namespaceName = ""; // Hack! Everything is currently being put into root...
  for (ASTNode object : libObjects[namespaceName]) {
    if (object->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          static_pointer_cast<DeclarationNode>(object);
      if (decl->getObjectType() == "toolRequirement") {
        auto toolInstanceNode = decl->getPropertyValue("toolInstance");
        if (toolInstanceNode && toolInstanceNode->getNodeType() == AST::Block) {
          auto toolInstance =
              static_pointer_cast<BlockNode>(toolInstanceNode)->getName();
          if (toolManager.localTools.find(toolInstance) !=
              toolManager.localTools.end()) {

            tools[QString::fromStdString(toolInstance)] =
                QString::fromStdString(toolManager.localTools[toolInstance]);
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

QMap<QString, QString>
StrideSystem::getFrameworkPaths(std::string namespaceName) {
  QMap<QString, QString> paths;

  ToolManager toolManager(m_strideRoot.toStdString());

  map<string, vector<ASTNode>> libObjects = getBuiltinObjectsReference();
  if (libObjects.find(namespaceName) == libObjects.end()) { // Invalid namespace
    assert(0 == 1);
    return QMap<QString, QString>();
  }
  for (ASTNode object : libObjects[namespaceName]) {
    if (object->getNodeType() == AST::Declaration) {
      std::shared_ptr<DeclarationNode> decl =
          static_pointer_cast<DeclarationNode>(object);
      if (decl->getObjectType() == "pathRequirement") {
        auto toolInstanceNode = decl->getPropertyValue("pathInstance");
        if (toolInstanceNode && toolInstanceNode->getNodeType() == AST::Block) {
          auto pathInstance =
              static_pointer_cast<BlockNode>(toolInstanceNode)->getName();
          if (toolManager.localPaths.find(pathInstance) !=
              toolManager.localPaths.end()) {

            paths[QString::fromStdString(pathInstance)] =
                QString::fromStdString(toolManager.localPaths[pathInstance]);
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

string StrideSystem::substituteTokens(string namespaceName, string text) {
  auto paths = getFrameworkPaths(namespaceName);

  for (auto mapEntry : paths.keys()) {
    size_t index = 0;
    while (true) {
      index = text.find("%" + mapEntry.toStdString() + "%", index);
      if (index == std::string::npos) {
        break;
      }
      text.replace(index, mapEntry.size() + 2, paths[mapEntry].toStdString());
      index += mapEntry.size() + 2;
    }
  }
  size_t index = 0;
  while (index < text.size() && text.find("%", index) != std::string::npos) {
    index = text.find("%", index);
    size_t endIndex = text.find("%", index + 1);
    if (endIndex != std::string::npos) {
      auto tokenName = text.substr(index + 1, endIndex - index - 1);
      if (m_systemConfig.platformConfigurations["all"].find(tokenName) !=
          m_systemConfig.platformConfigurations["all"].end()) {
        text.replace(index, endIndex - index + 1,
                     m_systemConfig.platformConfigurations["all"][tokenName]
                         .toString()
                         .toStdString());
      }

      index = endIndex + 1;
    } else {
      index = endIndex;
    }
  }

  return text;
}

vector<string> StrideSystem::getFrameworkNames() {
  vector<string> names;
  for (auto platform : m_frameworks) {
    names.push_back(platform->getFramework());
  }
  return names;
}

map<string, vector<ASTNode>> StrideSystem::getBuiltinObjectsReference() {
  map<string, vector<ASTNode>> objects;
  objects[""] = vector<ASTNode>();
  for (auto platform : m_frameworks) {
    vector<ASTNode> platformObjects = platform->getPlatformObjectsReference();

    // Add all platform objects with their namespace name

    //        std::string platformName = platform->getFramework();
    //        if (platformEntry.first != "") {
    //            platformName = platformEntry.first;
    //        }
    //        objects[platformName] = platformObjects;
    // Then put first platform's objects in default namespace
    objects[platform->getFramework()] = vector<ASTNode>();
    std::shared_ptr<StrideFramework> rootPlatform;
    for (auto node : platformObjects) {
      objects[platform->getFramework()].push_back(node);
      if (node->getNodeType() == AST::Declaration ||
          node->getNodeType() == AST::BundleDeclaration) {
        //        auto validScopes = node->getCompilerProperty("namespaceTree");
        //        if (validScopes) {
        //          assert(validScopes->getNodeType() == AST::List);
        //          bool rootFound = false;
        //          bool relativeScopeFound = false;
        //          for (auto child : validScopes->getChildren()) {
        //            std::shared_ptr<ValueNode> stringValue =
        //                std::static_pointer_cast<ValueNode>(child);
        //            assert(child->getNodeType() == AST::String);
        //            if (stringValue->getStringValue() == "::") {
        //              rootFound = true;
        //            } else if (stringValue->getStringValue() == "") {
        //              relativeScopeFound = true;
        //            }
        //          }
        //          //                    if (!rootFound) {
        //          //
        //          validScopes->addChild(std::make_shared<ValueNode>(string("::"),
        //          //                        __FILE__, __LINE__));
        //          //                    }
        //          if (!relativeScopeFound) {
        //            validScopes->addChild(
        //                std::make_shared<ValueNode>(string(""), __FILE__,
        //                __LINE__));
        //          }
        //        } else {
        //          auto newList = std::make_shared<ListNode>(
        //              std::make_shared<ValueNode>(string("::"), __FILE__,
        //              __LINE__),
        //              __FILE__, __LINE__);
        //          newList->addChild(
        //              std::make_shared<ValueNode>(string(""), __FILE__,
        //              __LINE__));
        //          node->setCompilerProperty("namespaceTree", newList);
        //        }
        if (platform->getRootNamespace() == "") {
          objects[""].push_back(node);
        }
      }
    }
  }

  // finally add library objects.
  for (auto libNamespace : m_library.getLibraryMembers()) {
    if (objects.find(libNamespace.first) == objects.end()) {
      objects[libNamespace.first] = std::vector<ASTNode>();
    }
    objects[libNamespace.first].insert(objects[libNamespace.first].end(),
                                       libNamespace.second.begin(),
                                       libNamespace.second.end());
  }
  for (std::shared_ptr<DeclarationNode> decl : m_platformDefinitions) {
    objects[""].push_back(decl);
  }
  return objects;
}

vector<ASTNode> StrideSystem::getOptionTrees(std::string systemPath) {
  vector<ASTNode> optionTrees;
  QStringList nameFilters;
  nameFilters.push_back("*.stride");
  QString optionPath =
      QString::fromStdString(systemPath) + QDir::separator() + "options";
  QFileInfoList optionFiles = QDir(optionPath).entryInfoList(nameFilters);
  for (auto fileInfo : optionFiles) {
    ASTNode optionTree = AST::parseFile(
        fileInfo.absoluteFilePath().toStdString().c_str(), nullptr);
    if (optionTree) {
      ASTNode finalTree = std::make_shared<AST>();
      for (auto child : optionTree->getChildren()) {
        bool osMatch = true;
        if (child->getNodeType() == AST::Declaration) {
          auto decl = static_pointer_cast<DeclarationNode>(child);
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
      qDebug() << "Error parsing option file: " << fileInfo.absolutePath();
    }
  }
  return optionTrees;
}

std::vector<std::shared_ptr<DeclarationNode>>
StrideSystem::getFrameworkSynchronization(std::string frameworkName) {
  QStringList nameFilters;
  std::vector<std::shared_ptr<DeclarationNode>> syncNodes;
  nameFilters.push_back("*.stride");
  for (auto platform : m_frameworks) {
    if ((frameworkName == platform->getRootNamespace()) ||
        frameworkName == platform->getFramework()) {
      string platformPath =
          platform->buildPlatformLibPath(m_strideRoot.toStdString());
      QStringList libraryFiles =
          QDir(QString::fromStdString(platformPath)).entryList(nameFilters);

      for (QString file : libraryFiles) {
        QString fileName = QDir::cleanPath(
            QString::fromStdString(platformPath) + QDir::separator() + file);
        auto newTree = AST::parseFile(fileName.toLocal8Bit().data(), nullptr);
        if (newTree) {
          for (ASTNode node : newTree->getChildren()) {
            if (node->getNodeType() == AST::Declaration) {
              auto decl = static_pointer_cast<DeclarationNode>(node);
              if (decl->getObjectType() == "synchronization") {

                syncNodes.push_back(decl);
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
  QStringList nameFilters;
  nameFilters.push_back("*.stride");
  for (auto platform : m_frameworks) {
    if ((frameworkName == platform->getRootNamespace()) ||
        frameworkName == platform->getFramework()) {
      string platformPath =
          platform->buildPlatformLibPath(m_strideRoot.toStdString());
      QStringList libraryFiles =
          QDir(QString::fromStdString(platformPath)).entryList(nameFilters);

      for (QString file : libraryFiles) {
        QString fileName = QDir::cleanPath(
            QString::fromStdString(platformPath) + QDir::separator() + file);
        auto newTree = AST::parseFile(fileName.toLocal8Bit().data(), nullptr);
        if (newTree) {
          for (ASTNode node : newTree->getChildren()) {
            if (node->getNodeType() == AST::Declaration) {
              auto decl = static_pointer_cast<DeclarationNode>(node);
              if (decl->getObjectType() == "frameworkDataType") {
                if (decl->getName() == strideDataType) {
                  return decl;
                }
              }
            }
          }
        }
      }
    }
  }
  return nullptr;
}

string StrideSystem::getFrameworkDefaultDataType(string frameworkName,
                                                 string strideType) {
  QStringList nameFilters;
  nameFilters.push_back("*.stride");
  for (auto platform : m_frameworks) {
    if ((frameworkName == platform->getRootNamespace()) ||
        frameworkName == platform->getFramework()) {
      string platformPath =
          platform->buildPlatformLibPath(m_strideRoot.toStdString());
      QStringList libraryFiles =
          QDir(QString::fromStdString(platformPath)).entryList(nameFilters);

      for (QString file : libraryFiles) {
        QString fileName = QDir::cleanPath(
            QString::fromStdString(platformPath) + QDir::separator() + file);
        auto newTree = AST::parseFile(fileName.toLocal8Bit().data(), nullptr);
        if (newTree) {
          for (ASTNode node : newTree->getChildren()) {
            if (node->getNodeType() == AST::Declaration) {
              auto decl = static_pointer_cast<DeclarationNode>(node);
              if (decl->getObjectType() == "_frameworkDescription" &&
                  strideType.size() == 0) {
                auto defaultNode = decl->getPropertyValue("defaultDataType");
                if (defaultNode) {
                  if (defaultNode->getNodeType() == AST::String) {
                    return static_pointer_cast<ValueNode>(defaultNode)
                        ->getStringValue();
                  } else if (defaultNode->getNodeType() == AST::Block) {
                    return CodeValidator::streamMemberName(defaultNode);
                  }
                }
              } else if (decl->getObjectType() == "strideTypeDefaultDataType") {
                auto strideTypeNode = decl->getPropertyValue("strideType");
                if (strideTypeNode &&
                    strideTypeNode->getNodeType() == AST::String) {
                  auto strideTypeName =
                      static_pointer_cast<ValueNode>(strideTypeNode)
                          ->getStringValue();
                  if (strideTypeName == strideType) {
                    auto frameworkTypeNode =
                        decl->getPropertyValue("frameworkType");
                    if (frameworkTypeNode) {
                      return CodeValidator::streamMemberName(frameworkTypeNode);
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
  return std::string();
}

std::vector<std::shared_ptr<DeclarationNode>>
StrideSystem::getFrameworkOperators(string frameworkName) {
  std::vector<std::shared_ptr<DeclarationNode>> operators;
  map<string, vector<ASTNode>> libObjects = getBuiltinObjectsReference();
  if (libObjects.find(frameworkName) != libObjects.end()) {
    for (auto node : libObjects[frameworkName]) {
      if (node->getNodeType() == AST::Declaration) {
        auto decl = static_pointer_cast<DeclarationNode>(node);
        if (decl->getObjectType() == "platformOperator") {
          operators.push_back(decl);
        }
      }
    }
  }
  return operators;
}

string StrideSystem::getDataType(ASTNode node, ASTNode tree) {
  for (auto child : tree->getChildren()) {
    if (child->getNodeType() == AST::Block) {
      std::string domainId = CodeValidator::getDomainIdentifier(node, {}, tree);
      auto frameworkName = CodeValidator::getFrameworkForDomain(domainId, tree);
      frameworkName = getFrameworkAlias(frameworkName);
      auto decl =
          CodeValidator::findDeclaration(CodeValidator::streamMemberName(child),
                                         {}, tree, child->getNamespaceList());

      if (decl) {
        return CodeValidator::getDataTypeForDeclaration(decl, tree);
      } else {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ERROR, could not find declaration for node" << std::endl;
      }
    } else if (child->getNodeType() == AST::Declaration) {
      auto nodeDecl = static_pointer_cast<DeclarationNode>(child);
      return CodeValidator::getDataTypeForDeclaration(nodeDecl, tree);
    }
  }
  return std::string();
}

ASTNode StrideSystem::loadImportTree(std::string importName,
                                     std::string importAs,
                                     std::string platformName) {
  ASTNode tree = std::make_shared<AST>();
  QStringList nameFilters;
  nameFilters.push_back("*.stride");

  // If platform name is not provided, then import from everywhere (library and
  // framework)
  // TODO also try to load from current file directory
  if (platformName.size() == 0) {
    tree = m_library.loadImportTree(QString::fromStdString(importName),
                                    QString::fromStdString(importAs),
                                    QStringList());
  }

  for (auto platform : m_frameworks) {
    if (platformName.size() == 0 ||
        ((platformName == platform->getRootNamespace()) ||
         platformName == platform->getFramework())) {
      string platformPath =
          platform->buildPlatformLibPath(m_strideRoot.toStdString());
      string platformImportName = platformName;
      platformImportName = platform->getRootNamespace();
      string includeSubPath = platformPath + "/" + importName;
      QStringList libraryFiles =
          QDir(QString::fromStdString(includeSubPath)).entryList(nameFilters);
      //      if (QFile::exists(QString::fromStdString(platformPath) + "/" +
      //                        importName + ".stride")) {
      //        libraryFiles << "../" + importName + ".stride";
      //      }
      for (QString file : libraryFiles) {
        QString fileName = QDir::cleanPath(
            QString::fromStdString(includeSubPath) + QDir::separator() + file);
        auto newTree = AST::parseFile(fileName.toLocal8Bit().data(), nullptr);
        if (newTree) {
          for (ASTNode node : newTree->getChildren()) {
            //              node->setCompilerProperty(
            //                  "framework",
            //                  std::make_shared<ValueNode>(platformName.toStdString(),
            //                                              __FILE__,
            //                                              __LINE__));
            // FIXME check if we are bashing an existing name in the tree.

            //            size_t indexExtension =
            //            file.toStdString().find(".stride"); string scopeName
            //            = file.toStdString().substr(0, indexExtension);

            if (importAs.size() > 0) {
              node->appendToPropertyValue(
                  "namespaceTree",
                  std::make_shared<ValueNode>(importAs, __FILE__, __LINE__));
            }
            node->setCompilerProperty(
                "framework", std::make_shared<ValueNode>(platformImportName,
                                                         __FILE__, __LINE__));
            tree->addChild(node);
          }
        } else {
          qDebug() << "ERROR importing tree";
          vector<LangError> errors = AST::getParseErrors();
          for (LangError error : errors) {
            qDebug() << QString::fromStdString(error.getErrorText());
          }
          continue;
        }
      }
    }
  }

  if (tree && tree->getChildren().size() > 0) {
    return tree;
  }

  return nullptr;
}

string StrideSystem::getFrameworkAlias(string frameworkName) {
  for (auto platform : m_frameworks) {
    if (frameworkName == platform->getFramework()) {
      return platform->getRootNamespace();
    }
  }
  return frameworkName;
}

string StrideSystem::getFrameworkFromAlias(string frameworkAlias) {
  for (auto platform : m_frameworks) {
    if (frameworkAlias == platform->getRootNamespace()) {
      return platform->getFramework();
    }
  }
  return frameworkAlias;
}

void StrideSystem::generateDomainConnections(ASTNode tree) {
  std::vector<ASTNode> newChildren;
  for (auto node : tree->getChildren()) {
    if (node->getNodeType() == AST::Stream) {
      auto stream = static_pointer_cast<StreamNode>(node);
      std::vector<std::shared_ptr<StreamNode>> newStreams;
      ASTNode previousNode;
      while (stream) {
        auto node = stream->getLeft();
        previousNode = node;
        auto previousDomainId = CodeValidator::getDomainIdentifier(
            CodeValidator::getNodeDomain(previousNode, {}, tree), {}, tree);

        ASTNode next;
        if (stream->getRight()->getNodeType() == AST::Stream) {
          next = static_pointer_cast<StreamNode>(stream->getRight())->getLeft();
        } else {
          next = stream->getRight();
        }
        auto nextDomainId = CodeValidator::getDomainIdentifier(
            CodeValidator::getNodeDomain(next, {}, tree), {}, tree);
        if (nextDomainId != previousDomainId) {
          qDebug() << "Domain change: "
                   << QString::fromStdString(previousDomainId) << " -> "
                   << QString::fromStdString(nextDomainId);
          auto domainChangeNodes =
              getDomainChangeStreams(previousDomainId, nextDomainId);

          //
          auto nextInstance = CodeValidator::getInstance(next, {}, tree);
          if (nextInstance) {
            auto writes = nextInstance->getCompilerProperty("writes");
            if (writes) {
              auto newWrites = std::make_shared<ListNode>(__FILE__, __LINE__);
              for (auto w : writes->getChildren()) {
                auto writeDomain = static_pointer_cast<BlockNode>(w)->getName();
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
                CodeValidator::getFrameworkForDomain(previousDomainId, tree);
            // We should validate with provided connection framework
            auto nodeDecl = CodeValidator::findDeclaration(
                CodeValidator::streamMemberName(stream->getRight()), {},
                domainChangeNodes.sourceImports, {},
                getFrameworkAlias(previousFramework));
            stream->getRight()->setCompilerProperty("declaration", nodeDecl);
            newStreams.push_back(stream);

            auto connectionNode =
                domainChangeNodes.destStreams->getChildren()[0]
                    ->getChildren()[0];
            connectionNode->setCompilerProperty("outputBlock", next);
            auto destFramework =
                CodeValidator::getFrameworkForDomain(nextDomainId, tree);
            // We should validate with provided connection framework
            nodeDecl = CodeValidator::findDeclaration(
                CodeValidator::streamMemberName(connectionNode), {},
                domainChangeNodes.destImports, {},
                getFrameworkAlias(destFramework));
            connectionNode->setCompilerProperty("declaration", nodeDecl);

            // We need to remove the read domain from the reads metadata in
            // "next"

            auto connectionDomain =
                CodeValidator::getNodeDomain(connectionNode, {}, tree);
            nextInstance->appendToPropertyValue("writes", connectionDomain);

            std::shared_ptr<StreamNode> newStream =
                std::make_shared<StreamNode>(connectionNode, next, __FILE__,
                                             __LINE__);
            newStreams.push_back(newStream);
            auto srcImportNodes =
                domainChangeNodes.sourceImports->getChildren();
            newChildren.insert(newChildren.end(), srcImportNodes.begin(),
                               srcImportNodes.end());
            auto destImportNodes = domainChangeNodes.destImports->getChildren();
            newChildren.insert(newChildren.end(), destImportNodes.begin(),
                               destImportNodes.end());
          } else {
          }
        }
        if (stream->getRight()->getNodeType() == AST::Stream) {
          stream = static_pointer_cast<StreamNode>(stream->getRight());
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

ConnectionNodes StrideSystem::getDomainChangeStreams(string previousDomainId,
                                                     string nextDomainId) {
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
          if (static_pointer_cast<ValueNode>(sourceDomain)->getStringValue() ==
              previousDomainId) {
            sourceDomainsMatch = true;
            break;
          }
        }
      }
      bool destDomainsMatch = false;
      for (auto destDomain : destDomainsList->getChildren()) {
        if (destDomain->getNodeType() == AST::String) {
          if (static_pointer_cast<ValueNode>(destDomain)->getStringValue() ==
              nextDomainId) {
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
                  static_pointer_cast<ValueNode>(import)->getStringValue();
              auto platformName = connector->getPropertyValue("sourcePlatform");
              Q_ASSERT(platformName->getNodeType() == AST::String);
              auto importTree =
                  loadImportTree(importName, "",

                                 static_pointer_cast<ValueNode>(platformName)
                                     ->getStringValue());

              auto importTreeLib = loadImportTree(importName, "", "");
              for (auto child : importTreeLib->getChildren()) {
                importTree->addChild(child);
              }
              for (auto node : importTree->getChildren()) {
                //                                CodeResolver::fillDefaultPropertiesForNode(node,
                //                                m_tree);
                CodeResolver::fillDefaultPropertiesForNode(node, importTree);
              }
              domainChangeNodes.sourceImports = importTree;
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
                  static_pointer_cast<ValueNode>(import)->getStringValue();
              auto platformName =
                  connector->getPropertyValue("destinationFramework");
              Q_ASSERT(platformName->getNodeType() == AST::String);
              auto importTree =
                  loadImportTree(importName, "",
                                 static_pointer_cast<ValueNode>(platformName)
                                     ->getStringValue());
              auto builtinObjects = getBuiltinObjectsReference();
              for (auto child : importTree->getChildren()) {
                CodeResolver::insertBuiltinObjectsForNode(child, builtinObjects,
                                                          importTree);
              }

              domainChangeNodes.destImports = importTree;
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

string StrideSystem::getCommonAncestorDomain(string domainId1, string domainId2,
                                             ASTNode tree) {

  std::function<std::vector<std::string>(std::string, std::string)> getParents =
      [&](std::string domainId,
          std::string framework) -> std::vector<std::string> {
    std::vector<std::string> parents;
    if (framework.size() == 0) {
      framework = CodeValidator::getFrameworkForDomain(domainId, tree);
    }
    auto domainDeclaration =
        CodeValidator::findDomainDeclaration(domainId, tree);
    parents.push_back(domainId);
    if (domainDeclaration) {
      auto parentDomain = domainDeclaration->getPropertyValue("parentDomain");
      if (parentDomain && parentDomain->getNodeType() != AST::None) {
        auto parentDomainCopy = parentDomain->deepCopy();
        parentDomainCopy->setRootScope(framework);
        //      auto parentDomainName =
        //      CodeValidator::streamMemberName(parentDomain); auto
        //      parentDomainDecl = CodeValidator::findDomainDeclaration(
        //          parentDomainName, framework, tree);
        auto parentDomainId =
            CodeValidator::getDomainIdentifier(parentDomainCopy, {}, tree);
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

std::string StrideSystem::getParentDomain(string domainId, ASTNode tree) {
  std::string parentDomainId;
  std::string framework = CodeValidator::getFrameworkForDomain(domainId, tree);

  auto domainDeclaration = CodeValidator::findDomainDeclaration(domainId, tree);
  if (domainDeclaration) {
    auto parentDomain = domainDeclaration->getPropertyValue("parentDomain");
    if (parentDomain && parentDomain->getNodeType() != AST::None) {
      auto parentDomainCopy = parentDomain->deepCopy();
      parentDomainCopy->setRootScope(framework);
      parentDomainId =
          CodeValidator::getDomainIdentifier(parentDomainCopy, {}, tree);
    }
  }
  return parentDomainId;
}

std::shared_ptr<DeclarationNode>
StrideSystem::getFrameworkOperator(std::string platform, std::string leftType,
                                   std::string rightType, std::string type) {

  return nullptr;
}

void StrideSystem::installFramework(string frameworkName) {
  for (auto framework : m_frameworks) {
    if (framework->getFramework() == frameworkName) {
      auto configPath =
          framework->buildPlatformLibPath(m_strideRoot.toStdString()) + "/" +
          "Configuration.stride";
      ASTNode tree = AST::parseFile(configPath.c_str(), nullptr);
      if (tree) {
        for (auto node : tree->getChildren()) {
          if (node->getNodeType() == AST::Declaration) {
            auto decl = static_pointer_cast<DeclarationNode>(node);
            if (decl->getObjectType() == "_frameworkConfiguration") {
              auto installActions = decl->getPropertyValue("installActions");
              if (installActions) {
                for (auto action : installActions->getChildren()) {
                  if (action->getNodeType() == AST::Declaration) {
                    auto commandNode =
                        static_pointer_cast<DeclarationNode>(action)
                            ->getPropertyValue("command");
                    if (commandNode &&
                        commandNode->getNodeType() == AST::String) {
                      auto command = static_pointer_cast<ValueNode>(commandNode)
                                         ->getStringValue();
                      auto ret = system(command.c_str());
                      if (ret != 0) {
                        qDebug() << "Error executing "
                                 << QString::fromStdString(command);
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

QString StrideSystem::makeProject(QString fileName) {
  QFileInfo info(fileName);
  QString dirName =
      info.absolutePath() + QDir::separator() + info.fileName() + "_Products";
  if (!QFile::exists(dirName)) {
    if (!QDir().mkpath(dirName)) {
      return QString();
    }
  }
  return dirName;
}
