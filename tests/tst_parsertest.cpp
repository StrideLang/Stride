#include <QString>
#include <QtTest>

#include "streamparser.h"
#include "streamplatform.h"
#include "codevalidator.h"
#include "coderesolver.h"

extern AST *parse(const char* fileName);

class ParserTest : public QObject
{
    Q_OBJECT

public:
    ParserTest();

private:

private Q_SLOTS:


    //Expansion
    void testMultichannelUgens();
    void testConstantResolution();
    void testStreamRates();
    void testStreamExpansion();

    //Platform
    void testPlatform();
    void testPlatformCommonObjects();
    void testValueTypeBundleResolution();
    void testValueTypeExpressionResolution();
    void testDuplicates();
    void testListConsistency();

    // Parser
    void testTreeBuildNoneSwitch();
    void testTreeBuildArray();
    void testTreeBuildFunctions();
    void testTreeBuildStream();
    void testTreeBuildBundles();
    void testTreeBuildBlocks();
    void testTreeBuildBasic();
    void testParser();
};

ParserTest::ParserTest()
{
}

void ParserTest::testMultichannelUgens()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/multichn.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    CodeValidator generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(!generator.isValid());

    QList<LangError> errors = generator.getErrors();
    LangError error = errors.takeFirst();
    QVERIFY(error.type == LangError::StreamMemberSizeMismatch);
    QVERIFY(error.lineNumber == 20);
    QVERIFY(error.errorTokens[0] == "2");
    QVERIFY(error.errorTokens[1] == "pan");
    QVERIFY(error.errorTokens[2] == "1");

    error = errors.takeFirst();
    QVERIFY(error.type == LangError::StreamMemberSizeMismatch);
    QVERIFY(error.lineNumber == 23);
    QVERIFY(error.errorTokens[0] == "2");
    QVERIFY(error.errorTokens[1] == "DummyStereo");
    QVERIFY(error.errorTokens[2] == "1");

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testConstantResolution()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/constantRes.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    CodeValidator generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(generator.isValid());

    BlockNode *block = static_cast<BlockNode *>(tree->getChildren().at(4));
    QVERIFY(block->getNodeType() == AST::Block);
    ValueNode *value = static_cast<ValueNode *>(block->getPropertyValue("value"));
    QVERIFY(value != NULL);
    QVERIFY(value->getNodeType() == AST::Real);

    QVERIFY(qFuzzyCompare(value->getRealValue(), 2.0 + (3.1 * 0.1)));

    block = static_cast<BlockNode *>(tree->getChildren().at(5));
    QVERIFY(block->getNodeType() == AST::Block);
    value = static_cast<ValueNode *>(block->getPropertyValue("value"));
    QVERIFY(value != NULL);
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(qFuzzyCompare(value->getRealValue(), -0.1));

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testStreamRates()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/rates.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    CodeValidator generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(generator.isValid());
    QVERIFY(generator.platformIsValid());

    vector<AST *> nodes = tree->getChildren();

    // AudioIn[1] >> Signal >> AudioOut[1];
    StreamNode *stream = static_cast<StreamNode *>(nodes.at(1));
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getRate() == 44100);

    stream = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getRate() == 44100);

    NameNode *name = static_cast<NameNode *>(stream->getLeft());
    QVERIFY(name->getNodeType() == AST::Name);
    QVERIFY(name->getRate() == 44100);

    BundleNode *bundle = static_cast<BundleNode *>(stream->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getRate() == 44100);

    // Rate1 >> Rate2 >> AudioOut[1];
    stream = static_cast<StreamNode *>(nodes.at(4));
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 22050);

    stream = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 11025);

    NameNode *nameNode = static_cast<NameNode *>(stream->getRight());
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getRate() == 44100);

    // GetRate1 >> Rate1 >> GetRate2 >> Rate2 >> GetAudioRate >> AudioOut[1];
    stream = static_cast<StreamNode *>(nodes.at(5));
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 44100);

    stream = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 22050);

    stream = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 44100);

    stream = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 11025);

    stream = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 44100);

    name = static_cast<NameNode *>(stream->getRight());
    QVERIFY(name->getNodeType() == AST::Name);
    QVERIFY(name->getRate() == 44100);

//    oscillator() >> Rate1 >> lowPass() >> Output3;
    stream = static_cast<StreamNode *>(nodes.at(6));
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 22050);
    stream = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 22050);
    stream = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(stream->getNodeType() == AST::Stream);
    QVERIFY(stream->getLeft()->getRate() == 44100);
    QVERIFY(stream->getRight()->getRate() == 44100);

//    InSignal >> pan() >> OutSignal;

//    InSignal2 >> [pan(), pan()] >> OutSignal2;
    tree->deleteChildren();
    delete tree;
}

