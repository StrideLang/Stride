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

#include <iostream>

#include "../parser/declarationnode.h"
#include "../parser/valuenode.h"
#include "strideframework.hpp"

using namespace std;

StrideFramework::StrideFramework(std::string strideRoot, std::string framework,
                                 std::string fwVersion, std::string hardware,
                                 std::string hardwareVersion,
                                 std::string rootNamespace,
                                 std::string identifier)
    : m_strideRoot(strideRoot), m_framework(framework),
      m_frameworkVersion(fwVersion), m_hardware(hardware),
      m_hardwareVersion(hardwareVersion), m_rootNamespace(rootNamespace),
      m_identifier(identifier) {

  auto platformPath = buildPlatformPath(strideRoot);
}

StrideFramework::~StrideFramework() {}

string StrideFramework::getFramework() const { return m_framework; }

string StrideFramework::getFrameworkVersion() const {
  return m_frameworkVersion;
}

string StrideFramework::getHardware() const { return m_hardware; }

string StrideFramework::getHardwareVersion() const { return m_hardwareVersion; }

string StrideFramework::getRootNamespace() const { return m_rootNamespace; }

bool StrideFramework::getRequired() const { return m_required; }

StrideFramework::PlatformAPI StrideFramework::getAPI() const { return m_api; }

string StrideFramework::buildPlatformPath(string strideRoot) {
  string path = strideRoot + "/";
  path += "frameworks/" + m_framework + "/" + m_frameworkVersion;
  return path;
}

string StrideFramework::buildPlatformLibPath(string strideRoot) {
  string path = buildPlatformPath(strideRoot) + "/";
  path += "platformlib";
  return path;
}

string StrideFramework::buildTestingLibPath(string strideRoot) {
  string path = buildPlatformLibPath(strideRoot);
  path += "/testing";
  return path;
}

string StrideFramework::getPlatformDetails() {
  auto frameworkRoot = buildPlatformLibPath(m_strideRoot);
  return frameworkRoot;
}

void StrideFramework::installFramework() {
  auto frameworkRoot = buildPlatformLibPath(m_strideRoot);
  auto fileName = frameworkRoot + "/Configuration.stride";
  auto configuration = AST::parseFile(fileName.c_str());
  if (configuration) {
    for (auto node : configuration->getChildren()) {
      if (node->getNodeType() == AST::Declaration) {
        auto decl = std::static_pointer_cast<DeclarationNode>(node);
        auto installationNode = decl->getPropertyValue("installation");
        if (installationNode) {
          for (auto installDirectiveNode : installationNode->getChildren()) {
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
                  QString workingDirectory =
                      QString::fromStdString(frameworkRoot);
                  if (dirNode->getNodeType() == AST::String) {
                    workingDirectory +=
                        "/" + QString::fromStdString(
                                  std::static_pointer_cast<ValueNode>(dirNode)
                                      ->getStringValue());
                  }
                  QProcess installProcess;
                  installProcess.setWorkingDirectory(workingDirectory);
                  auto command =
                      std::static_pointer_cast<ValueNode>(commandNode)
                          ->getStringValue();
                  qDebug() << "Running command: "
                           << QString::fromStdString(command);
                  installProcess.start(QString::fromStdString(command));
                  installProcess.waitForFinished();
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
  for (auto treeEntry : m_platformTrees) {
    for (auto node : treeEntry.second->getChildren()) {
    }
  }
  return std::vector<string>();
}

void StrideFramework::addTree(string treeName, ASTNode treeRoot) {
  if (m_platformTrees.find(treeName) != m_platformTrees.end()) {
    std::cerr << "WARNING: tree: '" << treeName << "' exists. Replacing."
              << std::endl;
  } else {
    m_platformTrees[treeName] = treeRoot;
  }
}

void StrideFramework::addTestingTree(string treeName, ASTNode treeRoot) {
  m_platformTestTrees[treeName] = treeRoot;
}

vector<ASTNode> StrideFramework::getPlatformObjectsReference() {
  vector<ASTNode> objects;
  auto blockGroup = m_platformTrees.begin();
  while (blockGroup != m_platformTrees.end()) {
    for (ASTNode element : blockGroup->second->getChildren()) {
      if (element->getNodeType() == AST::Declaration) {
        objects.push_back(element);
      } else {
        objects.push_back(element); // TODO: This inserts everything. Only
                                    // insert what is needed
      }
    }
    blockGroup++;
  }
  return objects;
}

vector<ASTNode> StrideFramework::getPlatformTestingObjectsRef() {
  vector<ASTNode> objects;
  auto blockGroup = m_platformTestTrees.begin();
  while (blockGroup != m_platformTestTrees.end()) {
    for (ASTNode element : blockGroup->second->getChildren()) {
      if (element->getNodeType() == AST::Declaration) {
        objects.push_back(element);
      } else {
        objects.push_back(element); // TODO: This inserts everything. Only
                                    // insert what is needed
      }
    }
    blockGroup++;
  }
  return objects;
}

bool StrideFramework::getPluginDetails(string &pluginName, int &majorVersion,
                                       int &minorVersion) {
  if (m_platformTrees.find("") != m_platformTrees.end()) {
    for (auto declNode : m_platformTrees[""]->getChildren()) {
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
