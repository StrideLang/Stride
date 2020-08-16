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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

//#include "ast.h"
#include "coderesolver.h"
#include "codevalidator.h"
#include "pythonproject.h"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("stridecc");
  QCoreApplication::setApplicationVersion("0.1-alpha");

  QCommandLineParser parser;
  parser.setApplicationDescription("Stride command line compiler");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument(
      "source", QCoreApplication::translate("main", "Source file to build."));
  //    parser.addPositionalArgument("destination",
  //    QCoreApplication::translate("main", "Destination directory."));

  QCommandLineOption targetDirectoryOption(
      QStringList() << "s"
                    << "stride-root",
      QCoreApplication::translate("main", "Path to strideroot directory"),
      QCoreApplication::translate("main", "directory"));
  parser.addOption(targetDirectoryOption);
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  QString platformRootPath = parser.value(targetDirectoryOption);

  if (args.size() < 1) {
    parser.helpText();
    return -1;
  }
  QString fileName = args.at(0);

  if (platformRootPath.isEmpty()) {
    platformRootPath =
        "/home/andres/Documents/src/Stride/Stride/strideroot"; // For my
                                                               // convenience :)
  }

  //    qDebug() << args.at(0);
  //    qDebug() << platformRootPath;

  ASTNode tree;
  tree = AST::parseFile(fileName.toLocal8Bit().constData());

  bool buildOK = true;
  if (tree) {
    SystemConfiguration config;
    config.testing = true;
    CodeResolver resolver(tree, platformRootPath, config);
    resolver.process();

    CodeValidator validator(tree);

    if (!validator.isValid()) {
      QList<LangError> errors = validator.getErrors();
      for (LangError error : errors) {
        qDebug() << QString::fromStdString(error.getErrorText());
      }
      return -1;
    }
    std::shared_ptr<StrideSystem> system = resolver.getSystem();

    QFileInfo info(fileName);
    QString dirName = info.absolutePath() + QDir::separator() + info.fileName();
    if (!QFile::exists(dirName)) {
      if (!QDir().mkpath(dirName)) {
        qDebug() << "Error creating project path";
        return -1;
      }
    }
    vector<Builder *> builders = system->createBuilders(dirName, tree);

    for (auto builder : builders) {
      auto domainMap = builder->generateCode(tree);
      // TODO find a better way to pass system than here.
      builder->m_system = system;

      if (builder->build(domainMap)) {
        qDebug() << "Built in directory:" << dirName;
      } else {
        qDebug() << "Build failed for " << fileName;
        qDebug() << "Using framework: " << builder->getPlatformPath();

        qDebug() << builder->getStdOut();
        qDebug() << builder->getStdErr();
        buildOK = false;
      }
    }
  } else {
    vector<LangError> errors = AST::getParseErrors();
    for (LangError err : errors) {
      qDebug() << QString::fromStdString(err.getErrorText());
    }
    buildOK = false;
  }
  return buildOK ? 0 : -1;
}