void ParserTest::testStreamExpansion()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/expansions.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    CodeValidator generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(generator.isValid());
    QVERIFY(generator.platformIsValid());

    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() > 3);
    StreamNode *stream = static_cast<StreamNode *>(nodes.at(1));
    QVERIFY(stream->getNodeType() == AST::Stream);

    BundleNode *bundle = static_cast<BundleNode *>(stream->getLeft());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioIn");
    ListNode *index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    ValueNode *value = static_cast<ValueNode *>(index->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    StreamNode *right = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(right->getNodeType() == AST::Stream);
    FunctionNode *func = static_cast<FunctionNode *>(right->getLeft());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "level");

    bundle = static_cast<BundleNode *>(right->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioOut");
    index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    stream = static_cast<StreamNode *>(nodes.at(2));
    QVERIFY(stream->getNodeType() == AST::Stream);

    bundle = static_cast<BundleNode *>(stream->getLeft());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioIn");
    index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    right = static_cast<StreamNode *>(stream->getRight());
    QVERIFY(right->getNodeType() == AST::Stream);
    func = static_cast<FunctionNode *>(right->getLeft());
    QVERIFY(func->getNodeType() == AST::Function);
    QVERIFY(func->getName() == "level");

    bundle = static_cast<BundleNode *>(right->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "AudioOut");
    index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testPlatform()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/platform.stream")).toStdString().c_str());

    QVERIFY(tree != NULL);
    CodeValidator generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(generator.isValid());
    QVERIFY(generator.platformIsValid());

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testPlatformCommonObjects()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/platformBasic.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    CodeValidator generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();

    QVERIFY(errors.size() == 5);

    QVERIFY(errors[0].type == LangError::UnknownType);
    QVERIFY(errors[0].lineNumber == 3);
    QVERIFY(errors[0].errorTokens[0] == "invalid");

    QVERIFY(errors[1].type == LangError::InvalidPortType);
    QVERIFY(errors[1].lineNumber == 11);
    QVERIFY(errors[1].errorTokens[0]  == "object");
    QVERIFY(errors[1].errorTokens[1]  == "meta");
    QVERIFY(errors[1].errorTokens[2]  == "CIP");

    QVERIFY(errors[2].type == LangError::InvalidPortType);
    QVERIFY(errors[2].lineNumber == 20);
    QVERIFY(errors[2].errorTokens[0]  == "switch");
    QVERIFY(errors[2].errorTokens[1]  == "default");
    QVERIFY(errors[2].errorTokens[2]  == "CIP");

    QVERIFY(errors[3].type == LangError::InvalidPort);
    QVERIFY(errors[3].lineNumber == 31);
    QVERIFY(errors[3].errorTokens[0]  == "signal");
    QVERIFY(errors[3].errorTokens[1]  == "badproperty");

    QVERIFY(errors[4].type == LangError::InvalidPortType);
    QVERIFY(errors[4].lineNumber == 56);
    QVERIFY(errors[4].errorTokens[0]  == "control");
    QVERIFY(errors[4].errorTokens[1]  == "maximum");
    QVERIFY(errors[4].errorTokens[2]  == "CSP");

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testValueTypeBundleResolution()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/bundleResolution.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    CodeValidator generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(generator.platformIsValid());
    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();

    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.at(1)->getNodeType() == AST::BlockBundle);
    BlockNode *block = static_cast<BlockNode *>(nodes.at(1));
    QVERIFY(block->getObjectType() == "constant");
    BundleNode *bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Integers");
    ListNode *index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    ValueNode *value = static_cast<ValueNode *>(index->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    QVERIFY(block->getProperties().size() == 2);
    PropertyNode *property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    AST *propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    ListNode *listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getListType() == AST::Int);
    vector<AST *>listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 4);

    QVERIFY(nodes.at(2)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(2));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Floats");
    index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getListType() == AST::Real);
    listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 4);

    QVERIFY(nodes.at(3)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(3));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Strings");
    index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    value = static_cast<ValueNode *>(index->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getListType() == AST::String);
    listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 4);

    QVERIFY(nodes.at(4)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(4));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Int");

    QVERIFY(nodes.at(5)->getNodeType() == AST::Block);
    block = static_cast<BlockNode *>(nodes.at(5));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getName() == "Value_Meta");

    QVERIFY(nodes.at(6)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(6));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Value_Meta_1");

    QVERIFY(nodes.at(7)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(7));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_1");

    QVERIFY(nodes.at(8)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(8));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_2");

    // The following cases should produce errors

    QVERIFY(nodes.at(9)->getNodeType() == AST::Block);
    block = static_cast<BlockNode *>(nodes.at(9));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getName() == "Bad_Value_Meta");

    LangError error = errors.takeFirst();
    QVERIFY(error.type == LangError::InvalidPortType);
    QVERIFY(error.lineNumber == 39);
    QVERIFY(error.errorTokens[0] == "constant");
    QVERIFY(error.errorTokens[1] == "meta");
    QVERIFY(error.errorTokens[2] == "CRP");

    QVERIFY(nodes.at(10)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(10));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_Mismatch");
    error = errors.takeFirst();
    QVERIFY(error.type == LangError::BundleSizeMismatch);
    QVERIFY(error.lineNumber == 44);
    QVERIFY(error.errorTokens[0] == "Values_Mismatch");
    QVERIFY(error.errorTokens[1] == "3");
    QVERIFY(error.errorTokens[2] == "4");


    QVERIFY(nodes.at(11)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(11));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_Mismatch_2");
    error = errors.takeFirst();
    QVERIFY(error.lineNumber == 48);
    QVERIFY(error.errorTokens[0] == "Values_Mismatch_2");
    QVERIFY(error.errorTokens[1] == "3");
    QVERIFY(error.errorTokens[2] == "2");

    QVERIFY(nodes.at(12)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(12));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Float");
    error = errors.takeFirst();
    QVERIFY(error.type == LangError::IndexMustBeInteger);
    QVERIFY(error.lineNumber == 52);
    QVERIFY(error.errorTokens[0] == "Array_Float");
    QVERIFY(error.errorTokens[1] == "CRP");

    QVERIFY(nodes.at(13)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(13));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_String");
    error = errors.takeFirst();
    QVERIFY(error.type == LangError::IndexMustBeInteger);
    QVERIFY(error.lineNumber == 53);
    QVERIFY(error.errorTokens[0] == "Array_String");
    QVERIFY(error.errorTokens[1] == "CSP");

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testValueTypeExpressionResolution()
{

}

