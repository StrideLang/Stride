#include <QString>
#include <QtTest>

#include "ast.h"
#include "platformnode.h"
#include "streamnode.h"
#include "bundlenode.h"
#include "blocknode.h"
#include "valuenode.h"
#include "propertynode.h"
#include "namenode.h"
#include "functionnode.h"
#include "expressionnode.h"
#include "listnode.h"

extern AST *parse(const char* fileName);

class ParserTest : public QObject
{
    Q_OBJECT

public:
    ParserTest();

private:

private Q_SLOTS:
    void testTreeBuildArray();
    void testTreeBuildStream();
    void testTreeBuildLists();
    void testTreeBuildBlocks();
    void testTreeBuildBasic();
    void testParser();
};

ParserTest::ParserTest()
{

}

void ParserTest::testTreeBuildArray()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/array.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 15);

    QVERIFY(nodes.at(0)->getNodeType() == AST::Block);
    BlockNode *block = static_cast<BlockNode *>(nodes.at(0));
    QVERIFY(block->getObjectType() == "constant");

    QVERIFY(nodes.at(1)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(1));
    QVERIFY(block->getObjectType() == "constant");
    BundleNode *bundle = block->getBundle();
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    ValueNode *value = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 4);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    PropertyNode *property = static_cast<PropertyNode *>(block->getChildren().at(0));
    ListNode * listnode = static_cast<ListNode *>(property->getValue());
    QVERIFY(listnode->getNodeType() == AST::List);
    QVERIFY(listnode->getChildren().size() == 4);
    value = static_cast<ValueNode *>(listnode->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 16);
    value = static_cast<ValueNode *>(listnode->getChildren().at(1));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 32);
    value = static_cast<ValueNode *>(listnode->getChildren().at(2));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 64);
    value = static_cast<ValueNode *>(listnode->getChildren().at(3));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 128);

    // constant Array [ ( CONST * 2 ) + 1 ] {}
    block = static_cast<BlockNode *>(nodes.at(6));
    QVERIFY(block->getNodeType() == AST::BlockBundle);
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Parens");
    QVERIFY(bundle->getChildren().size() == 1);
    ExpressionNode *expr = static_cast<ExpressionNode *>(bundle->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Add);
    ExpressionNode *expr2 = static_cast<ExpressionNode *>(expr->getLeft());
    QVERIFY(expr2->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expr2->getLeft()->getNodeType() == AST::Name);
    QVERIFY(expr2->getRight()->getNodeType() == AST::Int);

    // constant Array_Expr [ SIZE [1] + SIZE [1 * 2] ] {}
    block = static_cast<BlockNode *>(nodes.at(8));
    QVERIFY(block->getNodeType() == AST::BlockBundle);
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Expr");
    QVERIFY(bundle->getChildren().size() == 1);
    expr = static_cast<ExpressionNode *>(bundle->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Add);
    bundle = static_cast<BundleNode *>(expr->getLeft());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    value = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(expr->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    expr = static_cast<ExpressionNode *>(bundle->getChildren()[0]);
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expr->getLeft()->getNodeType() == AST::Int);
    QVERIFY(expr->getRight()->getNodeType() == AST::Int);

    // constant Array_Expr2 [ SIZE [1] / SIZE [1 - 2] ] {}
    block = static_cast<BlockNode *>(nodes.at(9));
    QVERIFY(block->getNodeType() == AST::BlockBundle);
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_Expr2");
    QVERIFY(bundle->getChildren().size() == 1);
    expr = static_cast<ExpressionNode *>(bundle->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Divide);
    bundle = static_cast<BundleNode *>(expr->getLeft());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    value = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(expr->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    expr = static_cast<ExpressionNode *>(bundle->getChildren()[0]);
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Subtract);
    QVERIFY(expr->getLeft()->getNodeType() == AST::Int);
    QVERIFY(expr->getRight()->getNodeType() == AST::Int);

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildStream()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/stream.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 6);

    // Val1 >> Val2 ;
    QVERIFY(nodes.at(0)->getNodeType() == AST::Stream);
    StreamNode *node = static_cast<StreamNode *>(nodes.at(0));
    vector<AST *>streamParts = node->getChildren();
    QVERIFY(streamParts.size() == 2);
    AST *streamComp = streamParts.at(0);
    QVERIFY(streamComp->getNodeType() == AST::Name);
    NameNode *nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val1");
    streamComp = streamParts.at(1);
    QVERIFY(streamComp->getNodeType() == AST::Name);
    nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val2");

    // Func1() >> Func2() ;
    QVERIFY(nodes.at(1)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(1));
    streamParts = node->getChildren();
    QVERIFY(streamParts.size() == 2);
    streamComp = streamParts.at(0);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    FunctionNode *functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func1");
    streamComp = streamParts.at(1);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func2");

    // Val1 >> Func1() >> Func2() >> Val2 ;
    QVERIFY(nodes.at(2)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(2));
    streamComp = node->getLeft();
    QVERIFY(streamComp->getNodeType() == AST::Name);
    nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(node->getRight());
    streamComp = node->getLeft();
    QVERIFY(streamComp->getNodeType() == AST::Function);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func1");
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(node->getRight());
    streamComp = node->getLeft();
    QVERIFY(streamComp->getNodeType() == AST::Function);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func2");
    QVERIFY(node->getRight()->getNodeType() == AST::Name);
    nameNode = static_cast<NameNode *>(node->getRight());
    QVERIFY(nameNode->getName() == "Val2");

