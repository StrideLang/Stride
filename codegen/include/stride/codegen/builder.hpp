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

#ifndef BASEPROJECT_H
#define BASEPROJECT_H

#include "stride/parser/ast.h"
#include "systemconfiguration.hpp"

#include <functional>

#define STRIDE_PLUGIN_MAX_STR_LEN 32

class Builder;

typedef Builder *(*create_object_t)(std::string projectDir,
                                    std::string strideRoot,
                                    std::string platformPath);
typedef void (*platform_name_t)(char *name);
typedef int (*platform_version_major_t)();
typedef int (*platform_version_minor_t)();

typedef struct {
  create_object_t create;
  platform_name_t get_name;
  platform_version_major_t get_version_major;
  platform_version_minor_t get_version_minor;
} PluginInterface;

class StrideSystem;

class Builder {
public:
  Builder(std::string projectDir, std::string strideRoot,
          std::string platformPath)
      : m_projectDir(projectDir), m_strideRoot(strideRoot),
        m_platformPath(platformPath) {}
  virtual ~Builder() {}

  std::string getPlatformPath() { return m_platformPath; }
  virtual std::string getStdErr() const = 0;
  virtual std::string getStdOut() const = 0;
  virtual void clearBuffers() = 0;

  // Additional information from stride system
  std::shared_ptr<StrideSystem>
      m_system; // The StrideSystem that created this builder.
  SystemConfiguration m_systemConfiguration;
  std::string m_frameworkName;

  std::function<void(std::string)> outputText = [](std::string) {};
  std::function<void(std::string)> errorText = [](std::string) {};
  std::function<void(void)> programStopped = []() {};
  std::function<void(void)> yieldCallback = []() {};

  virtual std::map<std::string, std::string> generateCode(ASTNode tree) = 0;
  virtual bool build(std::map<std::string, std::string> domainMap) = 0;
  virtual bool deploy() = 0;
  virtual bool run(bool pressed = true) = 0;
  //    // TODO this would need to send encrypted strings or remove the code
  //    sections? virtual QString requestTypesJson() {return "";} virtual
  //    QString requestFunctionsJson() {return "";} virtual QString
  //    requestObjectsJson() {return "";}
  virtual bool isValid() { return false; }

protected:
  std::string m_projectDir;
  std::string m_strideRoot;
  std::string m_platformPath;

  std::string substituteTokens(std::string text);

private:
  PluginInterface m_interface;
  //  QLibrary *m_pluginLibrary;
  //  Builder *m_project;
};

inline std::string Builder::substituteTokens(std::string text) {
  std::map<std::string, std::string> tokenMap;
  tokenMap["%projectDir%"] = m_projectDir;
  tokenMap["%strideRoot%"] = m_strideRoot;
  tokenMap["%platformRoot%"] = m_platformPath;
  tokenMap["%projectName%"] = m_platformPath;
  for (const auto &mapEntry : tokenMap) {
    size_t startPos;
    while ((startPos = text.find(mapEntry.first.c_str())) !=
           std::string::npos) {
      text.replace(startPos, mapEntry.first.size(), tokenMap[mapEntry.second]);
    }
  }
  return text;
}

#endif // BASEPROJECT_H
