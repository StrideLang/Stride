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

#include "stride/codegen/strideframework.hpp"

//#include <QProcess>

#include <cassert>
#include <filesystem>
#include <iostream>

#include "stride/codegen/astfunctions.hpp"
#include "stride/codegen/astquery.hpp"
#include "stride/parser/declarationnode.h"
#include "stride/parser/valuenode.h"

using namespace std;

StrideFramework::StrideFramework(std::string strideRoot, std::string framework,
                                 std::string fwVersion, std::string hardware,
                                 std::string hardwareVersion,
                                 std::string rootNamespace,
                                 std::string inherits,
                                 std::string inheritsVersion)
    : m_strideRoot(strideRoot), m_framework(framework),
      m_frameworkVersion(fwVersion), m_hardware(hardware),
      m_hardwareVersion(hardwareVersion), m_rootNamespace(rootNamespace),
      m_inherits(inherits), m_inheritsVersion(inheritsVersion) {
  m_inheritedPaths = getInheritedFrameworkPaths(buildPlatformLibPath());
  m_inheritedList = loadInheritedList(buildPlatformLibPath());
  auto nodes = loadFrameworkRoot(buildPlatformLibPath());
  m_trees.push_back(FrameworkTree{"", "", nodes, {}});
}

StrideFramework::~StrideFramework() {}

std::string StrideFramework::getFramework() const { return m_framework; }

std::string StrideFramework::getFrameworkVersion() const {
  return m_frameworkVersion;
}

std::string StrideFramework::getHardware() const { return m_hardware; }

std::string StrideFramework::getHardwareVersion() const {
  return m_hardwareVersion;
}

std::string StrideFramework::getRootNamespace() const {
  return m_rootNamespace;
}

bool StrideFramework::getRequired() const { return m_required; }

StrideFramework::PlatformAPI StrideFramework::getAPI() const { return m_api; }

std::string StrideFramework::buildPlatformPath() {
  std::string path = m_strideRoot + "/";
  path += "frameworks/" + m_framework + "/" + m_frameworkVersion;
  return path;
}

std::string StrideFramework::buildPlatformLibPath() {
  std::string path = buildPlatformPath() + "/";
  path += "platformlib";
  return path;
}

std::string StrideFramework::buildTestingLibPath() {
  std::string path = buildPlatformLibPath();
  path += "/testing";
  return path;
}

std::string StrideFramework::getPlatformDetails() {
  auto frameworkRoot = buildPlatformLibPath();
  return frameworkRoot;
}

void StrideFramework::installFramework() {
  auto frameworkRoot = buildPlatformLibPath();
  auto fileName = frameworkRoot + "/Configuration.stride";
  auto configuration = AST::parseFile(fileName.c_str());
  if (configuration) {
    for (const auto &node : configuration->getChildren()) {
      if (node->getNodeType() == AST::Declaration) {
        auto decl = std::static_pointer_cast<DeclarationNode>(node);
        auto installationNode = decl->getPropertyValue("installation");
        if (installationNode) {
          for (const auto &installDirectiveNode :
               installationNode->getChildren()) {
            if (installDirectiveNode->getNodeType() == AST::Declaration) {
              auto installDirective = std::static_pointer_cast<DeclarationNode>(
                  installDirectiveNode);
              if (installDirective->getObjectType() == "installAction") {
                // FIXME check for platform and language
                auto commandNode =
                    installDirective->getPropertyValue("command");
                auto dirNode =
                    installDirective->getPropertyValue("workingDirectory");
                if (commandNode && commandNode->getNodeType() == AST::String &&
                    dirNode) {
                  std::string workingDirectory = buildPlatformPath();
                  if (dirNode->getNodeType() == AST::String) {
                    workingDirectory +=
                        "/" + std::static_pointer_cast<ValueNode>(dirNode)
                                  ->getStringValue();
                  }
                  auto command =
                      std::static_pointer_cast<ValueNode>(commandNode)
                          ->getStringValue();
                  std::cerr << "Running command: " << command << " IN "
                            << workingDirectory << std::endl;
                  //                  QObject::connect(
                  //                      &installProcess,
                  //                      &QProcess::readyReadStandardOutput,
                  //                      [&]() {
                  //                        qDebug() <<
                  //                        installProcess.readAllStandardOutput();
                  //                      });
                  //                  QObject::connect(
                  //                      &installProcess,
                  //                      &QProcess::readyReadStandardError,
                  //                      [&]() {
                  //                        qDebug() <<
                  //                        installProcess.readAllStandardError();
                  //                      });
                  //                  installProcess.start(QString::fromStdString(command),
                  //                                       QStringList());
#ifdef Q_OS_LINUX
                  auto previousPath = std::filesystem::current_path();
                  std::filesystem::current_path(workingDirectory);
                  FILE *pipe = popen(command.c_str(), "r");
                  if (pipe) {
                    char buffer[128];
                    std::string result = "";
                    // read till end of process:
                    while (!feof(pipe)) {
                      // use buffer to read and add to result
                      if (fgets(buffer, 128, pipe) != NULL)
                        result += buffer;
                    }
                    auto ret = pclose(pipe);
                    if (WIFEXITED(ret)) {
                      if (WEXITSTATUS(ret) != 0) {
                        std::cerr << "ERROR executing : " << command
                                  << std::endl;
                        std::cerr << result << std::endl;
                      }
                    }
                  } else {
                    std::cerr << "popen failed!";
                  }

                  std::filesystem::current_path(previousPath);
#endif
                }
              }
            }
          }
        }
      }
    }
  }
}

