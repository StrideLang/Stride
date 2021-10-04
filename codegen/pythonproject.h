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

#ifndef PYTHONPROJECT_H
#define PYTHONPROJECT_H

#include <QAtomicInt>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QString>

#include "builder.h"

#include "ast.h"
#include "bundlenode.h"
#include "declarationnode.h"
#include "expressionnode.h"
#include "functionnode.h"
#include "platformnode.h"
#include "streamnode.h"
#include "valuenode.h"

class PythonProject : public Builder {
  Q_OBJECT
public:
  explicit PythonProject(std::string platformName, std::string platformPath,
                         std::string strideRoot,
                         std::string projectDir = std::string(),
                         std::string pythonExecutable = std::string());
  virtual ~PythonProject();

  std::string getStdErr() const override { return m_stdErr.toStdString(); }
  std::string getStdOut() const override { return m_stdOut.toStdString(); }
  void clearBuffers() override {
    m_stdErr = "";
    m_stdOut = "";
  }

signals:

public slots:

  virtual std::map<std::string, std::string>
  generateCode(ASTNode tree) override;
  virtual bool build(std::map<std::string, std::string> domainMap) override;
  virtual bool deploy() override { return true; }
  virtual bool run(bool pressed = true) override;
  virtual bool isValid() override;

  void consoleMessage();

  void stopRunning();

private:
  void writeAST(ASTNode tree);
  void astToJson(ASTNode node, QJsonObject &obj);
  void listToJsonArray(std::shared_ptr<ListNode> node, QJsonArray &obj);
  void streamToJsonArray(std::shared_ptr<StreamNode> node, QJsonArray &array);
  void functionToJson(std::shared_ptr<FunctionNode> node, QJsonObject &obj);
  void expressionToJson(std::shared_ptr<ExpressionNode> node, QJsonObject &obj);
  void appendStreamToArray(ASTNode node, QJsonArray &array);

  std::string m_platformName;
  std::string m_pythonExecutable;
  std::string m_jsonFilename;
  QAtomicInt m_running;
  QProcess m_runningProcess;
  QAtomicInt m_building;
  QProcess m_buildProcess;

  QString m_stdOut;
  QString m_stdErr;
};

#endif // PYTHONPROJECT_H