void ParserTest::testDuplicates()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/errorDuplicate.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    CodeValidator generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();

    QVERIFY(errors.size() == 2);
    LangError error = errors.takeFirst();
    QVERIFY(error.type == LangError::DuplicateSymbol);
    QVERIFY(error.lineNumber == 12);
    QVERIFY(error.errorTokens[0] == "Const");
    QVERIFY(error.errorTokens[1] == "3");

    error = errors.takeFirst();
    QVERIFY(error.type == LangError::DuplicateSymbol);
    QVERIFY(error.lineNumber == 18);
    QVERIFY(error.errorTokens[0] == "Size");
    QVERIFY(error.errorTokens[1] == "7");

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testListConsistency()
{
//    AST *tree;
    // FIXME: List consistency is checked by the parser, should be caught here.
//    tree = parse(QString(QFINDTESTDATA("data/errorLists.stream")).toStdString().c_str());
//    QVERIFY(tree == NULL);
//    Codegen generator(QFINDTESTDATA("/../platforms"), tree);
//    QVERIFY(!generator.isValid());
//    QList<LangError> errors = generator.getErrors();

//    tree->deleteChildren();
//    delete tree;
}

void ParserTest::testTreeBuildNoneSwitch()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/noneswitch.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 2);

    QVERIFY(nodes.at(0)->getNodeType() == AST::Block);
    BlockNode *block = static_cast<BlockNode *>(nodes.at(0));
    QVERIFY(block->getObjectType() == "object");
    vector<PropertyNode *> properties = block->getProperties();
    QVERIFY(properties.size() == 3);
    QVERIFY(properties.at(0)->getName() == "prop1");
    ValueNode *value = static_cast<ValueNode *>(properties.at(0)->getValue());
    QVERIFY(value->getNodeType() == AST::Switch);
    QVERIFY(value->getSwitchValue() == true);
    QVERIFY(properties.at(1)->getName() == "prop2");
    value = static_cast<ValueNode *>(properties.at(1)->getValue());
    QVERIFY(value->getNodeType() == AST::Switch);
    QVERIFY(value->getSwitchValue() == false);
    QVERIFY(properties.at(2)->getName() == "prop3");
    value = static_cast<ValueNode *>(properties.at(2)->getValue());
    QVERIFY(value->getNodeType() == AST::None);

    QVERIFY(nodes.at(1)->getNodeType() == AST::Stream);
    StreamNode *stream = static_cast<StreamNode *>(nodes.at(1));
    FunctionNode *func = static_cast<FunctionNode *>(stream->getLeft());
    QVERIFY(func->getNodeType() == AST::Function);
    properties = func->getProperties();
    QVERIFY(properties.size() == 3);
    QVERIFY(properties.at(0)->getName() == "propf1");
    value = static_cast<ValueNode *>(properties.at(0)->getValue());
    QVERIFY(value->getNodeType() == AST::Switch);
    QVERIFY(value->getSwitchValue() == true);
    QVERIFY(properties.at(1)->getName() == "propf2");
    value = static_cast<ValueNode *>(properties.at(1)->getValue());
    QVERIFY(value->getNodeType() == AST::Switch);
    QVERIFY(value->getSwitchValue() == false);
    QVERIFY(properties.at(2)->getName() == "propf3");
    value = static_cast<ValueNode *>(properties.at(2)->getValue());
    QVERIFY(value->getNodeType() == AST::None);

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildArray()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/array.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 23);

    QVERIFY(nodes.at(0)->getNodeType() == AST::Block);
    BlockNode *block = static_cast<BlockNode *>(nodes.at(0));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 8);

    QVERIFY(nodes.at(1)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(1));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 13);
    BundleNode *bundle = block->getBundle();
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    QVERIFY(bundle->getLine() == 13);
    ListNode *indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    ValueNode *value = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    QVERIFY(value->getLine() == 13);
    QVERIFY(block->getProperties().size() == 2);
    QVERIFY(block->getProperties().at(0)->getNodeType() == AST::Property);
    PropertyNode *property = static_cast<PropertyNode *>(block->getProperties().at(0));
    ListNode * listnode = static_cast<ListNode *>(property->getValue());
    QVERIFY(listnode->getNodeType() == AST::List);
    QVERIFY(listnode->getChildren().size() == 4);
    QVERIFY(listnode->getLine() == 14);
    value = static_cast<ValueNode *>(listnode->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 16);
    QVERIFY(value->getLine() == 14);
    value = static_cast<ValueNode *>(listnode->getChildren().at(1));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 32);
    QVERIFY(value->getLine() == 14);
    value = static_cast<ValueNode *>(listnode->getChildren().at(2));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 64);
    QVERIFY(value->getLine() == 14);
    value = static_cast<ValueNode *>(listnode->getChildren().at(3));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 128);
    QVERIFY(value->getLine() == 14);

    // constant Array_Parens [ ( CONST * 2 ) + 1 ] {}
    block = static_cast<BlockNode *>(nodes.at(6));
    QVERIFY(block->getNodeType() == AST::BlockBundle);
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 23);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Parens");
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    ExpressionNode *expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Add);
    QVERIFY(expr->getLine() == 23);
    ExpressionNode *expr2 = static_cast<ExpressionNode *>(expr->getLeft());
    QVERIFY(expr2->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expr2->getLeft()->getNodeType() == AST::Name);
    QVERIFY(expr2->getRight()->getNodeType() == AST::Int);
    QVERIFY(expr2->getLine() == 23);

    // constant Array_Expr [ SIZE [1] + SIZE [1 * 2] ] {}
    block = static_cast<BlockNode *>(nodes.at(8));
    QVERIFY(block->getNodeType() == AST::BlockBundle);
    QVERIFY(block->getLine() == 26);
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Expr");
    QVERIFY(bundle->getLine() == 26);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Add);
    QVERIFY(expr->getLine() == 26);
    bundle = static_cast<BundleNode *>(expr->getLeft());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(expr->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expr->getLeft()->getNodeType() == AST::Int);
    QVERIFY(expr->getRight()->getNodeType() == AST::Int);
    QVERIFY(expr->getLine() == 26);

    // constant Array_Expr2 [ SIZE [1] / SIZE [1 - 2] ] {}
    block = static_cast<BlockNode *>(nodes.at(9));
    QVERIFY(block->getNodeType() == AST::BlockBundle);
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 27);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Expr2");
    QVERIFY(bundle->getChildren().size() == 1);
    QVERIFY(bundle->getLine() == 27);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Divide);
    QVERIFY(expr->getLine() == 27);
    bundle = static_cast<BundleNode *>(expr->getLeft());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    QVERIFY(bundle->getLine() == 27);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    QVERIFY(value->getLine() == 27);
    bundle = static_cast<BundleNode *>(expr->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getLine() == 27);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Subtract);
    QVERIFY(expr->getLeft()->getNodeType() == AST::Int);
    QVERIFY(expr->getRight()->getNodeType() == AST::Int);
    QVERIFY(expr->getLine() == 27);

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildFunctions()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/functions.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();

    QVERIFY(nodes.at(0)->getNodeType() == AST::Stream);
    StreamNode *node = static_cast<StreamNode *>(nodes.at(0));
    FunctionNode *function = static_cast<FunctionNode *>(node->getLeft());
    QVERIFY(function->getNodeType() == AST::Function);
    QVERIFY(function->getName() == "Function1");
    vector<PropertyNode *> properties = function->getProperties();
    PropertyNode *property = properties.at(0);
    QVERIFY(property->getName() == "propReal");
    ValueNode *value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.1);
    property = properties.at(1);
    QVERIFY(property->getName() == "propInt");
    value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 23);
    property = properties.at(2);
    QVERIFY(property->getName() == "propString");
    value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::String);
    QVERIFY(value->getStringValue() == "hello");

    node = static_cast<StreamNode *>(node->getRight());
    QVERIFY(node->getNodeType() == AST::Stream);

    function = static_cast<FunctionNode *>(node->getLeft());
    QVERIFY(function->getNodeType() == AST::Function);
    QVERIFY(function->getName() == "Function2");
    properties = function->getProperties();
    property = properties.at(0);
    QVERIFY(property->getName() == "propReal");
    value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.2);
    property = properties.at(1);
    QVERIFY(property->getName() == "propInt");
    value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 123);
    property = properties.at(2);
    QVERIFY(property->getName() == "propString");
    value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::String);
    QVERIFY(value->getStringValue() == "function");

    function = static_cast<FunctionNode *>(node->getRight());
    QVERIFY(function->getNodeType() == AST::Function);
    QVERIFY(function->getName() == "Function3");
    properties = function->getProperties();
    property = properties.at(0);
    QVERIFY(property->getName() == "propReal");
    value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.3);
    property = properties.at(1);
    QVERIFY(property->getName() == "propInt");
    value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1123);
    property = properties.at(2);
    QVERIFY(property->getName() == "propString");
    value = static_cast<ValueNode *>(property->getValue());
    QVERIFY(value->getNodeType() == AST::String);
    QVERIFY(value->getStringValue() == "lines");

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildStream()
{
    AST *tree;
    tree = AST::parseFile(QString(QFINDTESTDATA("data/stream.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 10);

    // Val1 >> Val2 ;
    StreamNode *node = static_cast<StreamNode *>(nodes.at(1));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 3);
    vector<AST *>streamParts = node->getChildren();
    QVERIFY(streamParts.size() == 2);
    AST *streamComp = streamParts.at(0);
    QVERIFY(streamComp->getNodeType() == AST::Name);
    QVERIFY(streamComp->getLine() == 3);
    NameNode *nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 3);
    streamComp = streamParts.at(1);
    QVERIFY(streamComp->getNodeType() == AST::Name);
    QVERIFY(streamComp->getLine() == 3);
    nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val2");
    QVERIFY(nameNode->getLine() == 3);

    // Func1() >> Func2() ;
    node = static_cast<StreamNode *>(nodes.at(2));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 4);
    streamParts = node->getChildren();
    QVERIFY(streamParts.size() == 2);
    streamComp = streamParts.at(0);
    QVERIFY(streamComp->getLine() == 4);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    FunctionNode *functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func1");
    QVERIFY(functionNode->getLine() == 4);
    streamComp = streamParts.at(1);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    QVERIFY(streamComp->getLine() == 4);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func2");
    QVERIFY(functionNode->getLine() == 4);

    // Val1 >> Func1() >> Func2() >> Val2 ;
    node = static_cast<StreamNode *>(nodes.at(3));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 6);
    streamComp = node->getLeft();
    QVERIFY(streamComp->getNodeType() == AST::Name);
    QVERIFY(streamComp->getLine() == 6);
    nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 6);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(node->getRight());
    streamComp = node->getLeft();
    QVERIFY(streamComp->getLine() == 6);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func1");
    QVERIFY(functionNode->getLine() == 6);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(node->getRight());
    QVERIFY(node->getLine() == 7);
    streamComp = node->getLeft();
    QVERIFY(streamComp->getNodeType() == AST::Function);
    QVERIFY(streamComp->getLine() == 7);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func2");
    QVERIFY(functionNode->getLine() == 7);
    QVERIFY(node->getRight()->getNodeType() == AST::Name);
    nameNode = static_cast<NameNode *>(node->getRight());
    QVERIFY(nameNode->getName() == "Val2");
    QVERIFY(nameNode->getLine() == 8);

