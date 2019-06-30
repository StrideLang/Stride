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

#include <QString>
#include <QtTest>
#include <QScopedPointer>

#include "strideparser.h"
#include "strideplatform.hpp"
#include "codevalidator.h"
#include "coderesolver.h"
#include "buildtester.hpp"

#define STRIDEROOT "../strideroot"

extern AST *parse(const char* fileName);

class ParserTest : public QObject
{
    Q_OBJECT

public:
    ParserTest();

private:
    void testMultichannelUgens(); // TODO Need to complete support for this.

private Q_SLOTS:

//    // Test code generation
    void testCodeGeneration();

    // Parser
    void testModules();
    void testScope();
    void testPortProperty();
    void testBundleIndeces();
    void testBasicNoneSwitch();
    void testBasicFunctions();
    void testBasicStream();
    void testBasicBundle();
    void testBasicBlocks();
    void testHeader();
    void testLoop();
    void testBuffer();
    void testBlockIOResolution();

    //Expansion
    void testLibraryObjectInsertion();
    void testStreamExpansion();
    void testStreamRates();
    void testConstantResolution();
    void testNamespaces();

    //Platform
    void testPlatformCommonObjects();
    void testValueTypeExpressionResolution();
    void testDuplicates();
    void testValueTypeBundleResolution();
    void testImport();
    void testContextDomain();
    void testDomains();
    void testLists();
    void testPortNameValidation();
    void testTriggersRegistration();
    void testResetRegistration();

    // Connections
    void testConnectionErrors();
    void testConnectionCount();

//    void testBlockMembers();
    void testModuleDomains();
    void testPortTypeValidation();

    // Library
    void testLibraryBasicTypes();
    void testLibraryValidation();


    // Code generation/Compiler
//    void testCodeGeneration();
    void testCompilation();
};

ParserTest::ParserTest()
{
    qSetMessagePattern("[%{time yyyy-MM-dd h:mm:ss.zzz}%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}][%{file}:%{line} %{function}] %{message}");
}

void ParserTest::testMultichannelUgens()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/E03_multichn_streams.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(!generator.isValid());

    QList<LangError> errors = generator.getErrors();
    LangError error = errors.takeFirst();
    QVERIFY(error.type == LangError::StreamMemberSizeMismatch);
    QVERIFY(error.lineNumber == 22);
    QVERIFY(error.errorTokens[0] == "2");
    QVERIFY(error.errorTokens[1] == "Pan");
    QVERIFY(error.errorTokens[2] == "1");

    error = errors.takeFirst();
    QVERIFY(error.type == LangError::StreamMemberSizeMismatch);
    QVERIFY(error.lineNumber == 25);
    QVERIFY(error.errorTokens[0] == "4");
//    QVERIFY(error.errorTokens[1] == "DummyStereo");
    QVERIFY(error.errorTokens[2] == "2");
}

void ParserTest::testBuffer()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/16_buffer.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(generator.isValid());
}

void ParserTest::testBlockIOResolution()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/18_block_io_resolution.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(generator.isValid());
//    OuterInput >> TestModule()  >> OuterOutput;
    unsigned int index = 0;
    while (tree->getChildren()[index]->getNodeType() != AST::Stream) {
        index++;
    }
    StreamNode *stream = static_cast<StreamNode *>(tree->getChildren()[index].get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    auto node = stream->getRight()->getChildren()[0];
    QVERIFY(node->getNodeType() == AST::Function);
    auto outputBlock = static_pointer_cast<BlockNode>(node->getCompilerProperty("outputBlock"));
    auto inputBlock = static_pointer_cast<BlockNode>(node->getCompilerProperty("inputBlock"));

    QVERIFY(outputBlock->getNodeType() == AST::Block);
    QVERIFY(inputBlock->getNodeType() == AST::Block);

    QVERIFY(outputBlock->getName() == "OuterOutput");
    QVERIFY(inputBlock->getName() == "OuterInput");

    index++;
//    OuterInput >> TestModule() >> TestModule() >> OuterOutput;

    stream = static_cast<StreamNode *>(tree->getChildren()[index].get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    node = stream->getRight()->getChildren()[0];
    QVERIFY(node->getNodeType() == AST::Function);
    auto outputFunc = static_pointer_cast<FunctionNode>(node->getCompilerProperty("outputBlock"));
    inputBlock = static_pointer_cast<BlockNode>(node->getCompilerProperty("inputBlock"));
    QVERIFY(outputFunc->getNodeType() == AST::Function);
    QVERIFY(inputBlock->getNodeType() == AST::Block);
    QVERIFY(outputFunc->getName() == "TestModule");
    QVERIFY(inputBlock->getName() == "OuterInput");

    node = static_cast<StreamNode *>(stream->getRight()->getChildren()[1].get())->getLeft();

    outputBlock = static_pointer_cast<BlockNode>(node->getCompilerProperty("outputBlock"));
    auto inputFunc = static_pointer_cast<FunctionNode>(node->getCompilerProperty("inputBlock"));
    QVERIFY(outputBlock->getNodeType() == AST::Block);
    QVERIFY(inputFunc->getNodeType() == AST::Function);

    QVERIFY(outputBlock->getName() == "OuterOutput");
    QVERIFY(inputFunc->getName() == "TestModule");

}

void ParserTest::testLoop()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/17_loop.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(generator.isValid());

    return; // For now while loops are properly implemented
    auto members = tree->getChildren();
    auto loopIn = static_pointer_cast<DeclarationNode>(members[1]);
    QVERIFY(loopIn->getNodeType() == AST::Declaration);
    QVERIFY(loopIn->getName() == "LoopIn");

    auto loopOut = static_pointer_cast<DeclarationNode>(members[2]);
    QVERIFY(loopOut->getNodeType() == AST::Declaration);
    QVERIFY(loopOut->getName() == "LoopOut");

    auto loopInOut = static_pointer_cast<DeclarationNode>(members[3]);
    QVERIFY(loopInOut->getNodeType() == AST::Declaration);
    QVERIFY(loopInOut->getName() == "LoopInOut");

    auto module = static_pointer_cast<DeclarationNode>(members[4]);

    QVERIFY(module->getNodeType() == AST::Declaration);
    QVERIFY(module->getName() == "ContainsLoop");

    auto loopInModule = static_pointer_cast<DeclarationNode>(module->getPropertyValue("blocks")->getChildren()[0]);
    QVERIFY(loopInModule->getNodeType() == AST::Declaration);
    QVERIFY(loopInModule->getName() == "InModule");

    loopInModule = static_pointer_cast<DeclarationNode>(module->getPropertyValue("blocks")->getChildren()[1]);
    QVERIFY(loopInModule->getNodeType() == AST::Declaration);
    QVERIFY(loopInModule->getName() == "InModuleInput");
}

void ParserTest::testPortNameValidation()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P11_port_name_validation.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(!generator.isValid());

    QList<LangError> errors = generator.getErrors();
    QVERIFY(errors.size() == 1);
    LangError error = errors.takeFirst();
    QVERIFY(error.type == LangError::InvalidPort);
    QVERIFY(error.lineNumber == 20);
    QVERIFY(error.errorTokens[0] == "TestModule");
    QVERIFY(error.errorTokens[1] == "invalid");
}

void ParserTest::testCodeGeneration()
{
    QStringList testFiles;
    QDirIterator directories(QFINDTESTDATA(STRIDEROOT "/frameworks/RtAudio/1.0/_tests/"), QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);

    BuildTester tester(QFINDTESTDATA(STRIDEROOT).toStdString());

    QStringList toIgnore = {"simple","buffer", "module", "reactions", "combinations", "table", "loop", "sync"};
    while (directories.hasNext()) {
        QString dirName = directories.next();
        if (!toIgnore.contains(dirName.mid(dirName.lastIndexOf("/") + 1))) {
            QDir dir(dirName);
            dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
            dir.setSorting(QDir::Name);
            QStringList nameFilters;
            nameFilters << "*.stride";

            QFileInfoList list = dir.entryInfoList(nameFilters);
            for (auto fileInfo : list) {
                qDebug() << "Testing: " << fileInfo.absoluteFilePath();
                QString expectedName = fileInfo.absolutePath() + QDir::separator() + fileInfo.baseName() + ".expected";
                if (!QFile::exists(expectedName)) {
                    expectedName = "";
                }
                bool tolerant = false;
                if (dirName.endsWith("platform")) {
                    tolerant = true;
                }
                QVERIFY(tester.test(fileInfo.absoluteFilePath().toStdString(), expectedName.toStdString(), tolerant));
            }
        }
      }
}

