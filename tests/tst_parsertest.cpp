#include <QString>
#include <QtTest>

#include "ast.h"
#include "platformnode.h"
#include "streamnode.h"
#include "bundlenode.h"
#include "valuenode.h"
#include "propertynode.h"
#include "namenode.h"

extern AST *parse(const char* fileName);

class ParserTest : public QObject
{
    Q_OBJECT

public:
    ParserTest();

private:

private Q_SLOTS:
    void testTreeBuildLists();
    void testTreeBuildBlocks();
    void testTreeBuildBasic();
    void testParser();
};

ParserTest::ParserTest()
{

}

void ParserTest::testTreeBuildBasic()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/platform.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 1);
    QVERIFY(nodes.at(0)->getNodeType() == AST::Platform);
    PlatformNode *node = static_cast<PlatformNode *>(nodes.at(0));
    QVERIFY(node->platformName() == "PufferFish");
    QVERIFY(node->version() == 1.1f);
    delete tree;

    tree = parse(QString(QFINDTESTDATA("data/simple.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    nodes = tree->getChildren();
    QVERIFY(nodes.size() == 2);
    QVERIFY(nodes.at(1)->getNodeType() == AST::Stream);
    StreamNode *stream_node = static_cast<StreamNode *>(nodes.at(1));
    QVERIFY(stream_node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(stream_node->getRight()->getNodeType() == AST::Bundle);
    BundleNode *bundle = static_cast<BundleNode *>(stream_node->getLeft());
    QVERIFY(bundle->name() == "AudioIn");
    AST *leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(stream_node->getRight());
    QVERIFY(bundle->name() == "AudioOut");
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);
    delete tree;
}

void ParserTest::testTreeBuildLists()
{
//    AST *tree;
//    tree = parse(QString(QFINDTESTDATA("data/list.stream")).toStdString().c_str());
//    QVERIFY(tree != NULL);
//    vector<AST *> nodes = tree->getChildren();
//    QVERIFY(nodes.size() == 9);
}

void ParserTest::testTreeBuildBlocks()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/block.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 3);
    AST *node = nodes.at(0);
    QVERIFY(node->getNodeType() == AST::Object);
    vector<AST *> properties = node->getChildren();
    QVERIFY(properties.size() == 2);
    AST *property = properties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "rate");
    AST *propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Name);
    QVERIFY(static_cast<NameNode *>(propertyValue)->getName() == "AudioRate");
    property = properties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "meta");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Guitar input.");


    node = nodes.at(1);
    QVERIFY(node->getNodeType() == AST::Object);
    properties = node->getChildren();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "value");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getIntValue() == 5);
    property = properties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "meta");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Integer Value.");

    node = nodes.at(2);
    QVERIFY(node->getNodeType() == AST::Object);
    properties = node->getChildren();
    QVERIFY(properties.size() == 0);

    delete tree;
}

void ParserTest::testParser()
{
    AST *tree;
    QStringList files;
    files << "data/platform.stream" << "data/simple.stream" << "data/array.stream" << "data/list.stream"
          << "data/introBlock.stream"
          << "data/introConverter.stream" << "data/introFeedback.stream"
          << "data/introGenerator.stream" << "data/introProcessor.stream"
          << "data/introFM.stream" << "data/introRemote.stream";
    foreach (QString file, files) {
        tree = parse(QString(QFINDTESTDATA(file)).toStdString().c_str());
        QVERIFY(tree != NULL);
//        delete tree; // FIXME this leaks
    }
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