//    Bundle1[1] >> Bundle2[2];
    node = static_cast<StreamNode *>(nodes.at(4));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 10);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    BundleNode *bundle = static_cast<BundleNode *>(node->getLeft());
    QVERIFY(bundle->getName() == "Bundle1");
    QVERIFY(bundle->getLine() == 10);
    ListNode *indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    ValueNode *value = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 10);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle2");
    QVERIFY(bundle->getLine() == 10);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 10);
    QVERIFY(value->getIntValue() == 2);

//    Val1 * 3 >> Bundle[2];
    node = static_cast<StreamNode *>(nodes.at(5));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Expression);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    QVERIFY(node->getLine() == 11);
    ExpressionNode *expression = static_cast<ExpressionNode *>(node->getLeft());
    QVERIFY(expression->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expression->getLeft()->getNodeType() == AST::Name);
    QVERIFY(expression->getLine() == 11);
    nameNode = static_cast<NameNode *>(expression->getLeft());
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 11);
    QVERIFY(expression->getRight()->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(expression->getRight())->getIntValue() == 3);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(bundle->getLine() == 11);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 11);
    QVERIFY(value->getIntValue() == 2);

//    Bundle1[1] * 0.5 >> Bundle2[2];
    node = static_cast<StreamNode *>(nodes.at(6));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Expression);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    QVERIFY(node->getLine() == 12);
    expression = static_cast<ExpressionNode *>(node->getLeft());
    QVERIFY(expression->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expression->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(expression->getLine() == 12);
    bundle = static_cast<BundleNode *>(expression->getLeft());
    QVERIFY(bundle->getName() == "Bundle1");
    QVERIFY(bundle->getLine() == 12);
    QVERIFY(expression->getRight()->getNodeType() == AST::Real);
    QVERIFY(static_cast<ValueNode *>(expression->getRight())->getRealValue() == 0.5);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle2");
    QVERIFY(bundle->getLine() == 12);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    value = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 12);
    QVERIFY(value->getIntValue() == 2);

    //    BundleRange[1:2] >> BundleRange2[3:4];
    node = static_cast<StreamNode *>(nodes.at(7));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    QVERIFY(node->getLine() == 14);
    bundle = static_cast<BundleNode *>(node->getLeft());
    QVERIFY(bundle->getName() == "BundleRange");
    QVERIFY(bundle->getLine() == 14);
    ListNode *index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    RangeNode *range = static_cast<RangeNode *>(indexList->getChildren().at(0));
    QVERIFY(range->getNodeType() == AST::Range);
    QVERIFY(range->getLine() == 14);
    value = static_cast<ValueNode *>(range->startIndex());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    value = static_cast<ValueNode *>(range->endIndex());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 14);
    QVERIFY(value->getIntValue() == 2);

    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "BundleRange2");
    QVERIFY(bundle->getLine() == 14);
    index = static_cast<ListNode *>(bundle->index());
    QVERIFY(index->getNodeType() == AST::List);
    QVERIFY(index->getChildren().size() == 1);
    indexList = bundle->index();
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    range = static_cast<RangeNode *>(indexList->getChildren().at(0));
    QVERIFY(range->getNodeType() == AST::Range);
    QVERIFY(range->getLine() == 14);
    value = static_cast<ValueNode *>(range->startIndex());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 3);
    value = static_cast<ValueNode *>(range->endIndex());
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getLine() == 14);
    QVERIFY(value->getIntValue() == 4);

    //    AudioIn[1] >> level(gain: 1.5) >> AudioOut[1];
    node = static_cast<StreamNode *>(nodes.at(8));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    QVERIFY(node->getLine() == 16);
    node = static_cast<StreamNode *>(node->getRight());
    QVERIFY(node->getLeft()->getNodeType() == AST::Function);
    functionNode = static_cast<FunctionNode *>(node->getLeft());
    QVERIFY(functionNode->getName() == "Level");
    vector<PropertyNode *> properties = functionNode->getProperties();
    QVERIFY(properties.size() == 1);
    PropertyNode *prop = properties[0];
    QVERIFY(prop->getName() == "gain");
    value = static_cast<ValueNode *>(prop->getValue());
    QVERIFY(value->getNodeType() == AST::Real);
    QVERIFY(value->getRealValue() == 1.5);