void ParserTest::testCompilation()
{
    QStringList testFiles;
    QDirIterator directories(QFINDTESTDATA(STRIDEROOT "/frameworks/RtAudio/1.0/_tests/"), QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QStringList toIgnore = {"buffer", "loop", "sync"};
    while(directories.hasNext()){
        QString dirName = directories.next();
        if (!toIgnore.contains(dirName.mid(dirName.lastIndexOf("/") + 1))) {
            QDir subDir(directories.filePath());
            for (auto entry: subDir.entryList(QStringList() << "*.stride", QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks)) {
                testFiles << subDir.absolutePath() + QDir::separator()
                             + entry;
            }
        }
    }
//    qDebug() << testFiles;
    // BUILDPATH should be set by tests.pro from the current build directory
    QString compilerBin = BUILDPATH "../compiler/compiler";
    QScopedPointer<QProcess> compilerProcess(new QProcess(this));
    for (auto testFile:  testFiles) {
        QStringList arguments;
        arguments << "-s" + QFINDTESTDATA(STRIDEROOT)
                  << testFile;
        qDebug() << arguments.join(" ");
        int ret = compilerProcess->execute(compilerBin, arguments);
        QVERIFY(ret == 0);
    }
//    qDebug() << compilerBin;
}

//void ParserTest::testBlockMembers()
//{
//    ASTNode tree;
//    tree = AST::parseFile(QString(QFINDTESTDATA("data/14_members.stride")).toStdString().c_str());
//    QVERIFY(tree != nullptr);
//    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
//    QVERIFY(generator.isValid());

////    signal Sig {
////        rate: 10
////    }

////    Sig.rate >> Out1;

//    StreamNode *stream = static_cast<StreamNode *>(tree->getChildren().at(2).get());
//    QVERIFY(stream->getNodeType() == AST::Stream);
//    ValueNode *value = static_cast<ValueNode *>(stream->getLeft().get());
//    QVERIFY(value->getNodeType() == AST::Int);
//    QVERIFY(value->getIntValue() == 10);

//    //    Sig.rate + 1 >> Out2

//    stream = static_cast<StreamNode *>(tree->getChildren().at(3).get());
//    QVERIFY(stream->getNodeType() == AST::Stream);
//    value = static_cast<ValueNode *>(stream->getLeft().get());
//    QVERIFY(value->getNodeType() == AST::Int);
//    QVERIFY(value->getIntValue() == 11);

////    2 + Sig.rate >> Out3;

//    stream = static_cast<StreamNode *>(tree->getChildren().at(4).get());
//    QVERIFY(stream->getNodeType() == AST::Stream);
//    value = static_cast<ValueNode *>(stream->getLeft().get());
//    QVERIFY(value->getNodeType() == AST::Int);
//    QVERIFY(value->getIntValue() == 12);

////    2 + Sig.rate + 1 >> Out4;
//    stream = static_cast<StreamNode *>(tree->getChildren().at(5).get());
//    QVERIFY(stream->getNodeType() == AST::Stream);
//    value = static_cast<ValueNode *>(stream->getLeft().get());
//    QVERIFY(value->getNodeType() == AST::Int);
//    QVERIFY(value->getIntValue() == 13);

//    //    A + Sig.rate >> Out5;

//    stream = static_cast<StreamNode *>(tree->getChildren().at(6).get());
//    QVERIFY(stream->getNodeType() == AST::Stream);
//    ExpressionNode *expr = static_cast<ExpressionNode *>(stream->getLeft().get());
//    QVERIFY(expr->getNodeType() == AST::Expression);
//    value = static_cast<ValueNode *>(expr->getRight().get());
//    QVERIFY(value->getNodeType() == AST::Int);
//    QVERIFY(value->getIntValue() == 10);

//    //    #[Sig.rate, 4] >> Out4;
//}

void ParserTest::testConnectionCount()
{

    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/13_connection_count.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

//    signal InSignal {}
//    signal OutSignal {}
//    signal OutSignal2 {}

//    InSignal >> OutSignal;
//    InSignal >> OutSignal2;
//    InSignal2 >> OutSignal2;

    DeclarationNode *block = static_cast<DeclarationNode *>(tree->getChildren().at(1).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    ListNode *reads = static_cast<ListNode *>(block->getCompilerProperty("reads").get());
    QVERIFY(reads->getNodeType() == AST::List);
    QVERIFY(reads->getChildren().size() == 2);
    ListNode *writes = static_cast<ListNode *>(block->getCompilerProperty("writes").get());
    QVERIFY(writes->getNodeType() == AST::List);
    QVERIFY(writes->getChildren().size() == 0);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(2).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    reads = static_cast<ListNode *>(block->getCompilerProperty("reads").get());
    QVERIFY(reads->getNodeType() == AST::List);
    QVERIFY(reads->getChildren().size() == 0);
    writes = static_cast<ListNode *>(block->getCompilerProperty("writes").get());
    QVERIFY(writes->getNodeType() == AST::List);
    QVERIFY(writes->getChildren().size() == 1);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(3).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    reads = static_cast<ListNode *>(block->getCompilerProperty("reads").get());
    QVERIFY(reads->getNodeType() == AST::List);
    QVERIFY(reads->getChildren().size() == 0);
    writes = static_cast<ListNode *>(block->getCompilerProperty("writes").get());
    QVERIFY(writes->getNodeType() == AST::List);
    QVERIFY(writes->getChildren().size() == 2);

//    module Test {
//        blocks: [
//            signal Sig1 {}
//            signal Sig2 {}
//            signal Sig3 {}
//            signal Sig4 {}
//        ]
//        streams: [
//            Sig1 >> Sig2;
//            Sig2 >> Sig3;
//            Sig4 >> Sig3;
//        ]
//    }

    block = static_cast<DeclarationNode *>(tree->getChildren().at(7).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    QVERIFY(block->getName() == "Test");
    ListNode *blocks = static_cast<ListNode *>(block->getPropertyValue("blocks").get());
    QVERIFY(blocks != nullptr);
    QVERIFY(blocks->getNodeType() == AST::List);
    DeclarationNode *declaration = static_cast<DeclarationNode *>(blocks->getChildren().at(0).get());
    QVERIFY(declaration->getName() == "Sig1");

    reads = static_cast<ListNode *>(declaration->getCompilerProperty("reads").get());
    QVERIFY(reads->getNodeType() == AST::List);
    QVERIFY(reads->getChildren().size() == 1);
    writes = static_cast<ListNode *>(declaration->getCompilerProperty("writes").get());
    QVERIFY(writes->getNodeType() == AST::List);
    QVERIFY(writes->getChildren().size() == 0);

    declaration = static_cast<DeclarationNode *>(blocks->getChildren().at(1).get());
    QVERIFY(declaration->getName() == "Sig2");

    reads = static_cast<ListNode *>(declaration->getCompilerProperty("reads").get());
    QVERIFY(reads->getNodeType() == AST::List);
    QVERIFY(reads->getChildren().size() == 1);
    writes = static_cast<ListNode *>(declaration->getCompilerProperty("writes").get());
    QVERIFY(writes->getNodeType() == AST::List);
    QVERIFY(writes->getChildren().size() == 1);

    declaration = static_cast<DeclarationNode *>(blocks->getChildren().at(2).get());
    QVERIFY(declaration->getName() == "Sig3");

    reads = static_cast<ListNode *>(declaration->getCompilerProperty("reads").get());
    QVERIFY(reads->getNodeType() == AST::List);
    QVERIFY(reads->getChildren().size() == 0);
    writes = static_cast<ListNode *>(declaration->getCompilerProperty("writes").get());
    QVERIFY(writes->getNodeType() == AST::List);
    QVERIFY(writes->getChildren().size() == 1);

    declaration = static_cast<DeclarationNode *>(blocks->getChildren().at(3).get());
    QVERIFY(declaration->getName() == "Sig4");

    reads = static_cast<ListNode *>(declaration->getCompilerProperty("reads").get());
    QVERIFY(reads->getNodeType() == AST::List);
    QVERIFY(reads->getChildren().size() == 1);
    writes = static_cast<ListNode *>(declaration->getCompilerProperty("writes").get());
    QVERIFY(writes->getNodeType() == AST::List);
    QVERIFY(writes->getChildren().size() == 0);


//    module Test3 {

//        streams: [
//            Sig1 >> Sig2;
//            Sig2 >> Sig3;
//            Sig4 >> Sig3;
//        ]
//        streams: [ # Duplicate port names should trigger error, not crash
//            Sig1 >> Sig2;
//            Sig2 >> Sig3;
//            Sig4 >> Sig3;
//        ]
//    }

}

void ParserTest::testModuleDomains()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/12_modules_domains.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

    DeclarationNode *block = static_cast<DeclarationNode *>(tree->getChildren()[1].get());
    QVERIFY(block->getNodeType() == AST::Declaration);

    // Both the input and output blocks should be autodeclared to belong to the output domain
    ListNode *blockList = static_cast<ListNode *>(block->getPropertyValue("blocks").get());
    QVERIFY(blockList->getNodeType() == AST::List);
    DeclarationNode *internalBlock = CodeValidator::findDeclaration(std::string("Input"),
    {{block->getName(), blockList->getChildren()}}, nullptr).get();
    QVERIFY(internalBlock->getNodeType() == AST::Declaration);
    auto domainValue= static_pointer_cast<PortPropertyNode>(internalBlock->getPropertyValue("domain"));
    QVERIFY(domainValue);
    QVERIFY(domainValue->getNodeType() == AST::PortProperty);
    QVERIFY(domainValue->getName() == "OutputPort");
    QVERIFY(domainValue->getPortName() == "domain");

    auto reads = internalBlock->getCompilerProperty("reads")->getChildren();
    auto writes = internalBlock->getCompilerProperty("writes")->getChildren();

    QVERIFY(reads.size() == 1);
    QVERIFY(writes.size() == 0);

    auto readDomain = static_pointer_cast<PortPropertyNode>(reads.at(0));
    QVERIFY(readDomain->getNodeType() == AST::PortProperty);
    QVERIFY(readDomain->getPortName() == "domain");
    QVERIFY(readDomain->getName() == "OutputPort");

    internalBlock = CodeValidator::findDeclaration(std::string("Output"), {{block->getName(), blockList->getChildren()}},
                                    nullptr).get();
    QVERIFY(internalBlock->getNodeType() == AST::Declaration);
    domainValue= static_pointer_cast<PortPropertyNode>(internalBlock->getPropertyValue("domain"));
    QVERIFY(domainValue);
    QVERIFY(domainValue->getNodeType() == AST::PortProperty);
    QVERIFY(domainValue->getName() == "OutputPort");
    QVERIFY(domainValue->getPortName() == "domain");

    reads = internalBlock->getCompilerProperty("reads")->getChildren();
    writes = internalBlock->getCompilerProperty("writes")->getChildren();

    QVERIFY(reads.size() == 0);
    QVERIFY(writes.size() == 1);

    auto writeDomain = static_pointer_cast<PortPropertyNode>(writes.at(0));
    QVERIFY(writeDomain->getNodeType() == AST::PortProperty);
    QVERIFY(writeDomain->getPortName() == "domain");
    QVERIFY(writeDomain->getName() == "OutputPort");

//    ListNode *streamList = static_cast<ListNode *>(block->getPropertyValue("streams").get());
//    ValueNode *constant = static_cast<ValueNode *>(streamList->getChildren()[0]->getChildren()[0].get());
//    QVERIFY(constant->getNodeType() == AST::Int);
//    QVERIFY(constant->getDomain());
//    PortPropertyNode *domain = static_cast<PortPropertyNode *>(constant->getDomain().get());
//    QVERIFY(domain->getNodeType() == AST::PortProperty);
//    QVERIFY(domain->getPortName() == "domain");
//    QVERIFY(domain->getName() == "OutputPort");
}

void ParserTest::testConnectionErrors()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/15_connection_errors.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);

    QList<LangError> errors = generator.getErrors();
    QVERIFY(errors.size() > 0);
    LangError error = errors.at(0);
    QVERIFY(error.type == LangError::StreamMemberSizeMismatch);
}

void ParserTest::testPortTypeValidation()
{
// TODO: Check validation of bundles
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P07_type_validation.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);

    QList<LangError> errors = generator.getErrors();
    LangError error = errors.at(0);
//    QVERIFY(error.lineNumber == 55);
    QVERIFY(error.errorTokens[0] == "testType");
    QVERIFY(error.errorTokens[1] == "stringPort");
//    QVERIFY(error.errorTokens[2] == "CRP");
//    QVERIFY(error.errorTokens[3] == "CSP");
    error = errors.at(1);
//    QVERIFY(error.lineNumber == 56);
    QVERIFY(error.errorTokens[0] == "testType");
    QVERIFY(error.errorTokens[1] == "floatPort");
//    QVERIFY(error.errorTokens[2] == "CSP");
//    QVERIFY(error.errorTokens[3] == "CRP");
    error = errors.at(2);
//    QVERIFY(error.lineNumber == 57);
    QVERIFY(error.errorTokens[0] == "testType");
    QVERIFY(error.errorTokens[1] == "nonePort");
//    QVERIFY(error.errorTokens[2] == "CIP");
//    QVERIFY(error.errorTokens[3] == "none");
    error = errors.at(3);
//    QVERIFY(error.lineNumber == 58);
    QVERIFY(error.errorTokens[0] == "testType");
    QVERIFY(error.errorTokens[1] == "builtinTypePort");
//    QVERIFY(error.errorTokens[2] == "CRP");
//    QVERIFY(error.errorTokens[3] == "signal");
}

void ParserTest::testLibraryObjectInsertion()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/E05_library_objects.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

    std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(std::string("AudioIn"), {}, tree);
    QVERIFY(decl);
    QVERIFY(decl->getObjectType() == "_hwInput");

//    QList<LangError> errors;
    decl = CodeValidator::findTypeDeclarationByName("signal", {}, tree);
    QVERIFY(decl);
//    QVERIFY(errors.isEmpty());

    // the module type should get brought in by Level()
    decl = CodeValidator::findTypeDeclarationByName("module", {}, tree);
    QVERIFY(decl);
//    QVERIFY(errors.isEmpty());

    // Oscillator is not used and there should be no declaration for it
    decl = CodeValidator::findDeclaration(std::string("Oscillator"), {}, tree);
    QVERIFY(!decl);
}

void ParserTest::testContextDomain()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/E06_context_domain.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
//    QVERIFY(generator.isValid());
    auto mod = CodeValidator::findDeclaration(std::string("TestMod"), {}, tree);
    QVERIFY(mod);

    auto loop = CodeValidator::findDeclaration(std::string("TestLoop"),
    {{mod->getName(), mod->getPropertyValue("blocks")->getChildren()}}, tree);
    QVERIFY(loop);
}

void ParserTest::testDomains()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P06_domains.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

