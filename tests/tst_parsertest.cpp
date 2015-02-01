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
    QString path;

private Q_SLOTS:
    void testTreeBuilding();
    void testParser();
};

ParserTest::ParserTest()
{
    path = "../../StreamStack/tests/data/";
}

void ParserTest::testTreeBuilding()
{
    AST *tree;
    tree = parse(QString(path + "platform.stream").toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 1);
    QVERIFY(nodes.at(0)->getNodeType() == AST::Platform);
    PlatformNode *node = static_cast<PlatformNode *>(nodes.at(0));
    QVERIFY(node->platformName() == "PufferFish");
    QVERIFY(node->version() == 1.1f);

    tree = parse(QString(path + "simple.stream").toStdString().c_str());
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
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 1);
}

void ParserTest::testParser()
{
    AST *tree;
    QStringList files;
    files << "platform.stream" << "simple.stream" << "array.stream" << "list.stream"
          << "introBlock.stream"
          << "introConverter.stream" << "introFeedback.stream"
          << "introGenerator.stream" << "introProcessor.stream"
          << "introFM.stream" << "introRemote.stream";
    foreach (QString file, files) {
        tree = parse(QString(path + file).toStdString().c_str());
        QVERIFY(tree != NULL);
    }
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