//    A[1:2,3,4] >> B[1,2,3:4] >> C[1,2:3,4] >> D[1,2,3,4];
    node = static_cast<StreamNode *>(nodes.at(9));
    QVERIFY(node->getNodeType() == AST::Stream);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    bundle = static_cast<BundleNode *>(node->getLeft());
    QVERIFY(bundle->getName() == "A");
    QVERIFY(bundle->getLine() == 18);
    ListNode *list = static_cast<ListNode *>(bundle->index());
    QVERIFY(list->getNodeType() == AST::List);
    QVERIFY(list->getLine() == 18);


    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildBundles()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/bundle.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 8);
    BlockNode *block = static_cast<BlockNode *>(nodes.at(1));
    QVERIFY(block->getNodeType() == AST::BlockBundle);
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 7);
    BundleNode *bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Integer");
    QVERIFY(bundle->getLine() == 7);
    QVERIFY(bundle->getChildren().size() == 1);
    ListNode *indexList = static_cast<ListNode *>(bundle->index());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    ValueNode *valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(bundle->getLine() == 7);
    QVERIFY(block->getProperties().size() == 2);
    PropertyNode *property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    AST *propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    ListNode *listnode = static_cast<ListNode *>(propertyValue);
//    QVERIFY(listnode->getListType() == AST::Int);
    vector<AST *>listValues = listnode->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Int);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Expression);
    QVERIFY(listValues.at(2)->getNodeType() == AST::Int);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    // Next Block - Float list
    block = static_cast<BlockNode *>(nodes.at(2));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 12);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Float");
    QVERIFY(bundle->getLine() == 12);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 4);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
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
    property = static_cast<PropertyNode *>(block->getProperties().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - String list
    block = static_cast<BlockNode *>(nodes.at(3));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 17);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "String");
    QVERIFY(bundle->getLine() == 17);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 5);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
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
    block = static_cast<BlockNode *>(nodes.at(4));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 23);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "UVar");
    QVERIFY(bundle->getLine() == 23);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 7);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 7);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Name);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Name);
    QVERIFY(listValues.at(2)->getNodeType() == AST::Name);
    QVERIFY(listValues.at(3)->getNodeType() == AST::Name);
    QVERIFY(listValues.at(4)->getNodeType() == AST::Name);
    QVERIFY(listValues.at(5)->getNodeType() == AST::Name);
    QVERIFY(listValues.at(6)->getNodeType() == AST::Name);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    // Next Block - ArrayList list
    block = static_cast<BlockNode *>(nodes.at(5));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 28);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "ArrayList");
    QVERIFY(bundle->getLine() == 28);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 2);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 2);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Bundle);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Bundle);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    // Next Block - BlockList list
    block = static_cast<BlockNode *>(nodes.at(6));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 33);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "BlockList");
    QVERIFY(bundle->getLine() == 33);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Block);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Block);
    property = static_cast<PropertyNode *>(block->getProperties().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - BlockBundleList list
    block = static_cast<BlockNode *>(nodes.at(7));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 38);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "BlockBundleList");
    QVERIFY(bundle->getLine() == 38);
    QVERIFY(bundle->getChildren().size() == 1);
    indexList = static_cast<ListNode *>(bundle->index());
    QVERIFY(indexList->getNodeType() == AST::List);
    QVERIFY(indexList->size() == 1);
    valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::BlockBundle);
    QVERIFY(listValues.at(1)->getNodeType() == AST::BlockBundle);
    QVERIFY(listValues.at(2)->getNodeType() == AST::BlockBundle);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