//    [ TestMod(value: 10) , 1024.0] >> TestMod2(value: 1.5) >> ValueInOSCDomain;;

    StreamNode *stream = static_cast<StreamNode *>(tree->getChildren()[4].get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    ListNode *list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    FunctionNode *func = static_cast<FunctionNode *>(list->getChildren()[0].get());
    ValueNode *domainName = static_cast<ValueNode *>(func->getDomain().get());
//    QVERIFY(domainName);
//    QVERIFY(domainName->getNodeType() == AST::String);
//    QVERIFY(domainName->getStringValue() == "OSCInDomain");
    stream = static_cast<StreamNode *>(stream->getRight().get());
    func = static_cast<FunctionNode *>(stream->getLeft().get());
//    domainName = static_cast<ValueNode *>(func->getDomain().get());
//    QVERIFY(domainName->getNodeType() == AST::String);
//    QVERIFY(domainName->getStringValue() == "OSCInDomain");

    //    TestMod(value: ValueInOSCDomain) >> AudioOut;

    stream = static_cast<StreamNode *>(tree->getChildren()[7].get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    func = static_cast<FunctionNode *>(stream->getLeft().get());
    QVERIFY(func->getNodeType() == AST::Function);
    domainName = static_cast<ValueNode *>(func->getDomain().get());
//    QVERIFY(domainName->getNodeType() == AST::String);
//    QVERIFY(domainName->getStringValue() == "AudioDomain");

    // Check if "none" domain is set to platform domain
//    signal Modulator {}
//    TestMod(value: 5) >> Modulator;
//    TestMod(value:Modulator) >> AudioOut;

    std::shared_ptr<DeclarationNode> block = CodeValidator::findDeclaration(std::string("Modulator"), {}, tree);
    QVERIFY(block->getNodeType() == AST::Declaration);
    BlockNode *domain = static_cast<BlockNode *>(block->getDomain().get());
    QVERIFY(domain->getNodeType() == AST::Block);
    QVERIFY(domain->getName() == "AudioDomain");

    // AudioIn[1] >> TestMod2(value: 0.1) >> Signal;

    block = CodeValidator::findDeclaration(std::string("Signal"), {}, tree);
    QVERIFY(block->getNodeType() == AST::Declaration);
    auto domainBlock = static_cast<BlockNode *>(block->getDomain().get());
    QVERIFY(domainBlock->getNodeType() == AST::Block);
    QVERIFY(domainBlock->getName() == "AudioDomain");

//# This has two domain changes. Currently this is split into two
//# separate streams as it makes code generation easier.
//AudioIn[1] >> ValueInOSCDomain >> AudioOut[2];

    // Check slicing of domains when domain changes
    stream = static_cast<StreamNode *>(tree->getChildren()[10].get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    auto bundle = static_cast<BundleNode *>(stream->getLeft().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioIn");
    auto blocknode = static_cast<BlockNode *>(stream->getRight().get());
    QVERIFY(blocknode->getNodeType() == AST::Block);
    QVERIFY(blocknode->getName() == "ValueInOSCDomain");

//    stream = static_cast<StreamNode *>(tree->getChildren()[11].get());
//    QVERIFY(stream->getNodeType() == AST::Stream);
//    blocknode = static_cast<BlockNode *>(stream->getLeft().get());
//    QVERIFY(blocknode->getNodeType() == AST::Block);
//    QVERIFY(blocknode->getName() == "ValueInOSCDomain");
//    bundle = static_cast<BundleNode *>(stream->getRight().get());
//    QVERIFY(bundle->getNodeType() == AST::Bundle);
//    QVERIFY(bundle->getName() == "AudioOut");

}


void ParserTest::testModules()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/11_modules.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

    DeclarationNode *moduleNode = static_cast<DeclarationNode *>(tree->getChildren().at(1).get());
    QVERIFY(moduleNode->getName() == "SizeTest");
    ListNode *blockList = static_cast<ListNode *>(moduleNode->getPropertyValue("blocks").get());
    QVERIFY(blockList->getNodeType() == AST::List);
    for(size_t i = 0; i < blockList->getChildren().size(); i++) {
        AST *member = blockList->getChildren().at(i).get();
        if (member->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(member);
            if (block->getObjectType() == "constant") {

            }
        }
    }

    moduleNode = static_cast<DeclarationNode *>(tree->getChildren().at(2).get());
    QVERIFY(moduleNode->getName() == "BlocksTest");
    blockList = static_cast<ListNode *>(moduleNode->getPropertyValue("blocks").get());
    QVERIFY(blockList->getNodeType() == AST::List);
    QStringList blockNames;
    blockNames << "Test";
    blockNames << "Input";
    blockNames << "Output";
    blockNames << "AutoDeclared";
    for(auto name: blockNames) {
        auto decl = CodeValidator::findDeclaration(name,
        {{moduleNode->getName(), blockList->getChildren()}}, nullptr);
        QVERIFY(decl);
    }
    // Check to make sure input and output domains have propagated correctly
    for (auto blockNode:  blockList->getChildren()) {
        DeclarationNode *block = static_cast<DeclarationNode *>(blockNode.get());
        QVERIFY(block->getDomain());
        QVERIFY(block->getDomain()->getNodeType() == AST::PortProperty);
        auto domain = static_cast<PortPropertyNode *>(block->getDomain().get());
        if (block->getName() == "Input") {

            QVERIFY(domain->getPortName() == "domain");
            QVERIFY(domain->getName() == "OutputPort");
        } else {
            QVERIFY(domain->getPortName() == "domain");
            QVERIFY(domain->getName() == "OutputPort");
        }

    }
}

void ParserTest::testImport()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P04_import.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

//    ImportNode *import = static_cast<ImportNode *>(tree->getChildren().at(5));
//    QVERIFY(import->getNodeType() == AST::Import);
//    QVERIFY(import->importName() == "Filter");
//    QVERIFY(import->getScopeLevels() == 2);
//    QVERIFY(import->getScopeAt(0) == "Platform");
//    QVERIFY(import->getScopeAt(1) == "Filters");

    tree = AST::parseFile(QString(QFINDTESTDATA("data/P05_import_fail.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator2(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(!generator2.isValid());
    QList<LangError> errors = generator2.getErrors();
    LangError error = errors.front();
    QVERIFY(error.type == LangError::UndeclaredSymbol);
    QVERIFY(error.lineNumber == 6);
    QVERIFY(error.errorTokens[0] == "LowPass");

    errors.removeFirst();
    error = errors.front();
    QVERIFY(error.type == LangError::UndeclaredSymbol);
    QVERIFY(error.lineNumber == 9);
    QVERIFY(error.errorTokens[0] == "F::LowPass");
}

void ParserTest::testConstantResolution()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/E01_constant_res.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

    DeclarationNode *block = static_cast<DeclarationNode *>(tree->getChildren().at(4).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    ValueNode *value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(qFuzzyCompare(value->getRealValue(), -0.1));

    block = static_cast<DeclarationNode *>(tree->getChildren().at(5).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(6).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 3);

    //    constant ConstInt3 {value: ConstInt1 - 3}
    //    constant ConstInt4 {value: ConstInt1 * 4}
    //    constant ConstInt5 {value: ConstInt1 / 2}

    block = static_cast<DeclarationNode *>(tree->getChildren().at(7).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == -1);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(8).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 8);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(9).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    //# Float and int operations in expressions
    //constant ConstFloat1 {value: Const1 + 1}
    //constant ConstFloat2 {value: Const1 - 3}
    //constant ConstFloat3 {value: Const1 * 4}
    //constant ConstFloat4 {value: Const1 / 2}

    block = static_cast<DeclarationNode *>(tree->getChildren().at(10).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 3.0);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(11).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == -1.0);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(12).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 8.0);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(13).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.0);

    //# Float and float operations in expressions
    //constant ConstFloat5 {value: Const1 + 1.0}
    //constant ConstFloat6 {value: Const1 - 3.0}
    //constant ConstFloat7 {value: Const1 * 4.0}
    //constant ConstFloat8 {value: Const1 / 2.0}

    block = static_cast<DeclarationNode *>(tree->getChildren().at(14).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 3.0);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(15).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == -1.0);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(16).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 8.0);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(17).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.0);

    block = static_cast<DeclarationNode *>(tree->getChildren().at(18).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Real);

    QVERIFY(qFuzzyCompare(value->getRealValue(), 2.0 + (3.1 * 0.1)));

    // TODO Verify Unary operators
    //# Unary operators
    //constant ConstInt6 {value: -ConstInt1}
    //#constant ConstInt7 {value: (ConstInt1 * 15) | 16}
    //#constant ConstInt8 {value: (ConstInt1 * 15) & 8}
    //#constant ConstInt9 {value: ~(ConstInt1 * 15)}

    block = static_cast<DeclarationNode *>(tree->getChildren().at(19).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    value = static_cast<ValueNode *>(block->getPropertyValue("value").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == -2);


//    Unknown1 * (2 * 3.5) >> Unknown2;

    StreamNode * stream = static_cast<StreamNode *>(tree->getChildren().at(20).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    ExpressionNode *expr = static_cast<ExpressionNode *>(stream->getLeft().get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    value = static_cast<ValueNode *>(expr->getRight().get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 7.0);

//    [Const1 + (2.0 * Const3), Const1 + (Const2 * 1.0)] >> A;

    stream = static_cast<StreamNode *>(tree->getChildren().at(22).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    ListNode *list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    value = static_cast<ValueNode *>(list->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 2.2);
    value = static_cast<ValueNode *>(list->getChildren().at(1).get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 5.1);

//    constant AutoBlock {value: 440 }
//    module Test {
//      ports: [
//        port InputPort {
//          block: AutoBlock
//        }
//       ]
//    }
    DeclarationNode *module = static_cast<DeclarationNode *>(tree->getChildren().at(24).get());
    QVERIFY(module->getNodeType() == AST::Declaration);
    QVERIFY(module->getObjectType() == "module");
    ListNode *ports = static_cast<ListNode *>(module->getPropertyValue("ports").get());
    QVERIFY(ports->getNodeType() == AST::List);
    BlockNode *portBlock = static_cast<BlockNode *>(static_cast<DeclarationNode *>(ports->getChildren().at(0).get())->getPropertyValue("block").get());
    QVERIFY(portBlock->getNodeType() == AST::Block);
    QVERIFY(portBlock->getName() == "AutoBlock");
}

void ParserTest::testStreamRates()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/E04_rates.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(generator.isValid());
    QVERIFY(generator.platformIsValid());

    vector<ASTNode> nodes = tree->getChildren();

    // AudioIn[1] >> Signal >> AudioOut[1];
    StreamNode *stream = static_cast<StreamNode *>(nodes.at(2).get());
    QVERIFY(stream->getNodeType() == AST::Stream);

    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);

    std::shared_ptr<BlockNode> name = static_pointer_cast<BlockNode>(stream->getLeft());
    QVERIFY(name->getNodeType() == AST::Block);
    QVERIFY(CodeValidator::getNodeRate(name, {}, tree) == 44100);

    std::shared_ptr<BundleNode> bundle = static_pointer_cast<BundleNode>(stream->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(CodeValidator::getNodeRate(bundle, {}, tree) == 44100);

    // Rate1 >> Rate2 >> Output1;
    stream = static_cast<StreamNode *>(nodes.at(6).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 22050);

    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 11025);

    std::shared_ptr<BlockNode> nameNode = static_pointer_cast<BlockNode>(stream->getRight());
    QVERIFY(nameNode->getNodeType() == AST::Block);
    QVERIFY(CodeValidator::getNodeRate(nameNode, {}, tree) == 11025);

    // Signal1 >> Rate1 >> Signal2 >> Rate2 >> GetAudioRate >> Output2;
    stream = static_cast<StreamNode *>(nodes.at(7).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 22050);

    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 22050);

    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 11025);

    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 11025);

    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 11025);

    name = static_pointer_cast<BlockNode>(stream->getRight());
    QVERIFY(name->getNodeType() == AST::Block);
    QVERIFY(CodeValidator::getNodeRate(name, {}, tree) == 11025);

    //    Oscillator() >> Rate1 >> LowPass() >> Output3;
    stream = static_cast<StreamNode *>(nodes.at(8).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 22050);
    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 22050);
    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 44100);
    QVERIFY(CodeValidator::getNodeRate(stream->getRight(), {}, tree) == 44100);

//    Oscillator() >> AudioOut;
    stream = static_cast<StreamNode *>(nodes.at(9).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(CodeValidator::getNodeRate(stream->getLeft(), {}, tree) == 44100);
    QVERIFY(CodeValidator::getNodeRate(stream->getRight(), {}, tree) == 44100);
}

