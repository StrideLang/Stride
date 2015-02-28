#include <QString>
#include <QtTest>

#include "streamparser.h"
#include "streamplatform.h"
#include "codegen.h"
#include "treewalker.h"

extern AST *parse(const char* fileName);

class ParserTest : public QObject
{
    Q_OBJECT

public:
    ParserTest();

private:

private Q_SLOTS:
    //Platform
    void testPlatform();
    void testPlatformCommonObjects();
    void testValueTypeBundleResolution();
    void testValueTypeExpressionResolution();

    // Parser
    void testTreeBuildNoneSwitch();
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

void ParserTest::testPlatform()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/platform.stream")).toStdString().c_str());

    QVERIFY(tree != NULL);
    Codegen generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(generator.isValid());
    QVERIFY(generator.platformIsValid());
}

void ParserTest::testPlatformCommonObjects()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/platformBasic.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    Codegen generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();

//    QVERIFY(errors.size() == 14);

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
}

void ParserTest::testValueTypeBundleResolution()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/bundleResolution.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    Codegen generator(QFINDTESTDATA("/../platforms"), tree);
    QVERIFY(generator.platformIsValid());
    QVERIFY(!generator.isValid());
    QList<LangError> errors = generator.getErrors();

    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.at(1)->getNodeType() == AST::BlockBundle);
    BlockNode *block = static_cast<BlockNode *>(nodes.at(1));
    QVERIFY(block->getObjectType() == "constant");
    BundleNode *bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Integers");
    QVERIFY(bundle->index()->getNodeType() == AST::Int);
    ValueNode *valueNode = static_cast<ValueNode *>(bundle->index());
    QVERIFY(valueNode->getIntValue() == 4);
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
    QVERIFY(bundle->index()->getNodeType() == AST::Int);
    valueNode = static_cast<ValueNode *>(bundle->index());
    QVERIFY(valueNode->getIntValue() == 4);
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
    QVERIFY(bundle->index()->getNodeType() == AST::Int);
    valueNode = static_cast<ValueNode *>(bundle->index());
    QVERIFY(valueNode->getIntValue() == 4);
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
    QVERIFY(error.lineNumber == 37);
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
    QVERIFY(error.lineNumber == 42);
    QVERIFY(error.errorTokens[0] == "Values_Mismatch");
    QVERIFY(error.errorTokens[1] == "3");
    QVERIFY(error.errorTokens[2] == "4");


    QVERIFY(nodes.at(11)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(11));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Values_Mismatch_2");
    error = errors.takeFirst();
    QVERIFY(error.lineNumber == 46);
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
    QVERIFY(error.lineNumber == 50);
    QVERIFY(error.errorTokens[0] == "Array_Float");
    QVERIFY(error.errorTokens[1] == "CRP");

    QVERIFY(nodes.at(13)->getNodeType() == AST::BlockBundle);
    block = static_cast<BlockNode *>(nodes.at(13));
    QVERIFY(block->getObjectType() == "constant");
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Array_String");
    error = errors.takeFirst();
    QVERIFY(error.type == LangError::IndexMustBeInteger);
    QVERIFY(error.lineNumber == 51);
    QVERIFY(error.errorTokens[0] == "Array_String");
    QVERIFY(error.errorTokens[1] == "CSP");
}

void ParserTest::testValueTypeExpressionResolution()
{

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
    QVERIFY(nodes.size() == 15);

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
    ValueNode *value = static_cast<ValueNode *>(bundle->getChildren().at(0));
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
    ExpressionNode *expr = static_cast<ExpressionNode *>(bundle->getChildren().at(0));
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
    expr = static_cast<ExpressionNode *>(bundle->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Add);
    QVERIFY(expr->getLine() == 26);
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
    expr = static_cast<ExpressionNode *>(bundle->getChildren().at(0));
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Divide);
    QVERIFY(expr->getLine() == 27);
    bundle = static_cast<BundleNode *>(expr->getLeft());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getName() == "SIZE");
    QVERIFY(bundle->getChildren().size() == 1);
    QVERIFY(bundle->getLine() == 27);
    value = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(value->getNodeType() == AST::Int);
    QVERIFY(value->getIntValue() == 1);
    QVERIFY(value->getLine() == 27);
    bundle = static_cast<BundleNode *>(expr->getRight());
    QVERIFY(bundle->getNodeType() == AST::Bundle);
    QVERIFY(bundle->getLine() == 27);
    expr = static_cast<ExpressionNode *>(bundle->getChildren()[0]);
    QVERIFY(expr->getNodeType() == AST::Expression);
    QVERIFY(expr->getExpressionType() == ExpressionNode::Subtract);
    QVERIFY(expr->getLeft()->getNodeType() == AST::Int);
    QVERIFY(expr->getRight()->getNodeType() == AST::Int);
    QVERIFY(expr->getLine() == 27);

    tree->deleteChildren();
    delete tree;
}