std::vector<string> StrideFramework::getDomainIds() {
  std::vector<string> domainIds;
  for (const auto &treeEntry : m_trees) {
    for (const auto &node : treeEntry.nodes) {
      if (node->getNodeType() == AST::Declaration) {
        auto decl = static_pointer_cast<DeclarationNode>(node);
        if (decl->getObjectType() == "_domainDefinition") {
          domainIds.push_back(decl->getName());
        }
      }
    }
  }
  return domainIds;
}

void StrideFramework::addTestingTree(string treeName, ASTNode treeRoot) {
  m_platformTestTrees[treeName] = treeRoot;
}

std::map<string, std::vector<ASTNode>> StrideFramework::getFrameworkMembers() {
  std::map<std::string, std::vector<ASTNode>> libNamespace;
  for (const auto &tree : m_trees) {
    if (libNamespace.find(tree.importAs) == libNamespace.end()) {
      libNamespace[tree.importAs] = std::vector<ASTNode>();
    }
    for (const ASTNode &node : tree.nodes) {
      libNamespace[tree.importAs].push_back(node);
    }
  }
  return libNamespace;
}

vector<ASTNode> StrideFramework::getPlatformObjectsReference() {
  vector<ASTNode> objects;
  auto blockGroup = m_trees.begin();
  while (blockGroup != m_trees.end()) {
    for (const ASTNode &element : blockGroup->nodes) {
      objects.push_back(element);
    }
    blockGroup++;
  }
  return objects;
}

vector<ASTNode> StrideFramework::getPlatformTestingObjectsRef() {
  vector<ASTNode> objects;
  auto blockGroup = m_platformTestTrees.begin();
  while (blockGroup != m_platformTestTrees.end()) {
    for (const ASTNode &element : blockGroup->second->getChildren()) {
      objects.push_back(element);
    }
    blockGroup++;
  }
  return objects;
}

bool StrideFramework::getPluginDetails(string &pluginName, int &majorVersion,
                                       int &minorVersion) {
  for (const auto &tree : m_trees) {
    for (const auto &declNode : tree.nodes) {
      if (declNode->getNodeType() == AST::Declaration) {
        auto decl = static_pointer_cast<DeclarationNode>(declNode);
        if (decl->getObjectType() == "generatorDirectives") {
          auto nameNode = decl->getPropertyValue("codeGenerator");
          if (nameNode && nameNode->getNodeType() == AST::String) {
            pluginName =
                static_pointer_cast<ValueNode>(nameNode)->getStringValue();
          }
          auto majorVersionNode =
              decl->getPropertyValue("codeGeneratorMajorVersion");
          if (majorVersionNode && majorVersionNode->getNodeType() == AST::Int) {
            majorVersion =
                static_pointer_cast<ValueNode>(majorVersionNode)->getIntValue();
          }
          auto minorVersionNode =
              decl->getPropertyValue("codeGeneratorMinorVersion");
          if (minorVersionNode && minorVersionNode->getNodeType() == AST::Int) {
            minorVersion =
                static_pointer_cast<ValueNode>(minorVersionNode)->getIntValue();
          }
          return true;
        }
      }
    }
  }
  return false;
}

std::string StrideFramework::getInherits() const { return m_inherits; }

std::string StrideFramework::getInheritsVersion() const {
  return m_inheritsVersion;
}

std::vector<string> StrideFramework::getInheritedList() const {
  return m_inheritedList;
}

std::vector<ASTNode>
StrideFramework::loadFrameworkRoot(std::string frameworkRoot) {
  // determine inherited framework paths

  auto nodes = ASTFunctions::loadAllInDirectory(frameworkRoot);

  std::vector<ASTNode> inheritedNodes;
  for (const auto &inhPath : m_inheritedPaths) {
    inheritedNodes = ASTFunctions::loadAllInDirectory(inhPath);
    // Remove nodes from inherited if present in current.
    for (const auto &inhNode : inheritedNodes) {
      bool found = false;
      for (auto node = nodes.begin(); node != nodes.end(); node++) {
        if (ASTQuery::getNodeName(*node) == ASTQuery::getNodeName(inhNode)) {
          //          nodes.erase(node);
          found = true;
          break;
        }
      }
      // TODO this might cause issues if there are multiply defined names within
      // the inherited framework. This would be an error within the framework
      // itself that needs to be reported.
      if (!found) {
        nodes.push_back(inhNode);
      }
    }
  }
  for (auto newNode : nodes) {
    if (getRootNamespace().size() > 0) {
      assert(!newNode->getCompilerProperty("namespaceTree"));
      newNode->appendToPropertyValue(
          "namespaceTree",
          std::make_shared<ValueNode>(getRootNamespace(), __FILE__, __LINE__));
    }
    newNode->setCompilerProperty(
        "framework",
        std::make_shared<ValueNode>(getRootNamespace(), __FILE__, __LINE__));
  }
  return nodes;
}