void ParserTest::testStreamExpansion()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/E02_stream_expansions.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());
    QVERIFY(generator.platformIsValid());

    //# This should be expanded into lists of equal size:
    //AudioIn >> Level(gain: 1.5) >> AudioOut;

    vector<ASTNode> nodes = tree->getChildren();
    QVERIFY(nodes.size() > 3);
    StreamNode *stream = static_cast<StreamNode *>(nodes.at(1).get());
    QVERIFY(stream->getNodeType() == AST::Stream);

    ListNode *list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);

    BundleNode *bundle = static_cast<BundleNode *>(list->getChildren()[0].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioIn");
    ListNode *index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    ValueNode *value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(list->getChildren()[1].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioIn");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    StreamNode *right = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(right->getNodeType() == AST::Stream);
    list = static_cast<ListNode *>(right->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);
    FunctionNode *func = static_cast<FunctionNode *>(list->getChildren()[0].get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Level");
    func = static_cast<FunctionNode *>(list->getChildren()[1].get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Level");

    list = static_cast<ListNode *>(right->getRight().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);

    bundle = static_cast<BundleNode *>(list->getChildren()[0].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioOut");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(list->getChildren()[1].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioOut");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);


    //signal In[2] {
    //    default: [3,4.5]
    //}

    //signal Out[2] {
    //    default: [5,6.5]
    //}
    //#Expanded to lists of size 2
    //In >> Out;

    stream = static_cast<StreamNode *>(nodes.at(4).get());
    QVERIFY(stream->getNodeType() == AST::Stream);

    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);

    bundle = static_cast<BundleNode *>(list->getChildren()[0].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "In");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(list->getChildren()[1].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "In");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);


    list = static_cast<ListNode *>(stream->getRight().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);

    bundle = static_cast<BundleNode *>(list->getChildren()[0].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Out");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(list->getChildren()[1].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Out");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    //# Signals should be delcared as blocks of size 2
    //InSignal2 >> [Level(gain: 1.0), Level(gain: 1.0)] >> OutSignal2;

    vector<StreamNode *> streams = CodeValidator::getStreamsAtLine(tree, 17);
    std::shared_ptr<DeclarationNode> decl = CodeValidator::findDeclaration(std::string("OutSignal2"), {}, tree);

    QVERIFY(decl->getNodeType() == AST::BundleDeclaration);
    bundle = decl->getBundle().get();
    index = bundle->index().get();

    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    stream = streams.at(0);

    BlockNode *name = static_cast<BlockNode *>(stream->getLeft().get());
    QVERIFY(name->getNodeType() == AST::Block);
    QVERIFY(name->getName() == "InSignal2");

    right = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(right->getNodeType() == AST::Stream);
    list = static_cast<ListNode *>(right->getRight().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);

    bundle = static_cast<BundleNode *>(list->getChildren()[0].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "OutSignal2");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(list->getChildren()[1].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "OutSignal2");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    //# Level() should be duplicated and OutSignal3 declared as a bundle size 2
    //    In >> Level(gain: 1.0) >> OutSignal3;
    streams = CodeValidator::getStreamsAtLine(tree, 20);
    QVERIFY(streams.size() == 1);
    stream = streams.at(0);

    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);

    decl = CodeValidator::findDeclaration(std::string("OutSignal3"), {}, tree);
    QVERIFY(decl->getNodeType() == AST::BundleDeclaration);
    ListNode *bundleIndex = static_cast<ListNode *>(decl->getBundle()->index().get());
    QVERIFY(bundleIndex->size() == 1);
    ValueNode *declSize = static_cast<ValueNode *>(bundleIndex->getChildren()[0].get());
    QVERIFY(declSize->getNodeType() == AST::Int);
    QVERIFY(declSize->getIntValue() == 2);

    func = static_cast<FunctionNode *>(stream->getRight()->getChildren()[0]->getChildren()[0].get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Level");
    auto newfunc = static_cast<FunctionNode *>(stream->getRight()->getChildren()[0]->getChildren()[1].get());
    QVERIFY(newfunc != func);
    QVERIFY(newfunc->getNodeType() == AST::Function);
    QVERIFY(newfunc->getName() == "Level");

    list = static_cast<ListNode *>(stream->getRight()->getChildren()[1].get());
    QVERIFY(list->getChildren().size() == 2);
    bundle = static_cast<BundleNode *>(list->getChildren()[0].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "OutSignal3");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(list->getChildren()[1].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "OutSignal3");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

//    signal StereoOut[2] { }
//    MonoSignal >> Level(gain: 1.0) >> StereoOut;
    streams = CodeValidator::getStreamsAtLine(tree, 24);
    QVERIFY(streams.size() == 1);
    stream = streams.at(0);
    name = static_cast<BlockNode *>(stream->getLeft().get());
    QVERIFY(name->getNodeType() == AST::Block);
    QVERIFY(name->getName() == "MonoSignal");
    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    func = static_cast<FunctionNode *>(stream->getLeft().get());
    QVERIFY(func->getNodeType() == AST::Function);
    list = static_cast<ListNode *>(stream->getRight().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);

//    MonoSignal2 >> Level(gain: [1.0, 1.0]) >> StereoOut;
    streams = CodeValidator::getStreamsAtLine(tree, 27);
    QVERIFY(streams.size() == 1);
    stream = streams.at(0);
    name = static_cast<BlockNode *>(stream->getLeft().get());
    QVERIFY(name->getNodeType() == AST::Block);
    QVERIFY(name->getName() == "MonoSignal2");
    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);
    func = static_cast<FunctionNode *>(list->getChildren()[0].get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Level");
    vector<std::shared_ptr<PropertyNode>> props = func->getProperties();
    QVERIFY(props[0]->getName() == "gain" );
    value = static_cast<ValueNode *>(props[0]->getValue().get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.0);
    func = static_cast<FunctionNode *>(list->getChildren()[1].get());
    QVERIFY(func->getNodeType() == AST::Function);
    props = func->getProperties();
    QVERIFY(props[0]->getName() == "gain" );
    value = static_cast<ValueNode *>(props[0]->getValue().get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 2.0);

    list = static_cast<ListNode *>(stream->getRight().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getChildren().size() == 2);
    bundle = static_cast<BundleNode *>(list->getChildren()[0].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    index = bundle->index().get();
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren()[0].get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(list->getChildren()[1].get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    index = bundle->index().get();
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren()[0].get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

//    Out >> Level(gain: 1.0) >> NewSignal;
//    NewSignal >> Level(gain: 1.0) >> NewSignal2;

    streams = CodeValidator::getStreamsAtLine(tree, 30);
    QVERIFY(streams.size() == 1);
    stream = streams.at(0);
    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->size() == 2);
    bundle = static_cast<BundleNode *>(list->getChildren().at(0).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Out");
    ListNode *indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(list->getChildren().at(1).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Out");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->size() == 2);
    func = static_cast<FunctionNode *>(list->getChildren().at(0).get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Level");
    value = static_cast<ValueNode *>(func->getPropertyValue("gain").get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.0);
    func = static_cast<FunctionNode *>(list->getChildren().at(1).get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Level");
    value = static_cast<ValueNode *>(func->getPropertyValue("gain").get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.0);

    list = static_cast<ListNode *>(stream->getRight().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->size() == 2);
    bundle = static_cast<BundleNode *>(list->getChildren().at(0).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "NewSignal");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(list->getChildren().at(1).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "NewSignal");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    streams = CodeValidator::getStreamsAtLine(tree, 31);
    QVERIFY(streams.size() == 1);
    stream = streams.at(0);
    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->size() == 2);
    bundle = static_cast<BundleNode *>(list->getChildren().at(0).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "NewSignal");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(list->getChildren().at(1).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "NewSignal");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->size() == 2);
    func = static_cast<FunctionNode *>(list->getChildren().at(0).get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Level");
    value = static_cast<ValueNode *>(func->getPropertyValue("gain").get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.0);
    func = static_cast<FunctionNode *>(list->getChildren().at(1).get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Level");
    value = static_cast<ValueNode *>(func->getPropertyValue("gain").get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.0);

    list = static_cast<ListNode *>(stream->getRight().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->size() == 2);
    bundle = static_cast<BundleNode *>(list->getChildren().at(0).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "NewSignal2");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(list->getChildren().at(1).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "NewSignal2");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    //    Oscillator(frequency: [440,2] amplitude: 1.0) >> Osc;

    stream = static_cast<StreamNode *>(nodes.at(13).get());
    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->size() == 2);
    func = static_cast<FunctionNode *>(list->getChildren().at(0).get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Oscillator");
    value = static_cast<ValueNode *>(func->getPropertyValue("frequency").get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 440);
    func = static_cast<FunctionNode *>(list->getChildren().at(1).get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Oscillator");
    value = static_cast<ValueNode *>(func->getPropertyValue("frequency").get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    list = static_cast<ListNode *>(stream->getRight().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->size() == 2);
    bundle = static_cast<BundleNode *>(list->getChildren().at(0).get());
    QVERIFY(bundle->getName() == "Osc");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(list->getChildren().at(1).get());
    QVERIFY(bundle->getName() == "Osc");
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    streams = CodeValidator::getStreamsAtLine(tree, 22);
}

void ParserTest::testNamespaces()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/E07_namespaces.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());
    QVERIFY(generator.platformIsValid());
}

void ParserTest::testPlatformCommonObjects()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P01_platform_objects.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();
    QVERIFY(generator.platformIsValid());

    QVERIFY(errors.size() == 5);

    QVERIFY(errors[0].type == LangError::UnknownType);
    QVERIFY(errors[0].lineNumber == 3);
    QVERIFY(errors[0].errorTokens[0] == "invalid");

    QVERIFY(errors[1].type == LangError::InvalidPortType);
    QVERIFY(errors[1].lineNumber == 12);
    QVERIFY(errors[1].errorTokens[0]  == "signal");
    QVERIFY(errors[1].errorTokens[1]  == "meta");
//    QVERIFY(errors[1].errorTokens[2]  == "CIP");

    QVERIFY(errors[2].type == LangError::InvalidPortType);
    QVERIFY(errors[2].lineNumber == 21);
    QVERIFY(errors[2].errorTokens[0]  == "switch");
    QVERIFY(errors[2].errorTokens[1]  == "default");
//    QVERIFY(errors[2].errorTokens[2]  == "CIP");

    QVERIFY(errors[3].type == LangError::InvalidPort);
    QVERIFY(errors[3].lineNumber == 32);
    QVERIFY(errors[3].errorTokens[0]  == "signal");
    QVERIFY(errors[3].errorTokens[1]  == "badproperty");

    QVERIFY(errors[4].type == LangError::InvalidPortType);
    QVERIFY(errors[4].lineNumber == 42);
    QVERIFY(errors[4].errorTokens[0]  == "constant");
    QVERIFY(errors[4].errorTokens[1]  == "value");
//    QVERIFY(errors[4].errorTokens[2]  == "signal");

//    tree->deleteChildren();
//    delete tree;
}

void ParserTest::testValueTypeBundleResolution()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P03_bundle_resolution.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
//    generator.validate();
    QVERIFY(generator.platformIsValid());
    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();

    vector<ASTNode> nodes = tree->getChildren();
    QVERIFY(nodes.at(1)->getNodeType() == AST::BundleDeclaration);
    DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(1).get());
    QVERIFY(block->getObjectType() == "constant");
    std::shared_ptr<BundleNode> bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Integers");
    ListNode *index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    ValueNode *value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    AST *propertyValue = block->getPropertyValue("value").get();
    QVERIFY(propertyValue);
    QVERIFY(propertyValue->getNodeType() == AST::List);
    ListNode *listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getListType() == AST::Int);
    vector<ASTNode>listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 4);

    QVERIFY(nodes.at(2)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(2).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Floats");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    propertyValue = block->getPropertyValue("value").get();
    QVERIFY(propertyValue);
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getListType() == AST::Real);
    listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 4);

    QVERIFY(nodes.at(3)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(3).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Strings");
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    propertyValue = block->getPropertyValue("value").get();
    QVERIFY(propertyValue);
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getListType() == AST::String);
    listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 4);

    QVERIFY(nodes.at(4)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(4).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Int");

    QVERIFY(nodes.at(5)->getNodeType() == AST::Declaration);
    block = static_cast<DeclarationNode *>(nodes.at(5).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getName() == "Value_Meta");

    QVERIFY(nodes.at(6)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(6).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Value_Meta_1");

    QVERIFY(nodes.at(7)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(7).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_1");

    QVERIFY(nodes.at(8)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(8).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_2");

    // The following cases should produce errors

    QVERIFY(nodes.at(9)->getNodeType() == AST::Declaration);
    block = static_cast<DeclarationNode *>(nodes.at(9).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getName() == "Bad_Value_Meta");

    LangError error = errors.takeFirst();
//    QVERIFY(error.type == LangError::InvalidPortType);
//    QVERIFY(error.lineNumber == 41);
//    QVERIFY(error.errorTokens[0] == "constant");
//    QVERIFY(error.errorTokens[1] == "meta");
//    QVERIFY(error.errorTokens[2] == "CRP");

    QVERIFY(nodes.at(10)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(10).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_Mismatch");
//    error = errors.takeFirst();
    QVERIFY(error.type == LangError::BundleSizeMismatch);
    QVERIFY(error.lineNumber == 44);
    QVERIFY(error.errorTokens[0] == "Values_Mismatch");
    QVERIFY(error.errorTokens[1] == "3");
    QVERIFY(error.errorTokens[2] == "4");


    QVERIFY(nodes.at(11)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(11).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_Mismatch_2");
    error = errors.takeFirst();
    QVERIFY(error.lineNumber == 48);
    QVERIFY(error.errorTokens[0] == "Values_Mismatch_2");
    QVERIFY(error.errorTokens[1] == "3");
    QVERIFY(error.errorTokens[2] == "2");

    QVERIFY(nodes.at(12)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(12).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Float");
    QVERIFY(errors.size() > 0);
    error = errors.takeFirst();
    QVERIFY(error.type == LangError::IndexMustBeInteger);
    QVERIFY(error.lineNumber == 52);
    QVERIFY(error.errorTokens[0] == "Array_Float");
//    QVERIFY(error.errorTokens[1] == "CRP");

    QVERIFY(nodes.at(13)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(13).get());
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_String");
    error = errors.takeFirst();
    QVERIFY(error.type == LangError::IndexMustBeInteger);
    QVERIFY(error.lineNumber == 53);
    QVERIFY(error.errorTokens[0] == "Array_String");
//    QVERIFY(error.errorTokens[1] == "CSP");
}

void ParserTest::testValueTypeExpressionResolution()
{

}

void ParserTest::testDuplicates()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P02_check_duplicates.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();

    QVERIFY(errors.size() == 2);
    LangError error = errors.takeFirst();
    QVERIFY(error.type == LangError::DuplicateSymbol);
    QVERIFY(error.lineNumber == 12);
    QVERIFY(error.errorTokens[0] == "Const");
    QVERIFY(error.errorTokens[2] == "3");

    error = errors.takeFirst();
    QVERIFY(error.type == LangError::DuplicateSymbol);
    QVERIFY(error.lineNumber == 18);
    QVERIFY(error.errorTokens[0] == "Size");
    QVERIFY(error.errorTokens[2] == "7");
}

void ParserTest::testLists()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/09_lists.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();

    //    constant List_Integer [4] {
    //            value: [ 16, 32, 64, 128 ]
    //    }
    DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(1).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    vector<std::shared_ptr<PropertyNode> > props =  block->getProperties();
    ListNode *list = static_cast<ListNode *>(props.at(0)->getValue().get());
    QVERIFY(list->getNodeType() == AST::List);
    vector<ASTNode> members = list->getChildren();
    QVERIFY(members.size() == 4);
    for(ASTNode member : members) {
        ValueNode *value = static_cast<ValueNode *>(member.get());
        QVERIFY(value->getNodeType() == AST::Int);
    }
    //    constant List_Real [4] {
    //            value: [ 16., 32.1, 64., 128. ]
    //    }
    block = static_cast<DeclarationNode *>(nodes.at(2).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    props =  block->getProperties();
    list = static_cast<ListNode *>(props.at(0)->getValue().get());
    QVERIFY(list->getNodeType() == AST::List);
    members = list->getChildren();
    QVERIFY(members.size() == 4);
    for(ASTNode member : members) {
        ValueNode *value = static_cast<ValueNode *>(member.get());
        QVERIFY(value->getNodeType() == AST::Real);
    }

    //    constant List_Strings [4] {
    //            value: [ '16', "32.1", '64', "128" ]
    //    }
    block = static_cast<DeclarationNode *>(nodes.at(3).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    props =  block->getProperties();
    list = static_cast<ListNode *>(props.at(0)->getValue().get());
    QVERIFY(list->getNodeType() == AST::List);
    members = list->getChildren();
    QVERIFY(members.size() == 4);
    foreach(ASTNode member, members) {
        ValueNode *value = static_cast<ValueNode *>(member.get());
        QVERIFY(value->getNodeType() == AST::String);
    }

    //    constant List_Switches [4] {
    //            value: [ on, off, on, on ]
    //    }
    block = static_cast<DeclarationNode *>(nodes.at(4).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    props =  block->getProperties();
    list = static_cast<ListNode *>(props.at(0)->getValue().get());
    QVERIFY(list->getNodeType() == AST::List);
    members = list->getChildren();
    QVERIFY(members.size() == 4);
    for(ASTNode member : members) {
        ValueNode *value = static_cast<ValueNode *>(member.get());
        QVERIFY(value->getNodeType() == AST::Switch);
    }

    //    constant List_Names [4] {
    //            value: [ Name1, Name2, Name3, Name4 ]
    //    }
    block = static_cast<DeclarationNode *>(nodes.at(5).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    props =  block->getProperties();
    list = static_cast<ListNode *>(props.at(0)->getValue().get());
    QVERIFY(list->getNodeType() == AST::List);
    members = list->getChildren();
    QVERIFY(members.size() == 4);
    foreach(ASTNode member, members) {
        BlockNode *value = static_cast<BlockNode *>(member.get());
        QVERIFY(value->getNodeType() == AST::Bundle);
    }

    //    constant List_Namespaces [4] {
    //            value: [ ns.Name1, ns.Name2, ns.Name3, ns.Name4 ]
    //    }
    block = static_cast<DeclarationNode *>(nodes.at(6).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    props =  block->getProperties();
    list = static_cast<ListNode *>(props.at(0)->getValue().get());
    QVERIFY(list->getNodeType() == AST::List);
    members = list->getChildren();
    QVERIFY(members.size() == 4);
    foreach(ASTNode member, members) {
        BlockNode *value = static_cast<BlockNode *>(member.get());
        QVERIFY(value->getNodeType() == AST::Block);
        QVERIFY(value->getScopeLevels() == 1);
        QVERIFY(value->getScopeAt(0) == "Ns");
    }

    //    block BlockName {
    //    property: [ blockType1 BlockName2 { property: "value" },
    //                blockType1 BlockName3 { value: 1.0 } ]
    //    }
    block = static_cast<DeclarationNode *>(nodes.at(7).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    props =  block->getProperties();
    list = static_cast<ListNode *>(props.at(0)->getValue().get());
    QVERIFY(list->getNodeType() == AST::List);
    members = list->getChildren();
    QVERIFY(members.size() == 2);
    DeclarationNode *internalBlock = static_cast<DeclarationNode *>(members[0].get());
    QVERIFY(internalBlock->getNodeType() == AST::Declaration);
    QVERIFY(internalBlock->getObjectType() == "blockType1");
    QVERIFY(internalBlock->getName() == "BlockName2");

    internalBlock = static_cast<DeclarationNode *>(members[1].get());
    QVERIFY(internalBlock->getNodeType() == AST::Declaration);
    QVERIFY(internalBlock->getObjectType() == "blockType2");
    QVERIFY(internalBlock->getName() == "BlockName3");

//    constant IntegerList [3] {
//            value: [[ 9, 8, 7 ] , [ 6, 5, 4 ] , [ 3, 2, 1 ] ]
//            meta:	'List of lists'
//    }
    block = static_cast<DeclarationNode *>(nodes.at(8).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    props =  block->getProperties();
    list = static_cast<ListNode *>(props.at(0)->getValue().get());
    QVERIFY(list->getNodeType() == AST::List);
    members = list->getChildren();
    QVERIFY(members.size() == 3);
    QVERIFY(members.at(0)->getNodeType() == AST::List);
    QVERIFY(members.at(1)->getNodeType() == AST::List);
    QVERIFY(members.at(2)->getNodeType() == AST::List);

//    [ In >> Out; OtherIn >> OtherOut;] >> [Out1, Out2];
//    [ In >> Out; OtherIn >> OtherOut;] >> Out;
//    Out >> [ In >> Out; OtherIn >> OtherOut;];
    //FIXME allow this
//    StreamNode *stream = static_cast<StreamNode *>(nodes.at(9));
//    QVERIFY(stream->getNodeType() == AST::Stream);
//    list = static_cast<ListNode *>(stream->getLeft());
//    QVERIFY(list->getNodeType() == AST::List);
//    QVERIFY(list->getChildren().size() == 2);
//    QVERIFY(list->getChildren()[0]->getNodeType() == AST::Stream);
//    QVERIFY(list->getChildren()[1]->getNodeType() == AST::Stream);
//    stream = static_cast<StreamNode *>(nodes.at(10));
//    list = static_cast<ListNode *>(stream->getLeft());
//    QVERIFY(list->getNodeType() == AST::List);
//    QVERIFY(list->getChildren().size() == 2);
//    QVERIFY(list->getChildren()[0]->getNodeType() == AST::Stream);
//    QVERIFY(list->getChildren()[1]->getNodeType() == AST::Stream);
//    stream = static_cast<StreamNode *>(nodes.at(11));
//    list = static_cast<ListNode *>(stream->getRight());
//    QVERIFY(list->getNodeType() == AST::List);
//    QVERIFY(list->getChildren().size() == 2);
//    QVERIFY(list->getChildren()[0]->getNodeType() == AST::Stream);
//    QVERIFY(list->getChildren()[1]->getNodeType() == AST::Stream);
}

void ParserTest::testScope()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P08_scope.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();

//    import Namespace
//    import Namespace as Namespace
//    import NameSpace as Ns

    ImportNode *import = static_cast<ImportNode *>(nodes.at(0).get());
    QVERIFY(import->getNodeType() == AST::Import);
    QVERIFY(import->importName() == "Namespace");
    QVERIFY(import->importAlias() == "");

    import = static_cast<ImportNode *>(nodes.at(1).get());
    QVERIFY(import->getNodeType() == AST::Import);
    QVERIFY(import->importName() == "Namespace");
    QVERIFY(import->importAlias() == "Namespace");

    import = static_cast<ImportNode *>(nodes.at(2).get());
    QVERIFY(import->getNodeType() == AST::Import);
    QVERIFY(import->importName() == "NameSpace");
    QVERIFY(import->importAlias() == "Ns");

//    Ns::Value >> Constant;
//    Ns::Value_1 + 1.0 >> Constant;
//    1.0 + Ns::Value_1 >> Constant;
//    Ns::Value_1 + Ns::Value_2 >> Constant;
//    Ns::Block_1 >> Ns::Block_2;

    StreamNode *stream = static_cast<StreamNode *>(nodes.at(3).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    BlockNode *node = static_cast<BlockNode *>(stream->getLeft().get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Value");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");

    stream = static_cast<StreamNode *>(nodes.at(4).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    ExpressionNode *expr = static_cast<ExpressionNode *>(stream->getLeft().get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    node = static_cast<BlockNode *>(expr->getLeft().get());
    QVERIFY(node->getName() == "Value_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");

    stream = static_cast<StreamNode *>(nodes.at(5).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    expr = static_cast<ExpressionNode *>(stream->getLeft().get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    node = static_cast<BlockNode *>(expr->getRight().get());
    QVERIFY(node->getName() == "Value_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");

    stream = static_cast<StreamNode *>(nodes.at(6).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    expr = static_cast<ExpressionNode *>(stream->getLeft().get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    node = static_cast<BlockNode *>(expr->getLeft().get());
    QVERIFY(node->getName() == "Value_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    node = static_cast<BlockNode *>(expr->getRight().get());
    QVERIFY(node->getName() == "Value_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");

    stream = static_cast<StreamNode *>(nodes.at(7).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    node = static_cast<BlockNode *>(stream->getLeft().get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Block_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    node = static_cast<BlockNode *>(stream->getRight().get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Block_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");


    //    Ns::Bundle[1] + Ns::Bundle[2] >> Constant;

    stream = static_cast<StreamNode *>(nodes.at(8).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    expr = static_cast<ExpressionNode *>(stream->getLeft().get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    BundleNode *bundle = static_cast<BundleNode *>(expr->getLeft().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    QVERIFY(static_cast<ValueNode *>(bundle->index()->getChildren().at(0).get())->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(expr->getRight().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    QVERIFY(static_cast<ValueNode *>(bundle->index()->getChildren().at(0).get())->getIntValue() == 2);

//    [Ns::Value_1, Ns::Value_2] >> Constants;
//    [Ns::Bundle[1], Ns::Bundle[2]] >> Constants;

    stream = static_cast<StreamNode *>(nodes.at(9).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    ListNode *list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    node = static_cast<BlockNode *>(list->getChildren().at(0).get());
    QVERIFY(node->getName() == "Value_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    node = static_cast<BlockNode *>(list->getChildren().at(1).get());
    QVERIFY(node->getName() == "Value_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");

    stream = static_cast<StreamNode *>(nodes.at(10).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    list = static_cast<ListNode *>(stream->getLeft().get());
    QVERIFY(list->getNodeType() == AST::List);
    bundle = static_cast<BundleNode *>(list->getChildren().at(0).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    QVERIFY(static_cast<ValueNode *>(bundle->index()->getChildren().at(0).get())->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(list->getChildren().at(1).get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    QVERIFY(static_cast<ValueNode *>(bundle->index()->getChildren().at(0).get())->getIntValue() == 2);

//    Ns::Block_1[Ns::Index_1] >> Ns::Block_2 [Ns::Index_2];
//    Ns::Block_1[Ns::Bundle[1]] >> Ns::Block_2 [Ns::Bundle[2]];

    stream = static_cast<StreamNode *>(nodes.at(11).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    bundle = static_cast<BundleNode *>(stream->getLeft().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Block_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    list = static_cast<ListNode *>(bundle->index().get());
    node = static_cast<BlockNode *>(list->getChildren().at(0).get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Index_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    bundle = static_cast<BundleNode *>(stream->getRight().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Block_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    node = static_cast<BlockNode *>(bundle->index()->getChildren().at(0).get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Index_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");

    stream = static_cast<StreamNode *>(nodes.at(12).get());
    QVERIFY(stream->getNodeType() == AST::Stream);

//    Ns::Block_1[Ns::Index_1:Ns::Index_2] >> Ns::Block_2 [Ns::Index_1:Ns::Index_2];

    stream = static_cast<StreamNode *>(nodes.at(13).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    bundle = static_cast<BundleNode *>(stream->getLeft().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Block_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    list = static_cast<ListNode *>(bundle->index().get());
    RangeNode *range = static_cast<RangeNode *>(list->getChildren().at(0).get());
    QVERIFY(range->getNodeType() == AST::Range);
    node = static_cast<BlockNode *>(range->startIndex().get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Index_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    node = static_cast<BlockNode *>(range->endIndex().get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Index_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    bundle = static_cast<BundleNode *>(stream->getRight().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Block_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    list = static_cast<ListNode *>(bundle->index().get());
    range = static_cast<RangeNode *>(list->getChildren().at(0).get());
    QVERIFY(range->getNodeType() == AST::Range);
    node = static_cast<BlockNode *>(range->startIndex().get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Index_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    node = static_cast<BlockNode *>(range->endIndex().get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Index_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");

//    Ns::Block_1[Ns::Bundle[1]:Ns::Bundle[2]] >> Ns::Block_2 [Ns::Bundle[1]:Ns::Bundle[2]];

    stream = static_cast<StreamNode *>(nodes.at(14).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    bundle = static_cast<BundleNode *>(stream->getLeft().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Block_1");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    list = static_cast<ListNode *>(bundle->index().get());
    range = static_cast<RangeNode *>(list->getChildren().at(0).get());
    QVERIFY(range->getNodeType() == AST::Range);
    bundle = static_cast<BundleNode *>(range->startIndex().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    QVERIFY(static_cast<ValueNode *>(bundle->index()->getChildren().at(0).get())->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(range->endIndex().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    QVERIFY(static_cast<ValueNode *>(bundle->index()->getChildren().at(0).get())->getIntValue() == 2);
    bundle = static_cast<BundleNode *>(stream->getRight().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Block_2");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    list = static_cast<ListNode *>(bundle->index().get());
    range = static_cast<RangeNode *>(list->getChildren().at(0).get());
    QVERIFY(range->getNodeType() == AST::Range);
    bundle = static_cast<BundleNode *>(range->startIndex().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    QVERIFY(static_cast<ValueNode *>(bundle->index()->getChildren().at(0).get())->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(range->endIndex().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    QVERIFY(static_cast<ValueNode *>(bundle->index()->getChildren().at(0).get())->getIntValue() == 2);
    bundle = static_cast<BundleNode *>(stream->getRight().get());

    //    Input >> Ns::Function () >> Output;
    //    Input >> Ns::Function ( property: Ns::Value ) >> Output;

    stream = static_cast<StreamNode *>(nodes.at(15).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    FunctionNode *func = static_cast<FunctionNode *>(stream->getLeft().get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Function");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");

    stream = static_cast<StreamNode *>(nodes.at(16).get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    stream = static_cast<StreamNode *>(stream->getRight().get());
    QVERIFY(stream->getNodeType() == AST::Stream);
    func = static_cast<FunctionNode *>(stream->getLeft().get());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "Function");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
    vector<std::shared_ptr<PropertyNode>> props = func->getProperties();
    node = static_cast<BlockNode *>(props.at(0)->getValue().get());
    QVERIFY(node->getNodeType() == AST::Block);
    QVERIFY(node->getName() == "Value");
    QVERIFY(node->getScopeLevels() == 1);
    QVERIFY(node->getScopeAt(0) == "Ns");
}

void ParserTest::testPortProperty()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P09_port_property.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();

    DeclarationNode *block = static_cast<DeclarationNode *>(tree->getChildren().at(1).get());
    QVERIFY(block->getNodeType() == AST::Declaration);
    PortPropertyNode *value = static_cast<PortPropertyNode *>(block->getPropertyValue("property").get());
    QVERIFY(value != nullptr);
    QVERIFY(value->getNodeType() == AST::PortProperty);
    QVERIFY(value->getName() == "Port");
    QVERIFY(value->getPortName() == "rate");
}

void ParserTest::testBundleIndeces()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/07_bundle_indeces.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 23);

    QVERIFY(nodes.at(0)->getNodeType() == AST::Declaration);
    DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(0).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 8);

    QVERIFY(nodes.at(1)->getNodeType() == AST::BundleDeclaration);
    block = static_cast<DeclarationNode *>(nodes.at(1).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 13);
    BundleNode *bundle = block->getBundle().get();
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    QVERIFY(bundle->getLine() == 13);
    ListNode *indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    ValueNode *value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    QVERIFY(value->getLine() == 13);
    QVERIFY(block->getProperties().size() == 2);
    QVERIFY(block->getProperties().at(0)->getNodeType() == AST::Property);
    PropertyNode *property = static_cast<PropertyNode *>(block->getProperties().at(0).get());
    ListNode * listnode = static_cast<ListNode *>(property->getValue().get());
    QVERIFY(listnode->getNodeType() == AST::List);
    QVERIFY(listnode->getChildren().size() == 4);
    QVERIFY(listnode->getLine() == 14);
    value = static_cast<ValueNode *>(listnode->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 16);
    QVERIFY(value->getLine() == 14);
    value = static_cast<ValueNode *>(listnode->getChildren().at(1).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 32);
    QVERIFY(value->getLine() == 14);
    value = static_cast<ValueNode *>(listnode->getChildren().at(2).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 64);
    QVERIFY(value->getLine() == 14);
    value = static_cast<ValueNode *>(listnode->getChildren().at(3).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 128);
    QVERIFY(value->getLine() == 14);

    // constant Array_Parens [ ( CONST * 2 ) + 1 ] {}
    block = static_cast<DeclarationNode *>(nodes.at(6).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 23);
    bundle = block->getBundle().get();
    QVERIFY(bundle->getName() == "Array_Parens");
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    ExpressionNode *expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Add);
    QVERIFY(expr->getLine() == 23);
    ExpressionNode *expr2 = static_cast<ExpressionNode *>(expr->getLeft().get());
    QVERIFY(expr2->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expr2->getLeft()->getNodeType() == AST::Block);
    QVERIFY(expr2->getRight()->getNodeType() == AST::Int);
    QVERIFY(expr2->getLine() == 23);

    // constant Array_Expr [ SIZE [1] + SIZE [1 * 2] ] {}
    block = static_cast<DeclarationNode *>(nodes.at(8).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    QVERIFY(block->getLine() == 26);
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle().get();
    QVERIFY(bundle->getName() == "Array_Expr");
    QVERIFY(bundle->getLine() == 26);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Add);
    QVERIFY(expr->getLine() == 26);
    bundle = static_cast<BundleNode *>(expr->getLeft().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(expr->getRight().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expr->getLeft()->getNodeType() == AST::Int);
    QVERIFY(expr->getRight()->getNodeType() == AST::Int);
    QVERIFY(expr->getLine() == 26);

    // constant Array_Expr2 [ SIZE [1] / SIZE [1 - 2] ] {}
    block = static_cast<DeclarationNode *>(nodes.at(9).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 27);
    bundle = block->getBundle().get();
    QVERIFY(bundle->getName() == "Array_Expr2");
    QVERIFY(bundle->getChildren().size() == 1);
    QVERIFY(bundle->getLine() == 27);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Divide);
    QVERIFY(expr->getLine() == 27);
    bundle = static_cast<BundleNode *>(expr->getLeft().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    QVERIFY(bundle->getLine() == 27);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    QVERIFY(value->getLine() == 27);
    bundle = static_cast<BundleNode *>(expr->getRight().get());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getLine() == 27);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Subtract);
    QVERIFY(expr->getLeft()->getNodeType() == AST::Int);
    QVERIFY(expr->getRight()->getNodeType() == AST::Int);
    QVERIFY(expr->getLine() == 27);
}

void ParserTest::testBasicNoneSwitch()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/06_basic_noneswitch.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 2);

    QVERIFY(nodes.at(0)->getNodeType() == AST::Declaration);
    DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(0).get());
    QVERIFY(block->getObjectType() == "object");
    vector<std::shared_ptr<PropertyNode>> properties = block->getProperties();
    QVERIFY(properties.size() == 3);
    QVERIFY(properties.at(0)->getName() == "prop1");
    ValueNode *value = static_cast<ValueNode *>(properties.at(0)->getValue().get());
    QVERIFY(value->getNodeType() == AST::Switch);
    QVERIFY(value->getSwitchValue() == true);
    QVERIFY(properties.at(1)->getName() == "prop2");
    value = static_cast<ValueNode *>(properties.at(1)->getValue().get());
    QVERIFY(value->getNodeType() == AST::Switch);
    QVERIFY(value->getSwitchValue() == false);
    QVERIFY(properties.at(2)->getName() == "prop3");
    value = static_cast<ValueNode *>(properties.at(2)->getValue().get());
    QVERIFY(value->getNodeType() == AST::None);

    QVERIFY(nodes.at(1)->getNodeType() == AST::Stream);
    StreamNode *stream = static_cast<StreamNode *>(nodes.at(1).get());
    FunctionNode *func = static_cast<FunctionNode *>(stream->getLeft().get());
    QVERIFY(func->getNodeType() == AST::Function);
    vector<std::shared_ptr<PropertyNode>> funcProperties = func->getProperties();
    QVERIFY(funcProperties.size() == 3);
    QVERIFY(funcProperties.at(0)->getName() == "propf1");
    value = static_cast<ValueNode *>(funcProperties.at(0)->getValue().get());
    QVERIFY(value->getNodeType() == AST::Switch);
    QVERIFY(value->getSwitchValue() == true);
    QVERIFY(funcProperties.at(1)->getName() == "propf2");
    value = static_cast<ValueNode *>(funcProperties.at(1)->getValue().get());
    QVERIFY(value->getNodeType() == AST::Switch);
    QVERIFY(value->getSwitchValue() == false);
    QVERIFY(funcProperties.at(2)->getName() == "propf3");
    value = static_cast<ValueNode *>(funcProperties.at(2)->getValue().get());
    QVERIFY(value->getNodeType() == AST::None);
}

void ParserTest::testBasicFunctions()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/05_basic_functions.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();

    QVERIFY(nodes.at(0)->getNodeType() == AST::Stream);
    StreamNode *node = static_cast<StreamNode *>(nodes.at(0).get());
    FunctionNode *function = static_cast<FunctionNode *>(node->getLeft().get());
    QVERIFY(function->getNodeType() == AST::Function);
    QVERIFY(function->getName() == "Function1");
    vector<std::shared_ptr<PropertyNode>> properties = function->getProperties();
    PropertyNode *property = properties.at(0).get();
    QVERIFY(property->getName() == "propReal");
    ValueNode *value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.1);
    property = properties.at(1).get();
    QVERIFY(property->getName() == "propInt");
    value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 23);
    property = properties.at(2).get();
    QVERIFY(property->getName() == "propString");
    value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::String);
    QVERIFY(value->getStringValue() == "hello");

    node = static_cast<StreamNode *>(node->getRight().get());
    QVERIFY(node->getNodeType() == AST::Stream);

    function = static_cast<FunctionNode *>(node->getLeft().get());
    QVERIFY(function->getNodeType() == AST::Function);
    QVERIFY(function->getName() == "Function2");
    properties = function->getProperties();
    property = properties.at(0).get();
    QVERIFY(property->getName() == "propReal");
    value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.2);
    property = properties.at(1).get();
    QVERIFY(property->getName() == "propInt");
    value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 123);
    property = properties.at(2).get();
    QVERIFY(property->getName() == "propString");
    value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::String);
    QVERIFY(value->getStringValue() == "function");

    function = static_cast<FunctionNode *>(node->getRight().get());
    QVERIFY(function->getNodeType() == AST::Function);
    QVERIFY(function->getName() == "Function3");
    properties = function->getProperties();
    property = properties.at(0).get();
    QVERIFY(property->getName() == "propReal");
    value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.3);
    property = properties.at(1).get();
    QVERIFY(property->getName() == "propInt");
    value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1123);
    property = properties.at(2).get();
    QVERIFY(property->getName() == "propString");
    value = static_cast<ValueNode *>(property->getValue().get());
    QVERIFY(value->getNodeType() == AST::String);
    QVERIFY(value->getStringValue() == "lines");
}

void ParserTest::testBasicStream()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/04_basic_stream.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();

    // Val1 >> Val2 ;
    StreamNode *node = static_cast<StreamNode *>(nodes.at(1).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 3);
    vector<ASTNode>streamParts = node->getChildren();
    QVERIFY(streamParts.size() == 2);
    AST *streamComp = streamParts.at(0).get();
    QVERIFY(streamComp->getNodeType() == AST::Block);
    QVERIFY(streamComp->getLine() == 3);
    BlockNode *nameNode = static_cast<BlockNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 3);
    streamComp = streamParts.at(1).get();
    QVERIFY(streamComp->getNodeType() == AST::Block);
    QVERIFY(streamComp->getLine() == 3);
    nameNode = static_cast<BlockNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val2");
    QVERIFY(nameNode->getLine() == 3);

    // Func1() >> Func2() ;
    node = static_cast<StreamNode *>(nodes.at(2).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 4);
    streamParts = node->getChildren();
    QVERIFY(streamParts.size() == 2);
    streamComp = streamParts.at(0).get();
    QVERIFY(streamComp->getLine() == 4);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    FunctionNode *functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func1");
    QVERIFY(functionNode->getLine() == 4);
    streamComp = streamParts.at(1).get();
    QVERIFY(streamComp->getNodeType() == AST::Function);
    QVERIFY(streamComp->getLine() == 4);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func2");
    QVERIFY(functionNode->getLine() == 4);

    // Val1 >> Func1() >> Func2() >> Val2 ;
    node = static_cast<StreamNode *>(nodes.at(3).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 6);
    streamComp = node->getLeft().get();
    QVERIFY(streamComp->getNodeType() == AST::Block);
    QVERIFY(streamComp->getLine() == 6);
    nameNode = static_cast<BlockNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 6);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(node->getRight().get());
    streamComp = node->getLeft().get();
    QVERIFY(streamComp->getLine() == 6);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func1");
    QVERIFY(functionNode->getLine() == 6);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(node->getRight().get());
    QVERIFY(node->getLine() == 7);
    streamComp = node->getLeft().get();
    QVERIFY(streamComp->getNodeType() == AST::Function);
    QVERIFY(streamComp->getLine() == 7);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func2");
    QVERIFY(functionNode->getLine() == 7);
    QVERIFY(node->getRight()->getNodeType() == AST::Block);
    nameNode = static_cast<BlockNode *>(node->getRight().get());
    QVERIFY(nameNode->getName() == "Val2");
    QVERIFY(nameNode->getLine() == 8);

    //    Bundle1[1] >> Bundle2[2];
    node = static_cast<StreamNode *>(nodes.at(4).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 10);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    BundleNode *bundle = static_cast<BundleNode *>(node->getLeft().get());
    QVERIFY(bundle->getName() == "Bundle1");
    QVERIFY(bundle->getLine() == 10);
    ListNode *indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    ValueNode *value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 10);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(node->getRight().get());
    QVERIFY(bundle->getName() == "Bundle2");
    QVERIFY(bundle->getLine() == 10);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 10);
    QVERIFY(value->getIntValue() == 2);

    //    Val1 * 3 >> Bundle[2];
    node = static_cast<StreamNode *>(nodes.at(5).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Expression);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    QVERIFY(node->getLine() == 11);
    ExpressionNode *expression = static_cast<ExpressionNode *>(node->getLeft().get());
    QVERIFY(expression->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expression->getLeft()->getNodeType() == AST::Block);
    QVERIFY(expression->getLine() == 11);
    nameNode = static_cast<BlockNode *>(expression->getLeft().get());
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 11);
    QVERIFY(expression->getRight()->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(expression->getRight().get())->getIntValue() == 3);
    bundle = static_cast<BundleNode *>(node->getRight().get());
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(bundle->getLine() == 11);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 11);
    QVERIFY(value->getIntValue() == 2);

    //    Bundle1[1] * 0.5 >> Bundle2[2];
    node = static_cast<StreamNode *>(nodes.at(6).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Expression);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    QVERIFY(node->getLine() == 12);
    expression = static_cast<ExpressionNode *>(node->getLeft().get());
    QVERIFY(expression->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expression->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(expression->getLine() == 12);
    bundle = static_cast<BundleNode *>(expression->getLeft().get());
    QVERIFY(bundle->getName() == "Bundle1");
    QVERIFY(bundle->getLine() == 12);
    QVERIFY(expression->getRight()->getNodeType() == AST::Real);
    QVERIFY(static_cast<ValueNode *>(expression->getRight().get())->getRealValue() == 0.5);
    bundle = static_cast<BundleNode *>(node->getRight().get());
    QVERIFY(bundle->getName() == "Bundle2");
    QVERIFY(bundle->getLine() == 12);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 12);
    QVERIFY(value->getIntValue() == 2);

    //    BundleRange[1:2] >> BundleRange2[3:4];
    node = static_cast<StreamNode *>(nodes.at(7).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    QVERIFY(node->getLine() == 14);
    bundle = static_cast<BundleNode *>(node->getLeft().get());
    QVERIFY(bundle->getName() == "BundleRange");
    QVERIFY(bundle->getLine() == 14);
    ListNode *index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    RangeNode *range = static_cast<RangeNode *>(indexList->getChildren().at(0).get());
    QVERIFY(range->getNodeType() == AST::Range);
    QVERIFY(range->getLine() == 14);
    value = static_cast<ValueNode *>(range->startIndex().get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    value = static_cast<ValueNode *>(range->endIndex().get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 14);
    QVERIFY(value->getIntValue() == 2);

    bundle = static_cast<BundleNode *>(node->getRight().get());
    QVERIFY(bundle->getName() == "BundleRange2");
    QVERIFY(bundle->getLine() == 14);
    index = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    indexList = bundle->index().get();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    range = static_cast<RangeNode *>(indexList->getChildren().at(0).get());
    QVERIFY(range->getNodeType() == AST::Range);
    QVERIFY(range->getLine() == 14);
    value = static_cast<ValueNode *>(range->startIndex().get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 3);
    value = static_cast<ValueNode *>(range->endIndex().get());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 14);
    QVERIFY(value->getIntValue() == 4);

    //    AudioIn[1] >> level(gain: 1.5) >> AudioOut[1];
    node = static_cast<StreamNode *>(nodes.at(8).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 16);
    node = static_cast<StreamNode *>(node->getRight().get());
    QVERIFY(node->getLeft()->getNodeType() == AST::Function);
    functionNode = static_cast<FunctionNode *>(node->getLeft().get());
    QVERIFY(functionNode->getName() == "Level");
    vector<std::shared_ptr<PropertyNode>> properties = functionNode->getProperties();
    QVERIFY(properties.size() == 1);
    PropertyNode *prop = properties[0].get();
    QVERIFY(prop->getName() == "gain");
    value = static_cast<ValueNode *>(prop->getValue().get());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.5);

    //    A[1:2,3,4] >> B[1,2,3:4] >> C[1,2:3,4] >> D[1,2,3,4];
    node = static_cast<StreamNode *>(nodes.at(9).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    bundle = static_cast<BundleNode *>(node->getLeft().get());
    QVERIFY(bundle->getName() == "A");
    QVERIFY(bundle->getLine() == 18);
    ListNode *list = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getLine() == 18);

    node = static_cast<StreamNode *>(nodes.at(10).get());
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::List);
    QVERIFY(node->getRight()->getNodeType() == AST::List);
    ListNode *l = static_cast<ListNode *>(node->getLeft().get());
    QVERIFY(l->size() == 2);
    vector<ASTNode> elements = l->getChildren();
    QVERIFY(elements.size() == 2);
    QVERIFY(elements.at(0)->getNodeType() == AST::Bundle);
    QVERIFY(elements.at(1)->getNodeType() == AST::Bundle);
    l = static_cast<ListNode *>(node->getRight().get());
    QVERIFY(l->size() == 2);
    elements = l->getChildren();
    QVERIFY(elements.size() == 2);
    QVERIFY(elements.at(0)->getNodeType() == AST::Bundle);
    QVERIFY(elements.at(1)->getNodeType() == AST::Bundle);

    QVERIFY(l->getLine() == 20);
}

void ParserTest::testBasicBundle()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/03_basic_bundle.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 8);
    DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(1).get());
    QVERIFY(block->getNodeType() == AST::BundleDeclaration);
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 5);
    std::shared_ptr<BundleNode> bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Integer");
    QVERIFY(bundle->getLine() == 5);
    QVERIFY(bundle->getChildren().size() == 1);
    ListNode *indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    ValueNode *valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(bundle->getLine() == 5);
    QVERIFY(block->getProperties().size() == 2);
    std::shared_ptr<PropertyNode> property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    AST *propertyValue = property->getValue().get();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    ListNode *listnode = static_cast<ListNode *>(propertyValue);
    //    QVERIFY(listnode->getListType() == AST::Int);
    vector<ASTNode>listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Int);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Expression);
    QVERIFY(listValues.at(2)->getNodeType() == AST::Int);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    // Next Block - Float list
    block = static_cast<DeclarationNode *>(nodes.at(2).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 10);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Float");
    QVERIFY(bundle->getLine() == 10);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 4);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue().get();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listnode = static_cast<ListNode *>(propertyValue);
    //    QVERIFY(listnode->getListType() == AST::Real);
    listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 4);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Real);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Expression);
    QVERIFY(listValues.at(2)->getNodeType() == AST::Real);
    QVERIFY(listValues.at(3)->getNodeType() == AST::Real);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_pointer_cast<PropertyNode>(block->getProperties().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - String list
    block = static_cast<DeclarationNode *>(nodes.at(3).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 15);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "String");
    QVERIFY(bundle->getLine() == 15);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 5);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue().get();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getListType() == AST::String);
    listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 5);
    QVERIFY(listValues.at(0)->getNodeType() == AST::String);
    QVERIFY(listValues.at(1)->getNodeType() == AST::String);
    QVERIFY(listValues.at(2)->getNodeType() == AST::String);
    QVERIFY(listValues.at(3)->getNodeType() == AST::String);
    QVERIFY(listValues.at(4)->getNodeType() == AST::String);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    // Next Block - UVar list
    block = static_cast<DeclarationNode *>(nodes.at(4).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 21);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "UVar");
    QVERIFY(bundle->getLine() == 21);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 7);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue().get();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 7);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Block);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Block);
    QVERIFY(listValues.at(2)->getNodeType() == AST::Block);
    QVERIFY(listValues.at(3)->getNodeType() == AST::Block);
    QVERIFY(listValues.at(4)->getNodeType() == AST::Block);
    QVERIFY(listValues.at(5)->getNodeType() == AST::Block);
    QVERIFY(listValues.at(6)->getNodeType() == AST::Block);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    // Next Block - ArrayList list
    block = static_cast<DeclarationNode *>(nodes.at(5).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 26);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "ArrayList");
    QVERIFY(bundle->getLine() == 26);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 2);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue().get();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 2);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Bundle);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Bundle);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    // Next Block - BlockList list
    block = static_cast<DeclarationNode *>(nodes.at(6).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 31);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "BlockList");
    QVERIFY(bundle->getLine() == 31);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue().get();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Declaration);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Declaration);
    property = static_pointer_cast<PropertyNode>(block->getProperties().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - BlockBundleList list
    block = static_cast<DeclarationNode *>(nodes.at(7).get());
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 36);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "BlockBundleList");
    QVERIFY(bundle->getLine() == 36);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index().get());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue().get();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::BundleDeclaration);
    QVERIFY(listValues.at(1)->getNodeType() == AST::BundleDeclaration);
    QVERIFY(listValues.at(2)->getNodeType() == AST::BundleDeclaration);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    //    // Next Block - IntegerList list
    //    block = static_cast<DeclarationNode *>(nodes.at(7));
    //    QVERIFY(block->getObjectType() == "constant");
    //    QVERIFY(block->getLine() == 46);
    //    bundle = block->getBundle();
    //    QVERIFY(bundle->getName() == "IntegerList");
    //    QVERIFY(bundle->getLine() == 46);
    //    QVERIFY(bundle->getChildren().size() == 1);
    //    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    //    QVERIFY(valueNode->getNodeType() == AST::Int);
    //    QVERIFY(valueNode->getIntValue() == 3);
    //    QVERIFY(block->getProperties().size() == 2);
    //    property = block->getProperties().at(0);
    //    QVERIFY(property->getName() == "value");
    //    propertyValue = property->getValue();
    //    QVERIFY(propertyValue->getNodeType() == AST::List);
    //    listValues = propertyValue->getChildren();
    //    QVERIFY(listValues.size() == 3);
    //    QVERIFY(listValues.at(0)->getNodeType() == AST::List);
    //    QVERIFY(listValues.at(1)->getNodeType() == AST::List);
    //    QVERIFY(listValues.at(2)->getNodeType() == AST::List);
    //    property = block->getProperties().at(1);
    //    QVERIFY(property->getName() == "meta");

    //    // Next Block - IntegerList list
    //    block = static_cast<DeclarationNode *>(nodes.at(8));
    //    QVERIFY(block->getObjectType() == "constant");
    //    QVERIFY(block->getLine() == 52);
    //    bundle = block->getBundle();
    //    QVERIFY(bundle->getName() == "IntegerList");
    //    QVERIFY(bundle->getLine() == 53);
    //    QVERIFY(bundle->getChildren().size() == 1);
    //    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    //    QVERIFY(valueNode->getNodeType() == AST::Int);
    //    QVERIFY(valueNode->getIntValue() == 3);
    //    QVERIFY(block->getProperties().size() == 2);
    //    property = block->getProperties().at(0);
    //    QVERIFY(property->getName() == "value");
    //    propertyValue = property->getValue();
    //    QVERIFY(propertyValue->getNodeType() == AST::List);
    //    listValues = propertyValue->getChildren();
    //    QVERIFY(listValues.size() == 3);
    //    QVERIFY(listValues.at(0)->getNodeType() == AST::List);
    //    QVERIFY(listValues.at(1)->getNodeType() == AST::List);
    //    QVERIFY(listValues.at(2)->getNodeType() == AST::List);
    //    property = block->getProperties().at(1);
    //    QVERIFY(property->getName() == "meta");
}