void ParserTest::testTreeBuildStream()
{
    AST *tree;
    tree = parse(QString(QFINDTESTDATA("data/stream.stream")).toStdString().c_str());
    QVERIFY(tree != NULL);
    vector<AST *> nodes = tree->getChildren();
    QVERIFY(nodes.size() == 7);

    // Val1 >> Val2 ;
    QVERIFY(nodes.at(0)->getNodeType() == AST::Stream);
    StreamNode *node = static_cast<StreamNode *>(nodes.at(0));
    QVERIFY(node->getLine() == 1);
    vector<AST *>streamParts = node->getChildren();
    QVERIFY(streamParts.size() == 2);
    AST *streamComp = streamParts.at(0);
    QVERIFY(streamComp->getNodeType() == AST::Name);
    QVERIFY(streamComp->getLine() == 1);
    NameNode *nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 1);
    streamComp = streamParts.at(1);
    QVERIFY(streamComp->getNodeType() == AST::Name);
    QVERIFY(streamComp->getLine() == 1);
    nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val2");
    QVERIFY(nameNode->getLine() == 1);

    // Func1() >> Func2() ;
    QVERIFY(nodes.at(1)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(1));
    QVERIFY(node->getLine() == 2);
    streamParts = node->getChildren();
    QVERIFY(streamParts.size() == 2);
    streamComp = streamParts.at(0);
    QVERIFY(streamComp->getLine() == 2);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    FunctionNode *functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func1");
    QVERIFY(functionNode->getLine() == 2);
    streamComp = streamParts.at(1);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    QVERIFY(streamComp->getLine() == 2);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func2");
    QVERIFY(functionNode->getLine() == 2);

    // Val1 >> Func1() >> Func2() >> Val2 ;
    QVERIFY(nodes.at(2)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(2));
    QVERIFY(node->getLine() == 4);
    streamComp = node->getLeft();
    QVERIFY(streamComp->getNodeType() == AST::Name);
    QVERIFY(streamComp->getLine() == 4);
    nameNode = static_cast<NameNode *>(streamComp);
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 4);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(node->getRight());
    streamComp = node->getLeft();
    QVERIFY(streamComp->getLine() == 4);
    QVERIFY(streamComp->getNodeType() == AST::Function);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func1");
    QVERIFY(functionNode->getLine() == 4);
    QVERIFY(node->getRight()->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(node->getRight());
    QVERIFY(node->getLine() == 5);
    streamComp = node->getLeft();
    QVERIFY(streamComp->getNodeType() == AST::Function);
    QVERIFY(streamComp->getLine() == 5);
    functionNode = static_cast<FunctionNode *>(streamComp);
    QVERIFY(functionNode->getName() == "Func2");
    QVERIFY(functionNode->getLine() == 5);
    QVERIFY(node->getRight()->getNodeType() == AST::Name);
    nameNode = static_cast<NameNode *>(node->getRight());
    QVERIFY(nameNode->getName() == "Val2");
    QVERIFY(nameNode->getLine() == 6);

//    Bundle1[1] >> Bundle2[2];
    QVERIFY(nodes.at(3)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(3));
    QVERIFY(node->getLine() == 8);
    QVERIFY(node->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    BundleNode *bundle = static_cast<BundleNode *>(node->getLeft());
    QVERIFY(bundle->getName() == "Bundle1");
    QVERIFY(bundle->getLine() == 8);
    AST *leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 8);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 1);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle2");
    QVERIFY(bundle->getLine() == 8);
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 8);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);

//    Val1 * 3 >> Bundle[2];
    QVERIFY(nodes.at(4)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(4));
    QVERIFY(node->getLeft()->getNodeType() == AST::Expression);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    QVERIFY(node->getLine() == 9);
    ExpressionNode *expression = static_cast<ExpressionNode *>(node->getLeft());
    QVERIFY(expression->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expression->getLeft()->getNodeType() == AST::Name);
    QVERIFY(expression->getLine() == 9);
    nameNode = static_cast<NameNode *>(expression->getLeft());
    QVERIFY(nameNode->getName() == "Val1");
    QVERIFY(nameNode->getLine() == 9);
    QVERIFY(expression->getRight()->getNodeType() == AST::Int);
    QVERIFY(static_cast<ValueNode *>(expression->getRight())->getIntValue() == 3);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle");
    QVERIFY(bundle->getLine() == 9);
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 9);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);

