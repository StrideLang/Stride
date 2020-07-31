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

#include <iostream>

//#include "ast.h"
#include "toolmanager.hpp"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);
  QCoreApplication::setApplicationName("strdtools");
  QCoreApplication::setApplicationVersion("0.1-alpha");

  QCommandLineParser parser;
  parser.setApplicationDescription("Stride command line tool manager");
  parser.addPositionalArgument(
      "strideroot path",
      QCoreApplication::translate("main", "Path to strideroot"));
  //  parser.addPositionalArgument(
  //      "destination",
  //      QCoreApplication::translate("main", "Destination directory."));

  parser.addOption({"g", QCoreApplication::translate(
                             "main", "Generate all local configurations")});

  parser.addOption({"l", QCoreApplication::translate(
                             "main", "List all local configurations")});
  parser.addOption(
      {"n", QCoreApplication::translate("main", "Update interactively")});

  parser.addOption({"i",
                    QCoreApplication::translate("main", "install framework"),
                    "install", ""});

  parser.addHelpOption();
  parser.addVersionOption();
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  if (args.size() < 1) {
    std::cout << parser.helpText().toStdString() << std::endl;
    return -1;
  }

  // Process strideroot
  QString strideRoot = args.at(0);

  ToolManager toolManager(strideRoot.toStdString());

  bool updateInteractively = parser.isSet("i"); // TODO use
  for (auto toolTemplate : toolManager.toolTemplates) {
    std::cout << "Tool Template: " << toolTemplate->strideName << std::endl;
    std::cout << "  displayName:" << toolTemplate->displayName << std::endl;
  }

  // Process generate local command
  if (parser.isSet("g")) {
    std::cout << "Updating local tools config" << std::endl;
    toolManager.updateAllLocalConfigs();
  }

  if (parser.isSet("l")) {

    // TODO add list
  }

  if (parser.isSet("i")) {
    auto frameworkToInstall = parser.value("i");
  }

  // TODO add flag to update individual

  return 0;
}