void ParserTest::testBasicBlocks()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/02_basic_blocks.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 5);
    AST *node = nodes.at(0).get();
    QVERIFY(node->getNodeType() == AST::Declaration);
    vector<std::shared_ptr<PropertyNode>> properties = static_cast<DeclarationNode *>(node)->getProperties();
    QVERIFY(properties.size() == 2);
    std::shared_ptr<PropertyNode> property = properties.at(0);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "rate");
    AST *propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::Block);
    QVERIFY(static_cast<BlockNode *>(propertyValue)->getName() == "AudioRate");
    property = properties.at(1);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "meta");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Guitar input.");

    node = nodes.at(1).get();
    QVERIFY(node->getNodeType() == AST::Declaration);
    properties = static_cast<DeclarationNode *>(node)->getProperties();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getIntValue() == 5);
    property = properties.at(1);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "meta");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Integer Value.");

    // No properties
    node = nodes.at(2).get();
    QVERIFY(node->getNodeType() == AST::Declaration);
    properties = static_cast<DeclarationNode *>(node)->getProperties();
    QVERIFY(properties.size() == 0);

    // Property is an object
    node = nodes.at(3).get();
    QVERIFY(node->getNodeType() == AST::Declaration);
    properties = static_cast<DeclarationNode *>(node)->getProperties();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::Declaration);
    DeclarationNode *object = static_cast<DeclarationNode *>(propertyValue);
    QVERIFY(object->getName() == "");
    QVERIFY(object->getObjectType() == "");
    vector<std::shared_ptr<PropertyNode> > objProperties = static_cast<DeclarationNode *>(object)->getProperties();
    QVERIFY(objProperties.size() == 2);
    property = objProperties.at(0);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "prop1");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getIntValue() == 5);
    property = objProperties.at(1);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "prop2");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "hello");
    property = properties.at(1);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "meta");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Block as Property");

    node = nodes.at(4).get();
    QVERIFY(node->getNodeType() == AST::Declaration);
    properties = static_cast<DeclarationNode *>(node)->getProperties();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "process");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    ListNode *listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getNodeType() == AST::List);
    QVERIFY(listnode->getChildren().size() == 1);
    StreamNode *streamNode = static_cast<StreamNode *>(listnode->getChildren().at(0).get());
    QVERIFY(streamNode->getChildren().size() == 2);
    QVERIFY(streamNode->getChildren().at(0)->getNodeType() == AST::Function);
    QVERIFY(streamNode->getChildren().at(1)->getNodeType() == AST::Bundle);
    property = properties.at(1);
    QVERIFY(property != nullptr && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "meta");
    propertyValue = property->getChildren().at(0).get();
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Stream property");
}