//    Bundle1[1] >> Bundle2[2];
    QVERIFY(nodes.at(3)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(3));
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    BundleNode *bundle = static_cast<BundleNode *>(node->getLeft());
    QVERIFY(bundle->getName() == "Bundle1");
    AST *leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle2");
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);

//    Val1 * 3 >> Bundle[2];
    QVERIFY(nodes.at(4)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(4));
    QVERIFY(node->getLeft()->getNodeType() == AST::Expression);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    ExpressionNode *expression = static_cast<ExpressionNode *>(node->getLeft());
    QVERIFY(expression->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expression->getLeft()->getNodeType() == AST::Name);
    nameNode = static_cast<NameNode *>(expression->getLeft());
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(expression->getRight()->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(expression->getRight())->getIntValue() == 3);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle");
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);

//    Bundle1[1] * 0.5 >> Bundle2[2];
    QVERIFY(nodes.at(5)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(5));
    QVERIFY(node->getLeft()->getNodeType() == AST::Expression);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    expression = static_cast<ExpressionNode *>(node->getLeft());
    QVERIFY(expression->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expression->getLeft()->getNodeType() == AST::Bundle);
    bundle = static_cast<BundleNode *>(expression->getLeft());
    QVERIFY(bundle->getName() == "Bundle1");
    QVERIFY(expression->getRight()->getNodeType() == AST::Float);
    QVERIFY(static_cast<ValueNode *>(expression->getRight())->getFloatValue() == 0.5f);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle2");
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);

    tree->deleteChildren();
    delete tree;
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
    tree->deleteChildren();
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
    QVERIFY(bundle->getName() == "AudioIn");
    AST *leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(stream_node->getRight());
    QVERIFY(bundle->getName() == "AudioOut");
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildLists()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/list.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 9);
    QVERIFY(nodes.at(0)->getNodeType() == AST::BlockBundle);
    BlockNode *block = static_cast<BlockNode *>(nodes.at(0));
    QVERIFY(block->getObjectType() == "constant");
    BundleNode *bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Integer");
    QVERIFY(bundle->getChildren().size() == 1);
    ValueNode *valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    PropertyNode *property = static_cast<PropertyNode *>(block->getChildren().at(0));
    QVERIFY(property->getName() == "value");
    AST *propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    vector<AST *>listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Int);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Expression);
    QVERIFY(listValues.at(2)->getNodeType() == AST::Int);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - Float list
    block = static_cast<BlockNode *>(nodes.at(1));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Float");
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 4);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(0));
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 4);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Float);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Expression);
    QVERIFY(listValues.at(2)->getNodeType() == AST::Float);
    QVERIFY(listValues.at(3)->getNodeType() == AST::Float);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - String list
    block = static_cast<BlockNode *>(nodes.at(2));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "String");
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 5);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(0));
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 5);
    QVERIFY(listValues.at(0)->getNodeType() == AST::String);
    QVERIFY(listValues.at(1)->getNodeType() == AST::String);
    QVERIFY(listValues.at(2)->getNodeType() == AST::String);
    QVERIFY(listValues.at(3)->getNodeType() == AST::String);
    QVERIFY(listValues.at(4)->getNodeType() == AST::String);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - UVar list
    block = static_cast<BlockNode *>(nodes.at(3));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "UVar");
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 7);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(0));
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
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - ArrayList list
    block = static_cast<BlockNode *>(nodes.at(4));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "ArrayList");
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 2);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(0));
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 2);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Bundle);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Bundle);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - BlockList list
    block = static_cast<BlockNode *>(nodes.at(5));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "BlockList");
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(0));
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::Block);
    QVERIFY(listValues.at(1)->getNodeType() == AST::Block);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - BlockBundleList list
    block = static_cast<BlockNode *>(nodes.at(6));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "BlockBundleList");
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(0));
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::BlockBundle);
    QVERIFY(listValues.at(1)->getNodeType() == AST::BlockBundle);
    QVERIFY(listValues.at(2)->getNodeType() == AST::BlockBundle);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - IntegerList list
    block = static_cast<BlockNode *>(nodes.at(7));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "IntegerList");
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(0));
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::List);
    QVERIFY(listValues.at(1)->getNodeType() == AST::List);
    QVERIFY(listValues.at(2)->getNodeType() == AST::List);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

    // Next Block - IntegerList list
    block = static_cast<BlockNode *>(nodes.at(8));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "IntegerList");
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getChildren().size() == 2);
    QVERIFY(block->getChildren().at(0)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(0));
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::List);
    QVERIFY(listValues.at(1)->getNodeType() == AST::List);
    QVERIFY(listValues.at(2)->getNodeType() == AST::List);
    QVERIFY(block->getChildren().at(1)->getNodeType() == AST::Property);
    property = static_cast<PropertyNode *>(block->getChildren().at(1));
    QVERIFY(property->getName() == "meta");

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
    QVERIFY(node->getNodeType() == AST::Block);
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

    // No properties
    node = nodes.at(2);
    QVERIFY(node->getNodeType() == AST::Block);
    properties = node->getChildren();
    QVERIFY(properties.size() == 0);

    // Property is an object
    node = nodes.at(3);
    QVERIFY(node->getNodeType() == AST::Block);
    properties = node->getChildren();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "value");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Block);
    BlockNode *object = static_cast<BlockNode *>(propertyValue);
    QVERIFY(object->getName() == "");
    QVERIFY(object->getObjectType() == "");
    vector<AST *> objProperties = object->getChildren();
    QVERIFY(objProperties.size() == 2);
    property = objProperties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "prop1");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getIntValue() == 5);
    property = objProperties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "prop2");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "hello");
    property = properties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "meta");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Block as Property");

    node = nodes.at(4);
    QVERIFY(node->getNodeType() == AST::Block);
    properties = node->getChildren();
    QVERIFY(properties.size() == 2);
    property = properties.at(0);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "process");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::Stream);
    StreamNode *streamNode = static_cast<StreamNode *>(propertyValue);
    QVERIFY(streamNode->getChildren().size() == 2);
    QVERIFY(streamNode->getChildren().at(0)->getNodeType() == AST::Function);
    QVERIFY(streamNode->getChildren().at(1)->getNodeType() == AST::Bundle);
    property = properties.at(1);
    QVERIFY(property != NULL && property->getChildren().size() == 1);
    QVERIFY(static_cast<PropertyNode *>(property)->getName() == "meta");
    propertyValue = property->getChildren().at(0);
    QVERIFY(propertyValue->getNodeType() == AST::String);
    QVERIFY(static_cast<ValueNode *>(propertyValue)->getStringValue() == "Stream property");

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
          << "data/introFM.stream" << "data/introRemote.stream";
    foreach (QString file, files) {
        tree = parse(QString(QFINDTESTDATA(file)).toStdString().c_str());
        QVERIFY(tree != NULL);
        tree->deleteChildren(); // FIXME this leaks, but still crashes...
        delete tree;
    }
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
