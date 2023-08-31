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

#include <cmath>
#include <iostream>

#include <QFile>
#include <QList>
#include <QThread>

//#include "stride/codegen/astfunctions.hpp"
#include "stride/codegen/astquery.hpp"
#include "stride/codegen/coderesolver.hpp"
#include "stride/codegen/codevalidator.hpp"
#include "stride/codegen/systemconfiguration.hpp"
#include "stride/parser/ast.h"

#include "buildtester.hpp"

BuildTester::BuildTester(std::string strideRoot) { m_StrideRoot = strideRoot; }

bool BuildTester::test(std::string filename, std::string expectedResultFile,
                       bool tolerant) {
  bool buildOK = false;
  QList<LangError> errors;
  std::vector<LangError> syntaxErrors;

  ASTNode tree;
  tree = AST::parseFile(filename.c_str());

  syntaxErrors = AST::getParseErrors();

  if (syntaxErrors.size() > 0) {
    for (const auto &syntaxError : syntaxErrors) {
      errors << syntaxError;
    }
    for (LangError error : syntaxErrors) {
      std::cerr << error.getErrorText() << std::endl;
    }
    return false;
  }

  if (tree) {
    std::vector<ASTNode> optionTrees;

    for (const auto &system : ASTQuery::getSystemNodes(tree)) {
      optionTrees = StrideSystem::getOptionTrees(
          m_StrideRoot + "/systems/" + system->platformName() + "/" +
          std::to_string(system->majorVersion()) + "." +
          std::to_string(system->minorVersion()));
      break; // Just use the first system declaration.
    }
    // TODO write default config

    SystemConfiguration config;
    config.testing = true;
    config.readConfiguration(filename);

    CodeResolver resolver(tree, m_StrideRoot, config);
    resolver.process();

    CodeValidator validator(tree);
    for (const auto &err : validator.getErrors()) {
      errors.push_back(err);
    }

    if (errors.size() > 0) {
      for (LangError error : errors) {
        std::cerr << error.getErrorText() << std::endl;
      }
      return false;
    }
    std::shared_ptr<StrideSystem> system = resolver.getSystem();

    std::vector<Builder *> m_builders;

    system->generateDomainConnections(tree);

    m_builders = system->createBuilders(filename, tree);
    if (m_builders.size() == 0) {
      std::cerr << "Can't create builder" << std::endl;
      return false;
    }
    buildOK = true;

    std::vector<std::map<std::string, std::string>> domainMaps;
    for (auto &builder : m_builders) {
      builder->m_system = system;
      domainMaps.push_back(builder->generateCode(tree));
    }

    size_t counter = 0;
    for (auto &builder : m_builders) {
      buildOK &= builder->build(domainMaps[counter++]);
    }

    //    for (auto &builder : m_builders) {
    //      buildOK &= builder->deploy();
    //    }

    if (buildOK) {
      for (auto builder : m_builders) {
        builder->clearBuffers();
        buildOK &= builder->run();
        if (expectedResultFile.size() > 0) {
          QFile expectedResult(QString::fromStdString(expectedResultFile));
          QStringList outputLines =
              QString::fromStdString(builder->getStdOut()).split("\n");
          if (!expectedResult.open(QIODevice::ReadOnly | QIODevice::Text)) {
            std::cerr << "Can't open expected result" << std::endl;
            return false;
          }
          //                 for(int i = 0; i < 7; i++) {
          //                     outputLines.pop_front(); // Hack to remove
          //                     initial text
          //                 }
          if (outputLines.size() < 10) {
            std::cerr << "Too few lines" << std::endl;
            return false; // too few lines
          }

          int counter = 0;
          while (!expectedResult.atEnd() && !(counter >= outputLines.size())) {
            QByteArray line = expectedResult.readLine();
            if (line.endsWith("\n")) {
              line.chop(1);
            }
            if (line.size() > 0 && outputLines.at(counter).size() > 0) {
              double expected = line.toDouble();
              double out = outputLines.at(counter).toDouble();
              double tolerance = 0.01;
              if (tolerant) {
                tolerance = 0.05;
              }
              // To allow for possible 0 values
              double scaledTolerance = fabs(out * tolerance);
              if (out < 1.0e-13 && expected < 1.0e-13) {
                // Ignore result
              } else if (!(std::fabs(out - expected) <= scaledTolerance)) {
                std::cerr << "Failed comparison at line " << counter + 1
                          << std::endl;
                std::cerr << "Got " << outputLines.at(counter).toStdString()
                          << " Expected " << line.toStdString() << std::endl;
                QFile failedOutput("failed.output");
                if (failedOutput.open(QIODevice::WriteOnly)) {
                  failedOutput.write(builder->getStdOut().c_str());
                  failedOutput.close();
                }
                return false;
              }
            }
            counter++;
          }
        } else {
          std::cerr << "No expected results file for: " << filename
                    << std::endl;
        }
      }
    }
    for (auto builder : m_builders) {
      delete builder;
    }
  }
  if (!buildOK) {
    std::cerr << "Error in build/run" << std::endl;
  } else {
    std::cerr << "Passed comparison." << std::endl;
  }
  return buildOK;
}
