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

#ifndef CODEMODEL_HPP
#define CODEMODEL_HPP

#include <QMutex>
#include <QObject>

#include "ast.h"
#include "stridesystem.hpp"

class CodeModel : public QObject {
  Q_OBJECT
public:
  explicit CodeModel(QObject *parent = 0);

  ~CodeModel();

  QString getHtmlDocumentation(QString symbol);
  QString getTooltipText(QString symbol);
  QPair<QString, int> getSymbolLocation(QString symbol);

  std::shared_ptr<StrideSystem> getSystem() { return m_system; }

  // Copy of current tree, it is safe to use outside CodeModel
  // But the caller must clean it up.
  ASTNode getOptimizedTree();

  //    Builder *createBuilder(QString projectDir);

  QStringList getTypes();
  QStringList getFunctions();
  QStringList getObjectNames();
  QString getFunctionSyntax(QString symbol);
  QString getTypeSyntax(QString symbol);
  QList<LangError> getErrors();
  void updateCodeAnalysis(QString code, QString platformRootPath,
                          QString sourceFile);

signals:

public slots:

private:
  //    QList<AST *> m_platformObjects;
  std::shared_ptr<StrideSystem> m_system;
  std::vector<std::string> m_types;
  std::vector<std::string> m_funcs;
  std::vector<std::string> m_objectNames;
  std::vector<LangError> m_errors;
  QMutex m_validTreeLock;
  ASTNode m_lastValidTree;
};

#endif // CODEMODEL_HPP