std::vector<std::string>
StrideFramework::getInheritedFrameworkPaths(std::string frameworkRoot) {
  std::vector<std::string> inhPaths;
  auto nodes = ASTFunctions::loadAllInDirectory(frameworkRoot);
  for (const auto &newNode : nodes) {
    if (newNode->getNodeType() == AST::Declaration) {
      auto decl = static_pointer_cast<DeclarationNode>(newNode);
      if (decl->getObjectType() == "_frameworkDescription") {
        auto inheritsNode = decl->getPropertyValue("inherits");
        auto inheritsVersionNode = decl->getPropertyValue("inheritsVersion");
        if (inheritsNode && inheritsVersionNode &&
            inheritsNode->getNodeType() == AST::String &&
            inheritsVersionNode->getNodeType() == AST::String) {
          auto importName =
              static_pointer_cast<ValueNode>(inheritsNode)->getStringValue();
          auto importVersion =
              static_pointer_cast<ValueNode>(inheritsVersionNode)
                  ->getStringValue();
          auto fwRoot = m_strideRoot + "/frameworks/" + importName + "/" +
                        importVersion + "/platformlib";
          inhPaths = getInheritedFrameworkPaths(fwRoot);
          inhPaths.push_back(fwRoot);
        }
      }
    }
  }
  return inhPaths;
}

std::vector<string> StrideFramework::loadInheritedList(string frameworkRoot) {
  std::vector<std::string> inh;
  auto nodes = ASTFunctions::loadAllInDirectory(frameworkRoot);
  for (const auto &newNode : nodes) {
    if (newNode->getNodeType() == AST::Declaration) {
      auto decl = static_pointer_cast<DeclarationNode>(newNode);
      if (decl->getObjectType() == "_frameworkDescription") {
        auto inheritsNode = decl->getPropertyValue("inherits");
        auto inheritsVersionNode = decl->getPropertyValue("inheritsVersion");
        if (inheritsNode && inheritsVersionNode &&
            inheritsNode->getNodeType() == AST::String &&
            inheritsVersionNode->getNodeType() == AST::String) {
          // FIXME look for inheritance recursively
          auto inheritsName =
              static_pointer_cast<ValueNode>(inheritsNode)->getStringValue();
          inh.push_back(inheritsName);
        }
      }
    }
  }
  return inh;
}

std::vector<ASTNode> StrideFramework::loadImport(string importName,
                                                 string importAs) {
  std::vector<ASTNode> newNodes;
  std::string includeSubPath = buildPlatformLibPath();

  if (importName.size() > 0) {
    includeSubPath += "/" + importName;
  }
  auto nodes = ASTFunctions::loadAllInDirectory(includeSubPath);

  for (const auto &node : nodes) {
    newNodes.push_back(node);
  }
  for (const auto &inhPath : m_inheritedPaths) {
    auto subPath = inhPath + "/" + importName;
    auto inheritedNodes = ASTFunctions::loadAllInDirectory(subPath);
    for (auto inhNode : inheritedNodes) {
      for (auto node = nodes.begin(); node != nodes.end(); node++) {
        if (ASTQuery::getNodeName(*node) == ASTQuery::getNodeName(inhNode)) {
          nodes.erase(node);
          std::cerr << "Replacing inherited node: "
                    << ASTQuery::getNodeName(*node) << " from "
                    << (*node)->getFilename() << " with "
                    << ASTQuery::getNodeName(inhNode) << " from "
                    << inhNode->getFilename() << std::endl;
          break;
        }
      }
      // TODO this might cause issues if there are multiply defined names within
      // the inherited framework. This would be an error within the framework
      // itself that needs to be reported.
      newNodes.push_back(inhNode);
    }
  }
  for (auto newNode : nodes) {
    if (getRootNamespace().size() > 0) {
      assert(!newNode->getCompilerProperty("namespaceTree"));
      newNode->appendToPropertyValue(
          "namespaceTree",
          std::make_shared<ValueNode>(getRootNamespace(), __FILE__, __LINE__));
    }
    newNode->setCompilerProperty(
        "framework",
        std::make_shared<ValueNode>(getRootNamespace(), __FILE__, __LINE__));
  }
  for (auto &importTree : m_trees) {
    if (importTree.importAs == importAs) {
      importTree.nodes.insert(importTree.nodes.end(), newNodes.begin(),
                              newNodes.end());
      return newNodes;
    }
  }

  m_trees.push_back(FrameworkTree{importName, importAs, newNodes, {}});
  return newNodes;
}
