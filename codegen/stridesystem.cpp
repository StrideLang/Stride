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

StrideSystem::StrideSystem(QString strideRoot, QString systemName,
                           int majorVersion, int minorVersion,
                           std::vector<std::shared_ptr<ImportNode>> importList)
    : m_strideRoot(strideRoot), m_systemName(systemName),
      m_majorVersion(majorVersion), m_minorVersion(minorVersion),
      m_testing(false) {
  QString versionString =
      QString("%1.%2").arg(m_majorVersion).arg(m_minorVersion);
  m_systemPath =
      QDir(strideRoot + QDir::separator() + "systems" + QDir::separator() +
           systemName + QDir::separator() + versionString)
          .absolutePath();
  QString systemFile = m_systemPath + QDir::separator() + "System.stride";

  m_library.initializeLibrary(strideRoot);

  for (auto importNode : importList) {
    m_library.getImportTree(QString::fromStdString(importNode->importName()),
                            QString::fromStdString(importNode->importAlias()),
                            {});
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
      for (auto platform : m_frameworks) {
        QStringList nameFilters;
        nameFilters.push_back("*.stride");
        string platformPath =
            platform->buildTestingLibPath(m_strideRoot.toStdString());
        QFileInfoList libraryFiles = QDir(QString::fromStdString(platformPath))
                                         .entryInfoList(nameFilters);
        for (auto fileInfo : libraryFiles) {
          ASTNode tree = AST::parseFile(
              fileInfo.absoluteFilePath().toLocal8Bit().data(), nullptr);
          if (tree) {
            platform->addTestingTree(fileInfo.baseName().toStdString(), tree);
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

void StrideSystem::parseSystemTree(ASTNode systemTree) {
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
        if (platforms->getNodeType() == AST::List) {
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
    auto sourcePlatform = connection->getPropertyValue("sourcePlatform");
    auto destPlatform = connection->getPropertyValue("destinationPlatform");
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
                  "sourcePlatform", declaration->getPropertyValue("framework"));
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
                  "destinationPlatform",
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
                   platform->getFramework()) != usedFrameworks.end())) {
      if (platform->getAPI() == StrideFramework::PythonTools) {
        QString pythonExec = "python";
        Builder *builder = new PythonProject(
            QString::fromStdString(platform->getFramework()),
            QString::fromStdString(
                platform->buildPlatformPath(m_strideRoot.toStdString())),
            m_strideRoot, projectDir, pythonExec);
        if (builder) {
          if (builder->isValid()) {
            builder->m_connectors = m_connectionDefinitions;
            builder->m_system = this;
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
                    builder->m_connectors = m_connectionDefinitions;
                    builder->m_system = this;
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
        auto validScopes = node->getCompilerProperty("namespaceTree");
        if (validScopes) {
          assert(validScopes->getNodeType() == AST::List);
          bool rootFound = false;
          bool relativeScopeFound = false;
          for (auto child : validScopes->getChildren()) {
            std::shared_ptr<ValueNode> stringValue =
                std::static_pointer_cast<ValueNode>(child);
            assert(child->getNodeType() == AST::String);
            if (stringValue->getStringValue() == "::") {
              rootFound = true;
            } else if (stringValue->getStringValue() == "") {
              relativeScopeFound = true;
            }
          }
          //                    if (!rootFound) {
          //                        validScopes->addChild(std::make_shared<ValueNode>(string("::"),
          //                        __FILE__, __LINE__));
          //                    }
          if (!relativeScopeFound) {
            validScopes->addChild(
                std::make_shared<ValueNode>(string(""), __FILE__, __LINE__));
          }
        } else {
          auto newList = std::make_shared<ListNode>(
              std::make_shared<ValueNode>(string("::"), __FILE__, __LINE__),
              __FILE__, __LINE__);
          newList->addChild(
              std::make_shared<ValueNode>(string(""), __FILE__, __LINE__));
          node->setCompilerProperty("namespaceTree", newList);
        }
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
    //        auto nodeNoNameSpace = object->deepCopy();
    //        nodeNoNameSpace->setNamespaceList(vector<string>());
    //        objects[""].push_back(nodeNoNameSpace);
  }
  for (std::shared_ptr<DeclarationNode> decl : m_platformDefinitions) {
    objects[""].push_back(decl);
    //        auto nodeNoNameSpace = decl->deepCopy();
    //        nodeNoNameSpace->setNamespaceList(vector<string>());
    //        objects[""].push_back(nodeNoNameSpace);
  }
  return objects;
}

vector<ASTNode> StrideSystem::getOptionTrees() {
  vector<ASTNode> optionTrees;
  QStringList nameFilters;
  nameFilters.push_back("*.stride");
  QString optionPath = m_systemPath + QDir::separator() + "options";
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

ASTNode StrideSystem::getImportTree(QString importName, QString importAs,
                                    QString platformName) {
  ASTNode tree = std::make_shared<AST>();
  QStringList nameFilters;
  nameFilters.push_back("*.stride");
  // Look first in platforms
  // TODO currently search order is defined by the order in which platforms
  // are declared. Should there be more explicit ordering or restrictions?
  if (platformName.size() > 0) {
    for (auto platform : m_frameworks) {
      if (platformName.size() > 0 &&
          ((platformName.toStdString() == platform->getRootNamespace()) ||
           platformName.toStdString() == platform->getFramework())) {
        string platformPath =
            platform->buildPlatformLibPath(m_strideRoot.toStdString());
        QString includeSubPath =
            QString::fromStdString(platformPath) + "/" + importName;
        QStringList libraryFiles = QDir(includeSubPath).entryList(nameFilters);
        if (QFile::exists(QString::fromStdString(platformPath) + "/" +
                          importName + ".stride")) {
          libraryFiles << "../" + importName + ".stride";
        }
        for (QString file : libraryFiles) {
          QString fileName =
              QDir::cleanPath(includeSubPath + QDir::separator() + file);
          auto newTree = AST::parseFile(fileName.toLocal8Bit().data(), nullptr);
          if (newTree) {
            for (ASTNode node : newTree->getChildren()) {
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

          for (ASTNode node : tree->getChildren()) {
            size_t indexExtension = file.toStdString().find(".stride");
            string scopeName = file.toStdString().substr(0, indexExtension);
            // FIXME allow namespacing by filename
            node->appendToPropertyValue(
                "namespaceTree",
                std::make_shared<ValueNode>(importAs.toStdString(), __FILE__,
                                            __LINE__));
            node->appendToPropertyValue(
                "namespaceTree",
                std::make_shared<ValueNode>(scopeName, __FILE__, __LINE__));
            node->appendToPropertyValue(
                "namespaceTree",
                std::make_shared<ValueNode>(platform->getFramework(), __FILE__,
                                            __LINE__));
            node->appendToPropertyValue(
                "platform",
                std::make_shared<ValueNode>(platformName.toStdString(),
                                            __FILE__, __LINE__));
          }
        }
      }
    }

  } else {
    tree = m_library.getImportTree(importName, importAs, QStringList());
  }

  if (tree->getChildren().size() > 0) {
    return tree;
  }

  return nullptr;
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
          // FIXME currently only simple two member connector streams supported
          if (domainChangeNodes.sourceStreams &&
              domainChangeNodes.destStreams) {
            stream->setRight(domainChangeNodes.sourceStreams->getChildren()[0]
                                 ->getChildren()[1]);
            stream->getRight()->setCompilerProperty("inputBlock",
                                                    stream->getLeft());
            auto nodeDecl = CodeValidator::findDeclaration(
                CodeValidator::streamMemberName(stream->getRight()), {},
                domainChangeNodes.sourceImports);
            stream->getRight()->setCompilerProperty("declaration", nodeDecl);
            newStreams.push_back(stream);

            auto connectionNode =
                domainChangeNodes.destStreams->getChildren()[0]
                    ->getChildren()[0];
            connectionNode->setCompilerProperty("outputBlock", next);
            nodeDecl = CodeValidator::findDeclaration(
                CodeValidator::streamMemberName(connectionNode), {},
                domainChangeNodes.destImports);
            connectionNode->setCompilerProperty("declaration", nodeDecl);
            //                        auto connectionDomainNode =
            //                        CodeValidator::getNodeDomain(connectionNode,
            //                        {{"",
            //                        domainChangeNodes.destImports->getChildren()}},
            //                        tree); auto connectionDomainId =
            //                        CodeValidator::getDomainIdentifier(connectionDomainNode
            //                                                                                     , {{"", domainChangeNodes.destImports->getChildren()}}, tree);

            //                        if (connectionDomainId.size() > 0 &&
            //                        connectionDomainId != nextDomainId) { //
            //                        If domains don't match, add an extra
            //                        signal to make the connection
            //                            // Do we only need to do this for
            //                            platformModules, or should this always
            //                            be done? QString signalName =
            //                            "BridgeSignal"; auto signalDecl =
            //                            CodeResolver::createSignalDeclaration(signalName,
            //                            1, {}, tree);
            //                            signalDecl->setPropertyValue("domain",
            //                            connectionDomainNode->deepCopy());
            //                            newChildren.insert(newChildren.end(),
            //                            signalDecl); next =
            //                            std::make_shared<StreamNode>(
            //                                        std::make_shared<BlockNode>(signalName.toStdString(),__FILE__,
            //                                        __LINE__), next, __FILE__,
            //                                        __LINE__);

            //                        }
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
    auto sourcePlatform = connector->getPropertyValue("sourcePlatform");
    auto destPlatform = connector->getPropertyValue("destPlatform");
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
                  getImportTree(QString::fromStdString(importName), "",
                                QString::fromStdString(
                                    static_pointer_cast<ValueNode>(platformName)
                                        ->getStringValue()));

              auto importTreeLib =
                  getImportTree(QString::fromStdString(importName), "", "");
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
                  connector->getPropertyValue("destinationPlatform");
              Q_ASSERT(platformName->getNodeType() == AST::String);
              auto importTree =
                  getImportTree(QString::fromStdString(importName), "",
                                QString::fromStdString(
                                    static_pointer_cast<ValueNode>(platformName)
                                        ->getStringValue()));
              if (importTree) {

                auto builtinObjects = getBuiltinObjectsReference();
                for (auto child : importTree->getChildren()) {
                  CodeResolver::insertBuiltinObjectsForNode(
                      child, builtinObjects, importTree);
                }

                domainChangeNodes.destImports = importTree;
                //                                    scopeStack.push_back({"",
                //                                    importTree->getChildren()});
              } else {
                //                                    std::cerr << "ERROR:
                //                                    Cannot import " <<
                //                                    importName << " for
                //                                    connection " <<
                //                                    connector->getName() <<
                //                                    std::endl;
              }
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
