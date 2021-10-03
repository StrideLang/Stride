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

#ifndef STRIDESYSTEM_HPP
#define STRIDESYSTEM_HPP

#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#include "strideframework.hpp"
#include "stridelibrary.hpp"
#include "strideparser.h"
#include "systemconfiguration.hpp"

#include "builder.h"

typedef struct {
  ASTNode sourceStreams;
  ASTNode sourceImports;
  ASTNode destStreams;
  ASTNode destImports;
} ConnectionNodes;

class StrideSystem {
public:
  StrideSystem(std::string strideRoot, std::string systemName, int majorVersion,
               int minorVersion, std::vector<std::shared_ptr<ImportNode>>);
  ~StrideSystem();

  QStringList getErrors();
  QStringList getWarnings();
  QStringList getPlatformTypeNames();
  QStringList getFunctionNames();

  std::vector<Builder *> createBuilders(std::string fileName, ASTNode tree);

  ASTNode getPlatformDomain(
      std::string namespaceName = ""); // The platform's default domain

  QMap<QString, QString> getFrameworkTools(std::string namespaceName);
  QMap<QString, QString> getFrameworkPaths(std::string namespaceName);

  std::string substituteTokens(std::string namespaceName, std::string text);
  //    DeclarationNode *getFunction(QString functionName);
  std::vector<std::string> getFrameworkNames();

  // FIXME remove this function, use getImportTrees
  std::map<std::string, std::vector<ASTNode>>
  getBuiltinObjects(); // The key to the map is the namespace name

  //    bool typeHasPort(QString typeName, QString propertyName);

  static std::vector<ASTNode> getOptionTrees(std::string systemPath);

  // if framework != "" then import is limited to the specified framework
  std::vector<ASTNode> loadImportTree(std::string importName,
                                      std::string importAs,
                                      std::string frameworkName = "");
  std::map<std::string, std::vector<ASTNode>> getImportTrees();

  std::string getFrameworkAlias(std::string frameworkName);
  std::string getFrameworkFromAlias(std::string frameworkAlias);

  void generateDomainConnections(ASTNode tree);
  void injectResourceConfiguration(ASTNode tree);

  ConnectionNodes getDomainChangeStreams(std::string previousDomainId,
                                         std::string nextDomainId);

  std::string getCommonAncestorDomain(std::string domainId1,
                                      std::string domainId2, ASTNode tree);

  std::string getParentDomain(std::string domainId, ASTNode tree);

  std::shared_ptr<DeclarationNode> getFrameworkOperator(std::string platform,
                                                        std::string leftType,
                                                        std::string rightType,
                                                        std::string type);

  void installFramework(std::string frameworkName);

  std::vector<std::shared_ptr<StrideFramework>> getFrameworks() {
    return m_frameworks;
  }

  std::vector<std::shared_ptr<DeclarationNode>>
  getFrameworkSynchronization(std::string frameworkName);

  std::shared_ptr<DeclarationNode>
  getFrameworkDataType(std::string frameworkName, std::string strideDataType);

  std::string getFrameworkDefaultDataType(std::string frameworkName,
                                          std::string strideType);

  std::vector<std::shared_ptr<DeclarationNode>>
  getFrameworkOperators(std::string frameworkName);

  std::vector<std::string>
  getFrameworkAliasInherits(std::string frameworkAlias);

  std::string getDataType(ASTNode node, ASTNode tree);

  std::vector<std::shared_ptr<StrideFramework>> m_frameworks;
  std::vector<std::shared_ptr<DeclarationNode>> m_platformDefinitions;
  std::vector<std::shared_ptr<DeclarationNode>>
      m_connectionDefinitions; // Connections between frameworks
  std::map<std::string, std::shared_ptr<DeclarationNode>>
      m_synchronization; // Connections between domains
  SystemConfiguration m_systemConfig;

  std::string getStrideRoot() const;
  void setStrideRoot(const std::string &strideRoot);

  std::vector<std::string> listAvailableImports();

private:
  QVector<ASTNode> getPortsForTypeBlock(DeclarationNode *block);
  //    ListNode *getPortsForFunction(QString typeName);

  //  std::vector<ASTNode> processNewTrees(std::string platformPath,
  //                                       string importName);

  std::shared_ptr<DeclarationNode>
  findDataTypeInPath(std::string path, std::string strideDataType);
  std::string findDefaultDataTypeInPath(std::string path,
                                        std::string strideType);

  std::string makeProject(std::string fileName);

  QString readFile(QString fileName);

  void parseSystemTree(ASTNode systemTree, ASTNode configuration = nullptr);

  std::string m_strideRoot;
  std::string m_systemName;
  int m_majorVersion;
  int m_minorVersion;
  std::string m_systemPath;
  std::string m_rootPath;

  QStringList m_errors;
  QStringList m_warnings;

  QStringList m_types;

  QMap<QString, QString> m_importList;

  std::map<std::string, std::vector<ASTNode>> m_systemNodes;

  StrideLibrary m_library;
};

#endif // STRIDESYSTEM_HPP
