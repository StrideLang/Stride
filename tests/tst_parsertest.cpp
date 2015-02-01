#include <QString>
#include <QtTest>

#include "ast.h"
#include "platformnode.h"
#include "streamnode.h"
#include "bundlenode.h"
#include "valuenode.h"

extern AST *parse(const char* fileName);

class ParserTest : public QObject
{
    Q_OBJECT

public:
    ParserTest();

private:

private Q_SLOTS:
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
    }
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