//    Bundle1[1] * 0.5 >> Bundle2[2];
    QVERIFY(nodes.at(5)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(5));
    QVERIFY(node->getLeft()->getNodeType() == AST::Expression);
    QVERIFY(node->getRight()->getNodeType() == AST::Bundle);
    QVERIFY(node->getLine() == 10);
    expression = static_cast<ExpressionNode *>(node->getLeft());
    QVERIFY(expression->getExpressionType() == ExpressionNode::Multiply);
    QVERIFY(expression->getLeft()->getNodeType() == AST::Bundle);
    QVERIFY(expression->getLine() == 10);
    bundle = static_cast<BundleNode *>(expression->getLeft());
    QVERIFY(bundle->getName() == "Bundle1");
    QVERIFY(bundle->getLine() == 10);
    QVERIFY(expression->getRight()->getNodeType() == AST::Real);
    QVERIFY(static_cast<ValueNode *>(expression->getRight())->getFloatValue() == 0.5f);
    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "Bundle2");
    QVERIFY(bundle->getLine() == 10);
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 10);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);

    //    BundleRange[1:2] >> BundleRange2[3:4];
    QVERIFY(nodes.at(6)->getNodeType() == AST::Stream);
    node = static_cast<StreamNode *>(nodes.at(6));
    QVERIFY(node->getLeft()->getNodeType() == AST::BundleRange);
    QVERIFY(node->getRight()->getNodeType() == AST::BundleRange);
    QVERIFY(node->getLine() == 12);
    bundle = static_cast<BundleNode *>(node->getLeft());
    QVERIFY(bundle->getName() == "BundleRange");
    QVERIFY(bundle->getLine() == 12);
    leafnode = bundle->startIndex();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 12);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 1);
    leafnode = bundle->endIndex();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 12);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 2);

    bundle = static_cast<BundleNode *>(node->getRight());
    QVERIFY(bundle->getName() == "BundleRange2");
    QVERIFY(bundle->getLine() == 12);
    leafnode = bundle->startIndex();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 12);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 3);
    leafnode = bundle->endIndex();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 12);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 4);

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
    QVERIFY(node->getLine() == 4);
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
    AST *leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 5);
    QVERIFY(static_cast<ValueNode *>(leafnode)->getIntValue() == 1);

    bundle = static_cast<BundleNode *>(stream_node->getRight());
    QVERIFY(bundle->getName() == "AudioOut");
    leafnode = bundle->index();
    QVERIFY(leafnode->getNodeType() == AST::Int);
    QVERIFY(leafnode->getLine() == 5);
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
    QVERIFY(block->getLine() == 7);
    BundleNode *bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Integer");
    QVERIFY(bundle->getLine() == 7);
    QVERIFY(bundle->getChildren().size() == 1);
    ValueNode *valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
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
    block = static_cast<BlockNode *>(nodes.at(1));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 12);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "Float");
    QVERIFY(bundle->getLine() == 12);
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
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
    block = static_cast<BlockNode *>(nodes.at(2));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 17);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "String");
    QVERIFY(bundle->getLine() == 17);
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
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
    block = static_cast<BlockNode *>(nodes.at(3));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 23);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "UVar");
    QVERIFY(bundle->getLine() == 23);
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
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
    block = static_cast<BlockNode *>(nodes.at(4));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 28);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "ArrayList");
    QVERIFY(bundle->getLine() == 28);
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
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
    block = static_cast<BlockNode *>(nodes.at(5));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 33);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "BlockList");
    QVERIFY(bundle->getLine() == 33);
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
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
    block = static_cast<BlockNode *>(nodes.at(6));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 38);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "BlockBundleList");
    QVERIFY(bundle->getLine() == 38);
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
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

    // Next Block - IntegerList list
    block = static_cast<BlockNode *>(nodes.at(7));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 46);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "IntegerList");
    QVERIFY(bundle->getLine() == 46);
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::List);
    QVERIFY(listValues.at(1)->getNodeType() == AST::List);
    QVERIFY(listValues.at(2)->getNodeType() == AST::List);
    property = block->getProperties().at(1);
    QVERIFY(property->getName() == "meta");

    // Next Block - IntegerList list
    block = static_cast<BlockNode *>(nodes.at(8));
    QVERIFY(block->getObjectType() == "constant");
    QVERIFY(block->getLine() == 52);
    bundle = block->getBundle();
    QVERIFY(bundle->getName() == "IntegerList");
    QVERIFY(bundle->getLine() == 53);
    QVERIFY(bundle->getChildren().size() == 1);
    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
    QVERIFY(valueNode->getNodeType() == AST::Int);
    QVERIFY(valueNode->getIntValue() == 3);
    QVERIFY(block->getProperties().size() == 2);
    property = block->getProperties().at(0);
    QVERIFY(property->getName() == "value");
    propertyValue = property->getValue();
    QVERIFY(propertyValue->getNodeType() == AST::List);
    listValues = propertyValue->getChildren();
    QVERIFY(listValues.size() == 3);
    QVERIFY(listValues.at(0)->getNodeType() == AST::List);
    QVERIFY(listValues.at(1)->getNodeType() == AST::List);
    QVERIFY(listValues.at(2)->getNodeType() == AST::List);
    property = block->getProperties().at(1);
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
    QVERIFY(propertyValue->getNodeType() == AST::Stream);
    StreamNode *streamNode = static_cast<StreamNode *>(propertyValue);
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
        QVERIFY2(tree != NULL, QString("file:" + file).toStdString().c_str());
        tree->deleteChildren();
        delete tree;
    }
}

QTEST_APPLESS_MAIN(ParserTest)

#include "tst_parsertest.moc"
