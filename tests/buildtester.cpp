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
#include <cmath>

#include <QList>

#include "ast.h"
#include "codevalidator.h"

#include "buildtester.hpp"

BuildTester::BuildTester(std::string strideRoot)
{
    m_StrideRoot = strideRoot;

}

bool BuildTester::test(std::string filename, std::string expectedResultFile)
{
    bool buildOK = false;
     QList<LangError> errors;
     vector<LangError> syntaxErrors;

     ASTNode tree;
     tree = AST::parseFile(filename.c_str());

     syntaxErrors = AST::getParseErrors();

     if (syntaxErrors.size() > 0) {
         for (auto syntaxError:syntaxErrors) {
             errors << syntaxError;
         }
         foreach(LangError error, syntaxErrors) {
             std::cerr << error.getErrorText() << std::endl;
         }
         return false;
     }

     if (tree) {
         CodeValidator validator(QString::fromStdString(m_StrideRoot), tree, CodeValidator::USE_TESTING);
         errors << validator.getErrors();

         if (errors.size() > 0) {
             foreach(LangError error, syntaxErrors) {
                 std::cerr << error.getErrorText() << std::endl;
             }
             return false;
         }
         std::shared_ptr<StrideSystem> system = validator.getSystem();
         system->enableTesting(tree.get());

         std::vector<Builder *> m_builders;

         std::vector<std::string> domains = CodeValidator::getUsedDomains(tree);
         std::vector<std::string> usedFrameworks;
         for (string domain: domains) {
             usedFrameworks.push_back(CodeValidator::getFrameworkForDomain(domain, tree));
         }
         m_builders = system->createBuilders(QString::fromStdString(filename), usedFrameworks);
         if (m_builders.size() == 0) {
             std::cerr << "Can't create builder" << std::endl;
             return false;
         }
         buildOK = true;
         for (auto builder: m_builders) {
//             connect(builder, SIGNAL(outputText(QString)), this, SLOT(printConsoleText(QString)));
//             connect(builder, SIGNAL(errorText(QString)), this, SLOT(printConsoleError(QString)));
//             connect(builder, SIGNAL(programStopped()), this, SLOT(programStopped()));
             buildOK &= builder->build(tree);
         }
         if (buildOK) {
             for (auto builder: m_builders) {
                 builder->clearBuffers();
                 buildOK &= builder->run();
                 QFile expectedResult(QString::fromStdString(expectedResultFile));
                 QStringList outputLines = builder->getStdOut().split("\n");
                 if (!expectedResult.open(QIODevice::ReadOnly | QIODevice::Text)) {
                     return false;
                 }
                 for(int i = 0; i < 7; i++) {
                     outputLines.pop_front(); // Hack to remove initial text
                 }
                 if (outputLines.size() <  10) {
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
                         if (!(std::fabs(out - expected) < 0.000002)) {
                             std::cerr << "Failed comparison at line " << counter + 1 << std::endl;
                             std::cerr << "Got " << outputLines.at(counter).toStdString() << " Expected " << line.toStdString() << std::endl;
                             QFile failedOutput("failed.output");
                             if (failedOutput.open(QIODevice::WriteOnly)) {
                                 failedOutput.write(builder->getStdOut().toLocal8Bit());
                                 failedOutput.close();
                             }
                             return false;
                         }
                     }
                     counter++;
                 }
                 std::cerr << "Passed comparison." << std::endl;
//                 std::cout << builder->getStdOut().toStdString() << std::endl;
             }
         }
         for (auto builder: m_builders) {
             delete builder;
         }
     }

     return buildOK;
}