void ParserTest::testHeader()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/01_header.stride")).toStdString().c_str());

    QVERIFY(tree != nullptr);
    vector<ASTNode> nodes = tree->getChildren();
    SystemNode *node = static_cast<SystemNode *>(nodes.at(0).get());
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "PufferFish");
    QVERIFY(node->majorVersion() == 1);
    QVERIFY(node->minorVersion() == 1);
    QVERIFY(node->getChildren().size() == 0);
    QVERIFY(node->getLine() == 1);

    node = static_cast<SystemNode *>(nodes.at(1).get());
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->majorVersion() == -1);
    QVERIFY(node->minorVersion() == -1);
    QVERIFY(node->getChildren().size() == 0);
    QVERIFY(node->getLine() == 3);

    ImportNode *importnode = static_cast<ImportNode *>(nodes.at(2).get());
    QVERIFY(importnode->getNodeType() == AST::Import);
    QVERIFY(importnode->importName() == "File");
    QVERIFY(importnode->importAlias() == "");
    QVERIFY(importnode->getLine() == 5);

    importnode = static_cast<ImportNode *>(nodes.at(3).get());
    QVERIFY(importnode->getNodeType() == AST::Import);
    QVERIFY(importnode->importName() == "File");
    QVERIFY(importnode->importAlias() == "F");
    QVERIFY(importnode->getLine() == 6);
}

