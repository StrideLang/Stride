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

#ifndef STREAMPLATFORM_H
#define STREAMPLATFORM_H

#include <map>
#include <string>

#include <QDebug>
#include <QList>
#include <QProcess>

#include "ast.h"
#include "declarationnode.h"
#import "valuenode.h"

class StrideFramework {
public:
  StrideFramework(std::string strideRoot, std::string framework,
                  std::string fwVersion, std::string hardware = "",
                  std::string hardwareVersion = "",
                  std::string rootNamespace = "", std::string identifier = "");

  ~StrideFramework();

  typedef enum { PythonTools, PluginPlatform, NullPlatform } PlatformAPI;

  std::string getFramework() const;
  std::string getFrameworkVersion() const;
  std::string getHardware() const;
  std::string getHardwareVersion() const;
  std::string getRootNamespace() const;
  bool getRequired() const;
  PlatformAPI getAPI() const;
  std::string buildPlatformPath(std::string strideRoot);
  std::string buildPlatformLibPath(std::string strideRoot);
  std::string buildTestingLibPath(std::string strideRoot);

  std::string getPlatformDetails();

  void installFramework();

  std::vector<std::string> getDomainIds();

  void addTree(std::string treeName, ASTNode treeRoot);
  void addTestingTree(std::string treeName, ASTNode treeRoot);
  std::vector<ASTNode> getPlatformObjectsReference();
  std::vector<ASTNode> getPlatformTestingObjectsRef();

  bool getPluginDetails(std::string &pluginName, int &majorVersion,
                        int &minorVersion);

private:
  std::string m_strideRoot;
  std::string m_framework;
  std::string m_frameworkVersion;
  std::string m_hardware;
  std::string m_hardwareVersion;
  std::string m_rootNamespace;
  std::string m_identifier;
  bool m_required;
  PlatformAPI m_api{
      PluginPlatform}; // TODO Put back support for plugin platforms
  std::map<std::string, ASTNode> m_platformTrees;
  std::map<std::string, ASTNode> m_platformTestTrees;
};

#endif // STREAMPLATFORM_H