//    // Next Block - IntegerList list
//    block = static_cast<BlockNode *>(nodes.at(7));
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
//    block = static_cast<BlockNode *>(nodes.at(8));
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

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildBlocks()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/block.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 5);
    AST *node = nodes.at(0);
    QVERIFY(node->getNodeType() == AST::Block);
    vector<PropertyNode *> properties = static_cast<BlockNode *>(node)->getProperties();
    QVERIFY(properties.size() == 2);
    PropertyNode *property = properties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "rate");
    AST *propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Name);
    QVERIFY(static_cast<NameNode *>(propertyValue)->getName() == "AudioRate");
    property = properties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "meta");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Guitar input.");

    node = nodes.at(1);
    QVERIFY(node->getNodeType() == AST::Block);
    properties = static_cast<BlockNode *>(node)->getProperties();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getIntValue() == 5);
    property = properties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "meta");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Integer Value.");

    // No properties
    node = nodes.at(2);
    QVERIFY(node->getNodeType() == AST::Block);
    properties = static_cast<BlockNode *>(node)->getProperties();
    QVERIFY(properties.size() == 0);

    // Property is an object
    node = nodes.at(3);
    QVERIFY(node->getNodeType() == AST::Block);
    properties = static_cast<BlockNode *>(node)->getProperties();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Block);
    BlockNode *object = static_cast<BlockNode *>(propertyValue);
    QVERIFY(object->getName() == "");
    QVERIFY(object->getObjectType() == "");
    vector<PropertyNode *> objProperties = static_cast<BlockNode *>(object)->getProperties();
    QVERIFY(objProperties.size() == 2);
    property = objProperties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "prop1");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getIntValue() == 5);
    property = objProperties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "prop2");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "hello");
    property = properties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "meta");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Block as Property");

    node = nodes.at(4);
    QVERIFY(node->getNodeType() == AST::Block);
    properties = static_cast<BlockNode *>(node)->getProperties();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "process");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::List);
    ListNode *listnode = static_cast<ListNode *>(propertyValue);
    QVERIFY(listnode->getNodeType() == AST::List);
    QVERIFY(listnode->getChildren().size() == 1);
    StreamNode *streamNode = static_cast<StreamNode *>(listnode->getChildren().at(0));
    QVERIFY(streamNode->getChildren().size() == 2);
    QVERIFY(streamNode->getChildren().at(0)->getNodeType() == AST::Function);
    QVERIFY(streamNode->getChildren().at(1)->getNodeType() == AST::Bundle);
    property = properties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(property->getName() == "meta");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Stream property");

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildBasic()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/platform.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 15);
    PlatformNode *node = static_cast<PlatformNode *>(nodes.at(0));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "PufferFish");
    QVERIFY(node->version() == 1.1);
    QVERIFY(node->hwPlatform() == "PufferFish");
    QVERIFY(node->hwVersion() == -1.0);
    QVERIFY(node->getChildren().size() == 0);
    QVERIFY(node->getLine() == 1);

    node = static_cast<PlatformNode *>(nodes.at(1));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == -1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == -1.0);
    QVERIFY(node->getChildren().size() == 0);
    QVERIFY(node->getLine() == 3);

    node = static_cast<PlatformNode *>(nodes.at(2));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == 1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == -1.0);
    QVERIFY(node->getChildren().size() == 0);
    QVERIFY(node->getLine() == 4);

    node = static_cast<PlatformNode *>(nodes.at(3));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == 1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == 1.0);
    QVERIFY(node->getChildren().size() == 0);
    QVERIFY(node->getLine() == 5);

    node = static_cast<PlatformNode *>(nodes.at(4));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == -1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == -1.0);
    QVERIFY(node->getChildren().size() == 1);
    NameNode *nameNode = static_cast<NameNode *>(node->getChildren().at(0));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "Arduino");
    QVERIFY(node->getLine() == 7);

    node = static_cast<PlatformNode *>(nodes.at(5));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == 1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == -1.0);
    QVERIFY(node->getChildren().size() == 1);
    nameNode = static_cast<NameNode *>(node->getChildren().at(0));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "Arduino");
    QVERIFY(node->getLine() == 8);

    node = static_cast<PlatformNode *>(nodes.at(6));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == 1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == 1.0);
    QVERIFY(node->getChildren().size() == 1);
    nameNode = static_cast<NameNode *>(node->getChildren().at(0));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "Arduino");
    QVERIFY(node->getLine() == 9);

    node = static_cast<PlatformNode *>(nodes.at(7));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == -1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == -1.0);
    QVERIFY(node->getChildren().size() == 2);
    nameNode = static_cast<NameNode *>(node->getChildren().at(0));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "Arduino");
    nameNode = static_cast<NameNode *>(node->getChildren().at(1));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "RPi");
    QVERIFY(node->getLine() == 11);

    node = static_cast<PlatformNode *>(nodes.at(8));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == 1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == -1.0);
    QVERIFY(node->getChildren().size() == 2);
    nameNode = static_cast<NameNode *>(node->getChildren().at(0));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "Arduino");
    nameNode = static_cast<NameNode *>(node->getChildren().at(1));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "RPi");
    QVERIFY(node->getLine() == 12);

    node = static_cast<PlatformNode *>(nodes.at(9));
    QVERIFY(node->getNodeType() == AST::Platform);
    QVERIFY(node->platformName() == "Gamma");
    QVERIFY(node->version() == 1.0);
    QVERIFY(node->hwPlatform() == "PC");
    QVERIFY(node->hwVersion() == 1.0);
    QVERIFY(node->getChildren().size() == 2);
    nameNode = static_cast<NameNode *>(node->getChildren().at(0));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "Arduino");
    nameNode = static_cast<NameNode *>(node->getChildren().at(1));
    QVERIFY(nameNode->getNodeType() == AST::Name);
    QVERIFY(nameNode->getName() == "RPi");
    QVERIFY(node->getLine() == 13);

    ImportNode *importnode = static_cast<ImportNode *>(nodes.at(10));
    QVERIFY(importnode->getNodeType() == AST::Import);
    QVERIFY(importnode->importName() == "file");
    QVERIFY(importnode->importAlias() == "");
    QVERIFY(importnode->getLine() == 15);

    importnode = static_cast<ImportNode *>(nodes.at(11));
    QVERIFY(importnode->getNodeType() == AST::Import);
    QVERIFY(importnode->importName() == "file");
    QVERIFY(importnode->importAlias() == "file");
    QVERIFY(importnode->getLine() == 16);

    importnode = static_cast<ImportNode *>(nodes.at(12));
    QVERIFY(importnode->getNodeType() == AST::Import);
    QVERIFY(importnode->importName() == "File");
    QVERIFY(importnode->importAlias() == "file");
    QVERIFY(importnode->getLine() == 17);

    ForNode *fornode = static_cast<ForNode *>(nodes.at(13));
    QVERIFY(fornode->getNodeType() == AST::For);
    vector<AST *> fornames = fornode->getChildren();
    QVERIFY(fornames.size() == 1);
    nameNode = static_cast<NameNode *>(fornames.at(0));
    QVERIFY(nameNode->getName() == "Gamma");
    QVERIFY(fornode->getLine() == 19);

    fornode = static_cast<ForNode *>(nodes.at(14));
    QVERIFY(fornode->getNodeType() == AST::For);
    fornames = fornode->getChildren();
    QVERIFY(fornames.size() == 2);
    nameNode = static_cast<NameNode *>(fornames.at(0));
    QVERIFY(nameNode->getName() == "Gamma");
    nameNode = static_cast<NameNode *>(fornames.at(1));
    QVERIFY(nameNode->getName() == "PufferFish");
    QVERIFY(fornode->getLine() == 20);

    tree->deleteChildren();
    delete tree;

    tree = parse(QString(QFINDTESTDATA("data/simple.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    nodes = tree->getChildren();
    QVERIFY(nodes.size() == 2);
    QVERIFY(nodes.at(1)->getNodeType() == AST::Stream);
    StreamNode *stream_node = static_cast<StreamNode *>(nodes.at(1));
    QVERIFY(stream_node->getLine() == 5);
    QVERIFY(stream_node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(stream_node->getRight()->getNodeType() == AST::Bundle);
    BundleNode *bundle = static_cast<BundleNode *>(stream_node->getLeft());
    QVERIFY(bundle->getName() == "AudioIn");
    QVERIFY(bundle->getLine() == 5);
    ListNode *leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::List);
    QVERIFY(leafnode->getLine() == 5);
    QVERIFY(leafnode->size() == 1);
    ValueNode *value = static_cast<ValueNode *>(leafnode->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(stream_node->getRight());
    QVERIFY(bundle->getName() == "AudioOut");
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::List);
    QVERIFY(leafnode->getLine() == 5);
    QVERIFY(leafnode->size() == 1);
    value = static_cast<ValueNode *>(leafnode->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 2);

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testParser()
{
    AST *tree;
    QStringList files;
    files << "data/platform.stream" << "data/simple.stream" << "data/array.stream" << "data/list.stream"
          << "data/stream.stream" << "data/block.stream"
          << "data/introBlock.stream"
          << "data/introConverter.stream" << "data/introFeedback.stream"
          << "data/introGenerator.stream" << "data/introProcessor.stream"
          << "data/introRemote.stream"
          << "data/functions.stream" << "data/test.stream" << "data/expansions.stream";
    foreach (QString file, files) {
        tree = parse(QString(QFINDTESTDATA(file)).toStdString().c_str());
        QVERIFY2(tree != NULL, QString("file:" + file).toStdString().c_str());
        tree->deleteChildren();
        delete tree;
    }
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