void ParserTest::testLibraryBasicTypes()
{
    StrideLibrary library(QFINDTESTDATA(STRIDEROOT));
    std::shared_ptr<DeclarationNode> type;
    QStringList typesToCheck;
    typesToCheck << "rated" << "domainMember" << "type" << "base" << "port"
                 << "module" << "reaction";
    for(const QString &typeName : typesToCheck) {
        type = library.findTypeInLibrary(typeName);
        QVERIFY(type);
        QVERIFY(library.isValidBlock(type.get()));
    }
}

void ParserTest::testLibraryValidation()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/L01_library_types_validation.stride")).toStdString().c_str());
    QVERIFY(tree);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree);
    QVERIFY(generator.isValid());
}

void ParserTest::testTriggersRegistration()
{
    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P12_trigger_registration.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

    auto triggerDecl = CodeValidator::findDeclaration(std::string("Trig"), {}, tree);
    QVERIFY(triggerDecl);
    auto triggerSources = triggerDecl->getCompilerProperty("triggerSources");
    QVERIFY(triggerSources);
    QVERIFY(triggerSources->getNodeType() == AST::List);
    QVERIFY(triggerSources->getChildren().size() == 3);
    auto block = static_pointer_cast<BlockNode>(triggerSources->getChildren()[0]);
    QVERIFY(block->getName() == "Switch1");
    block = static_pointer_cast<BlockNode>(triggerSources->getChildren()[1]);
    QVERIFY(block->getName() == "Switch2");
    block = static_pointer_cast<BlockNode>(triggerSources->getChildren()[2]);
    QVERIFY(block->getName() == "Switch3");

}
void ParserTest::testResetRegistration()
{
//    trigger Trig { }

//    switch Switch1 { reset: Trig }
//    switch SwitchBundle[2] { reset: Trig }
//    signal Sig { reset: Trig }
//    signal SigBundle[2] { reset: Trig }

    ASTNode tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/P13_reset_registration.stride")).toStdString().c_str());
    QVERIFY(tree != nullptr);
    CodeValidator generator(QFINDTESTDATA(STRIDEROOT), tree, CodeValidator::NO_RATE_VALIDATION);
    QVERIFY(generator.isValid());

    auto triggerDecl = CodeValidator::findDeclaration(std::string("Trig"), {}, tree);
    QVERIFY(triggerDecl);
    auto triggerResets = triggerDecl->getCompilerProperty("triggerResets");
    QVERIFY(triggerResets);
    QVERIFY(triggerResets->getNodeType() == AST::List);
    QVERIFY(triggerResets->getChildren().size() == 4);
    auto block = static_pointer_cast<DeclarationNode>(triggerResets->getChildren()[0]);
    QVERIFY(block->getName() == "Switch1");
    block = static_pointer_cast<DeclarationNode>(triggerResets->getChildren()[1]);
    QVERIFY(block->getName() == "SwitchBundle");
    block = static_pointer_cast<DeclarationNode>(triggerResets->getChildren()[2]);
    QVERIFY(block->getName() == "Sig");
    block = static_pointer_cast<DeclarationNode>(triggerResets->getChildren()[3]);
    QVERIFY(block->getName() == "SigBundle");

}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
