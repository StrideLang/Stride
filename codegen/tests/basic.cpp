#include "gtest/gtest.h"

#include "stride/parser/ast.h"
#include "stride/codegen/astfunctions.hpp"
#include "stride/parser/blocknode.h"
#include "stride/parser/bundlenode.h"
#include "stride/parser/declarationnode.h"
#include "stride/parser/expressionnode.h"
#include "stride/parser/functionnode.h"
#include "stride/parser/importnode.h"
#include "stride/parser/platformnode.h"
#include "stride/parser/rangenode.h"
#include "stride/parser/valuenode.h"

/*
TEST(Basic, ModuleDomains) {
    ASTNode tree;
    tree = ASTFunctions::parseFile(BUILDPATH
                                   "/tests/data/12_modules_domains.stride");
    EXPECT_TRUE(tree != nullptr);
    CodeResolver resolver(tree, STRIDEROOT);
    resolver.process();
    CodeValidator validator(tree, CodeValidator::NO_RATE_VALIDATION);
    EXPECT_TRUE(validator.isValid());

    auto block =
        std::static_pointer_cast<DeclarationNode>(tree->getChildren()[1]);
    EXPECT_TRUE(block->getNodeType() == AST::Declaration);

    // Both the input and output blocks should be autodeclared to belong to the
    // output domain
    ListNode *blockList =
        static_cast<ListNode *>(block->getPropertyValue("blocks").get());
    EXPECT_TRUE(blockList->getNodeType() == AST::List);
    DeclarationNode *internalBlock =
        ASTQuery::findDeclarationByName(
            std::string("Input"), {{block, blockList->getChildren()}}, nullptr)
            .get();
    EXPECT_TRUE(internalBlock->getNodeType() == AST::Declaration);
    auto domainValue = std::static_pointer_cast<PortPropertyNode>(
        internalBlock->getPropertyValue("domain"));
    EXPECT_TRUE(domainValue);
    EXPECT_TRUE(domainValue->getNodeType() == AST::PortProperty);
    EXPECT_TRUE(domainValue->getName() == "OutputPort");
    EXPECT_TRUE(domainValue->getPortName() == "domain");

    auto reads = internalBlock->getCompilerProperty("reads")->getChildren();
    auto writes = internalBlock->getCompilerProperty("writes")->getChildren();

    EXPECT_TRUE(reads.size() == 1);
    EXPECT_TRUE(writes.size() == 0);

    auto readDomain = std::static_pointer_cast<PortPropertyNode>(reads.at(0));
    EXPECT_TRUE(readDomain->getNodeType() == AST::PortProperty);
    EXPECT_TRUE(readDomain->getPortName() == "domain");
    EXPECT_TRUE(readDomain->getName() == "OutputPort");

    internalBlock =
        ASTQuery::findDeclarationByName(
            std::string("Output"), {{block, blockList->getChildren()}}, nullptr)
            .get();
    EXPECT_TRUE(internalBlock->getNodeType() == AST::Declaration);
    domainValue = std::static_pointer_cast<PortPropertyNode>(
        internalBlock->getPropertyValue("domain"));
    EXPECT_TRUE(domainValue);
    EXPECT_TRUE(domainValue->getNodeType() == AST::PortProperty);
    EXPECT_TRUE(domainValue->getName() == "OutputPort");
    EXPECT_TRUE(domainValue->getPortName() == "domain");

    reads = internalBlock->getCompilerProperty("reads")->getChildren();
    writes = internalBlock->getCompilerProperty("writes")->getChildren();

    EXPECT_TRUE(reads.size() == 0);
    EXPECT_TRUE(writes.size() == 1);

    auto writeDomain = std::static_pointer_cast<PortPropertyNode>(writes.at(0));
    EXPECT_TRUE(writeDomain->getNodeType() == AST::PortProperty);
    EXPECT_TRUE(writeDomain->getPortName() == "domain");
    EXPECT_TRUE(writeDomain->getName() == "OutputPort");

    //    ListNode *streamList = static_cast<ListNode
    //    *>(block->getPropertyValue("streams").get()); ValueNode *constant =
    //    static_cast<ValueNode
    //    *>(streamList->getChildren()[0]->getChildren()[0].get());
    //    EXPECT_TRUE(constant->getNodeType() == AST::Int);
    //    EXPECT_TRUE(constant->getDomain());
    //    PortPropertyNode *domain = static_cast<PortPropertyNode
    //    *>(constant->getDomain().get()); EXPECT_TRUE(domain->getNodeType() ==
    //    AST::PortProperty); EXPECT_TRUE(domain->getPortName() == "domain");
    //    EXPECT_TRUE(domain->getName() == "OutputPort");
}

void ParserTest::testModules() {
    ASTNode tree;
    tree = ASTFunctions::parseFile(TESTS_SOURCE_DIR "basic/11_modules.stride");
    EXPECT_TRUE(tree != nullptr);
    CodeResolver resolver(tree, STRIDEROOT);
    resolver.process();
    CodeValidator validator(tree, CodeValidator::NO_RATE_VALIDATION);
    EXPECT_TRUE(validator.isValid());

    auto moduleNode =
        std::static_pointer_cast<DeclarationNode>(tree->getChildren().at(1));
    EXPECT_TRUE(moduleNode->getName() == "SizeTest");
    ListNode *blockList =
        static_cast<ListNode *>(moduleNode->getPropertyValue("blocks").get());
    EXPECT_TRUE(blockList->getNodeType() == AST::List);
    for (size_t i = 0; i < blockList->getChildren().size(); i++) {
        AST *member = blockList->getChildren().at(i).get();
        if (member->getNodeType() == AST::Declaration) {
            DeclarationNode *block = static_cast<DeclarationNode *>(member);
            if (block->getObjectType() == "constant") {
            }
        }
    }

    moduleNode =
        std::static_pointer_cast<DeclarationNode>(tree->getChildren().at(2));
    EXPECT_TRUE(moduleNode->getName() == "BlocksTest");
    blockList =
        static_cast<ListNode *>(moduleNode->getPropertyValue("blocks").get());
    EXPECT_TRUE(blockList->getNodeType() == AST::List);
    QStringList blockNames;
    blockNames << "Test";
    blockNames << "Input";
    blockNames << "Output";
    blockNames << "AutoDeclared";
    for (auto name : blockNames) {
        auto decl = ASTQuery::findDeclarationByName(
            name.toStdString(), {{moduleNode, blockList->getChildren()}},
nullptr); EXPECT_TRUE(decl);
    }
    // Check to make sure input and output domains have propagated correctly
    for (auto blockNode : blockList->getChildren()) {
        DeclarationNode *block = static_cast<DeclarationNode
*>(blockNode.get()); EXPECT_TRUE(block->getDomain());
        EXPECT_TRUE(block->getDomain()->getNodeType() == AST::PortProperty);
        auto domain = static_cast<PortPropertyNode *>(block->getDomain().get());
        if (block->getName() == "Input") {
            EXPECT_TRUE(domain->getPortName() == "domain");
            EXPECT_TRUE(domain->getName() == "OutputPort");
        } else {
            EXPECT_TRUE(domain->getPortName() == "domain");
            EXPECT_TRUE(domain->getName() == "OutputPort");
        }
    }
}*/

TEST(Basic, BundleIndeces) {
  ASTNode tree;
  tree = ASTFunctions::parseFile(TESTS_SOURCE_DIR
                                 "basic/07_bundle_indeces.stride");
  EXPECT_NE(tree, nullptr);
  std::vector<ASTNode> nodes = tree->getChildren();
  EXPECT_TRUE(nodes.size() == 23);

  EXPECT_TRUE(nodes.at(0)->getNodeType() == AST::Declaration);
  DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(0).get());
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_EQ(block->getLine(), 8);

  EXPECT_TRUE(nodes.at(1)->getNodeType() == AST::BundleDeclaration);
  block = static_cast<DeclarationNode *>(nodes.at(1).get());
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 13);
  BundleNode *bundle = block->getBundle().get();
  EXPECT_TRUE(bundle->getName() == "SIZE");
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  EXPECT_TRUE(bundle->getLine() == 13);
  ListNode *indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  ValueNode *value =
      static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 4);
  EXPECT_TRUE(value->getLine() == 13);
  EXPECT_TRUE(block->getProperties().size() == 2);
  EXPECT_TRUE(block->getProperties().at(0)->getNodeType() == AST::Property);
  PropertyNode *property =
      static_cast<PropertyNode *>(block->getProperties().at(0).get());
  ListNode *listnode = static_cast<ListNode *>(property->getValue().get());
  EXPECT_TRUE(listnode->getNodeType() == AST::List);
  EXPECT_TRUE(listnode->getChildren().size() == 4);
  EXPECT_TRUE(listnode->getLine() == 14);
  value = static_cast<ValueNode *>(listnode->getChildren().at(0).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 16);
  EXPECT_TRUE(value->getLine() == 14);
  value = static_cast<ValueNode *>(listnode->getChildren().at(1).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 32);
  EXPECT_TRUE(value->getLine() == 14);
  value = static_cast<ValueNode *>(listnode->getChildren().at(2).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 64);
  EXPECT_TRUE(value->getLine() == 14);
  value = static_cast<ValueNode *>(listnode->getChildren().at(3).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 128);
  EXPECT_TRUE(value->getLine() == 14);

  // constant Array_Parens [ ( CONST * 2 ) + 1 ] {}
  block = static_cast<DeclarationNode *>(nodes.at(6).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 23);
  bundle = block->getBundle().get();
  EXPECT_TRUE(bundle->getName() == "Array_Parens");
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  ExpressionNode *expr =
      static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(expr->getNodeType() == AST::Expression);
  EXPECT_TRUE(expr->getExpressionType() == ExpressionNode::Add);
  EXPECT_TRUE(expr->getLine() == 23);
  ExpressionNode *expr2 = static_cast<ExpressionNode *>(expr->getLeft().get());
  EXPECT_TRUE(expr2->getExpressionType() == ExpressionNode::Multiply);
  EXPECT_TRUE(expr2->getLeft()->getNodeType() == AST::Block);
  EXPECT_TRUE(expr2->getRight()->getNodeType() == AST::Int);
  EXPECT_TRUE(expr2->getLine() == 23);

  // constant Array_Expr [ SIZE [1] + SIZE [1 * 2] ] {}
  block = static_cast<DeclarationNode *>(nodes.at(8).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  EXPECT_TRUE(block->getLine() == 26);
  EXPECT_TRUE(block->getObjectType() == "constant");
  bundle = block->getBundle().get();
  EXPECT_TRUE(bundle->getName() == "Array_Expr");
  EXPECT_TRUE(bundle->getLine() == 26);
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(expr->getNodeType() == AST::Expression);
  EXPECT_TRUE(expr->getExpressionType() == ExpressionNode::Add);
  EXPECT_TRUE(expr->getLine() == 26);
  bundle = static_cast<BundleNode *>(expr->getLeft().get());
  EXPECT_TRUE(bundle->getNodeType() == AST::Bundle);
  EXPECT_TRUE(bundle->getName() == "SIZE");
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 1);
  bundle = static_cast<BundleNode *>(expr->getRight().get());
  EXPECT_TRUE(bundle->getNodeType() == AST::Bundle);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(expr->getNodeType() == AST::Expression);
  EXPECT_TRUE(expr->getExpressionType() == ExpressionNode::Multiply);
  EXPECT_TRUE(expr->getLeft()->getNodeType() == AST::Int);
  EXPECT_TRUE(expr->getRight()->getNodeType() == AST::Int);
  EXPECT_TRUE(expr->getLine() == 26);

  // constant Array_Expr2 [ SIZE [1] / SIZE [1 - 2] ] {}
  block = static_cast<DeclarationNode *>(nodes.at(9).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 27);
  bundle = block->getBundle().get();
  EXPECT_TRUE(bundle->getName() == "Array_Expr2");
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  EXPECT_TRUE(bundle->getLine() == 27);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(expr->getNodeType() == AST::Expression);
  EXPECT_TRUE(expr->getExpressionType() == ExpressionNode::Divide);
  EXPECT_TRUE(expr->getLine() == 27);
  bundle = static_cast<BundleNode *>(expr->getLeft().get());
  EXPECT_TRUE(bundle->getNodeType() == AST::Bundle);
  EXPECT_TRUE(bundle->getName() == "SIZE");
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  EXPECT_TRUE(bundle->getLine() == 27);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 1);
  EXPECT_TRUE(value->getLine() == 27);
  bundle = static_cast<BundleNode *>(expr->getRight().get());
  EXPECT_TRUE(bundle->getNodeType() == AST::Bundle);
  EXPECT_TRUE(bundle->getLine() == 27);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  expr = static_cast<ExpressionNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(expr->getNodeType() == AST::Expression);
  EXPECT_TRUE(expr->getExpressionType() == ExpressionNode::Subtract);
  EXPECT_TRUE(expr->getLeft()->getNodeType() == AST::Int);
  EXPECT_TRUE(expr->getRight()->getNodeType() == AST::Int);
  EXPECT_TRUE(expr->getLine() == 27);
}

TEST(Basic, Lists) {
  ASTNode tree;
  tree = ASTFunctions::parseFile(TESTS_SOURCE_DIR "basic/09_lists.stride");
  EXPECT_TRUE(tree != nullptr);
  std::vector<ASTNode> nodes = tree->getChildren();

  //    constant List_Integer [4] {
  //            value: [ 16, 32, 64, 128 ]
  //    }
  DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(1).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  std::vector<std::shared_ptr<PropertyNode>> props = block->getProperties();
  ListNode *list = static_cast<ListNode *>(props.at(0)->getValue().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  std::vector<ASTNode> members = list->getChildren();
  EXPECT_TRUE(members.size() == 4);
  for (ASTNode member : members) {
    ValueNode *value = static_cast<ValueNode *>(member.get());
    EXPECT_TRUE(value->getNodeType() == AST::Int);
  }
  //    constant List_Real [4] {
  //            value: [ 16., 32.1, 64., 128. ]
  //    }
  block = static_cast<DeclarationNode *>(nodes.at(2).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  props = block->getProperties();
  list = static_cast<ListNode *>(props.at(0)->getValue().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  members = list->getChildren();
  EXPECT_TRUE(members.size() == 4);
  for (ASTNode member : members) {
    ValueNode *value = static_cast<ValueNode *>(member.get());
    EXPECT_TRUE(value->getNodeType() == AST::Real);
  }

  //    constant List_Strings [4] {
  //            value: [ '16', "32.1", '64', "128" ]
  //    }
  block = static_cast<DeclarationNode *>(nodes.at(3).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  props = block->getProperties();
  list = static_cast<ListNode *>(props.at(0)->getValue().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  members = list->getChildren();
  EXPECT_TRUE(members.size() == 4);
  for (ASTNode member : members) {
    ValueNode *value = static_cast<ValueNode *>(member.get());
    EXPECT_TRUE(value->getNodeType() == AST::String);
  }

  //    constant List_Switches [4] {
  //            value: [ on, off, on, on ]
  //    }
  block = static_cast<DeclarationNode *>(nodes.at(4).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  props = block->getProperties();
  list = static_cast<ListNode *>(props.at(0)->getValue().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  members = list->getChildren();
  EXPECT_TRUE(members.size() == 4);
  for (ASTNode &member : members) {
    ValueNode *value = static_cast<ValueNode *>(member.get());
    EXPECT_TRUE(value->getNodeType() == AST::Switch);
  }

  //    constant List_Names [4] {
  //            value: [ Name1, Name2, Name3, Name4 ]
  //    }
  block = static_cast<DeclarationNode *>(nodes.at(5).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  props = block->getProperties();
  list = static_cast<ListNode *>(props.at(0)->getValue().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  members = list->getChildren();
  EXPECT_TRUE(members.size() == 4);
  for (ASTNode &member : members) {
    BlockNode *value = static_cast<BlockNode *>(member.get());
    EXPECT_TRUE(value->getNodeType() == AST::Bundle);
  }

  //    constant List_Namespaces [4] {
  //            value: [ ns.Name1, ns.Name2, ns.Name3, ns.Name4 ]
  //    }
  block = static_cast<DeclarationNode *>(nodes.at(6).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  props = block->getProperties();
  list = static_cast<ListNode *>(props.at(0)->getValue().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  members = list->getChildren();
  EXPECT_TRUE(members.size() == 4);
  for (ASTNode &member : members) {
    BlockNode *value = static_cast<BlockNode *>(member.get());
    EXPECT_TRUE(value->getNodeType() == AST::Block);
    EXPECT_TRUE(value->getScopeLevels() == 1);
    EXPECT_TRUE(value->getScopeAt(0) == "Ns");
  }

  //    block BlockName {
  //    property: [ blockType1 BlockName2 { property: "value" },
  //                blockType1 BlockName3 { value: 1.0 } ]
  //    }
  block = static_cast<DeclarationNode *>(nodes.at(7).get());
  EXPECT_TRUE(block->getNodeType() == AST::Declaration);
  props = block->getProperties();
  list = static_cast<ListNode *>(props.at(0)->getValue().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  members = list->getChildren();
  EXPECT_TRUE(members.size() == 2);
  DeclarationNode *internalBlock =
      static_cast<DeclarationNode *>(members[0].get());
  EXPECT_TRUE(internalBlock->getNodeType() == AST::Declaration);
  EXPECT_TRUE(internalBlock->getObjectType() == "blockType1");
  EXPECT_TRUE(internalBlock->getName() == "BlockName2");

  internalBlock = static_cast<DeclarationNode *>(members[1].get());
  EXPECT_TRUE(internalBlock->getNodeType() == AST::Declaration);
  EXPECT_TRUE(internalBlock->getObjectType() == "blockType2");
  EXPECT_TRUE(internalBlock->getName() == "BlockName3");

  //    constant IntegerList [3] {
  //            value: [[ 9, 8, 7 ] , [ 6, 5, 4 ] , [ 3, 2, 1 ] ]
  //            meta:	'List of lists'
  //    }
  block = static_cast<DeclarationNode *>(nodes.at(8).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  props = block->getProperties();
  list = static_cast<ListNode *>(props.at(0)->getValue().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  members = list->getChildren();
  EXPECT_TRUE(members.size() == 3);
  EXPECT_TRUE(members.at(0)->getNodeType() == AST::List);
  EXPECT_TRUE(members.at(1)->getNodeType() == AST::List);
  EXPECT_TRUE(members.at(2)->getNodeType() == AST::List);

  //    [ In >> Out; OtherIn >> OtherOut;] >> [Out1, Out2];
  //    [ In >> Out; OtherIn >> OtherOut;] >> Out;
  //    Out >> [ In >> Out; OtherIn >> OtherOut;];

  StreamNode *stream = static_cast<StreamNode *>(nodes.at(9).get());
  EXPECT_TRUE(stream->getNodeType() == AST::Stream);
  list = static_cast<ListNode *>(stream->getLeft().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  EXPECT_TRUE(list->getChildren().size() == 2);
  EXPECT_TRUE(list->getChildren()[0]->getNodeType() == AST::Stream);
  EXPECT_TRUE(list->getChildren()[1]->getNodeType() == AST::Stream);
  //  stream = static_cast<StreamNode *>(nodes.at(10).get());
  //  list = static_cast<ListNode *>(stream->getLeft().get());
  //  EXPECT_TRUE(list->getNodeType() == AST::List);
  //  EXPECT_TRUE(list->getChildren().size() == 2);
  //  EXPECT_TRUE(list->getChildren()[0]->getNodeType() == AST::Stream);
  //  EXPECT_TRUE(list->getChildren()[1]->getNodeType() == AST::Stream);
  stream = static_cast<StreamNode *>(nodes.at(10).get());
  list = static_cast<ListNode *>(stream->getRight().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  EXPECT_TRUE(list->getChildren().size() == 2);
  EXPECT_TRUE(list->getChildren()[0]->getNodeType() == AST::Stream);
  EXPECT_TRUE(list->getChildren()[1]->getNodeType() == AST::Stream);
}

TEST(Basic, NoneSwitch) {
  ASTNode tree;
  tree = ASTFunctions::parseFile(TESTS_SOURCE_DIR
                                 "basic/06_basic_noneswitch.stride");
  EXPECT_TRUE(tree != nullptr);
  std::vector<ASTNode> nodes = tree->getChildren();
  EXPECT_TRUE(nodes.size() == 2);

  EXPECT_TRUE(nodes.at(0)->getNodeType() == AST::Declaration);
  DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(0).get());
  EXPECT_TRUE(block->getObjectType() == "object");
  std::vector<std::shared_ptr<PropertyNode>> properties =
      block->getProperties();
  EXPECT_TRUE(properties.size() == 3);
  EXPECT_TRUE(properties.at(0)->getName() == "prop1");
  ValueNode *value =
      static_cast<ValueNode *>(properties.at(0)->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Switch);
  EXPECT_TRUE(value->getSwitchValue() == true);
  EXPECT_TRUE(properties.at(1)->getName() == "prop2");
  value = static_cast<ValueNode *>(properties.at(1)->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Switch);
  EXPECT_TRUE(value->getSwitchValue() == false);
  EXPECT_TRUE(properties.at(2)->getName() == "prop3");
  value = static_cast<ValueNode *>(properties.at(2)->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::None);

  EXPECT_TRUE(nodes.at(1)->getNodeType() == AST::Stream);
  StreamNode *stream = static_cast<StreamNode *>(nodes.at(1).get());
  FunctionNode *func = static_cast<FunctionNode *>(stream->getLeft().get());
  EXPECT_TRUE(func->getNodeType() == AST::Function);
  std::vector<std::shared_ptr<PropertyNode>> funcProperties =
      func->getProperties();
  EXPECT_TRUE(funcProperties.size() == 3);
  EXPECT_TRUE(funcProperties.at(0)->getName() == "propf1");
  value = static_cast<ValueNode *>(funcProperties.at(0)->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Switch);
  EXPECT_TRUE(value->getSwitchValue() == true);
  EXPECT_TRUE(funcProperties.at(1)->getName() == "propf2");
  value = static_cast<ValueNode *>(funcProperties.at(1)->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Switch);
  EXPECT_TRUE(value->getSwitchValue() == false);
  EXPECT_TRUE(funcProperties.at(2)->getName() == "propf3");
  value = static_cast<ValueNode *>(funcProperties.at(2)->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::None);
}

TEST(Basic, Functions) {
  ASTNode tree;
  tree = ASTFunctions::parseFile(TESTS_SOURCE_DIR
                                 "basic/05_basic_functions.stride");
  EXPECT_TRUE(tree != nullptr);
  std::vector<ASTNode> nodes = tree->getChildren();

  EXPECT_TRUE(nodes.at(0)->getNodeType() == AST::Stream);
  StreamNode *node = static_cast<StreamNode *>(nodes.at(0).get());
  FunctionNode *function = static_cast<FunctionNode *>(node->getLeft().get());
  EXPECT_TRUE(function->getNodeType() == AST::Function);
  EXPECT_TRUE(function->getName() == "Function1");
  std::vector<std::shared_ptr<PropertyNode>> properties =
      function->getProperties();
  PropertyNode *property = properties.at(0).get();
  EXPECT_TRUE(property->getName() == "propReal");
  ValueNode *value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Real);
  EXPECT_TRUE(value->getRealValue() == 1.1);
  property = properties.at(1).get();
  EXPECT_TRUE(property->getName() == "propInt");
  value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 23);
  property = properties.at(2).get();
  EXPECT_TRUE(property->getName() == "propString");
  value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::String);
  EXPECT_TRUE(value->getStringValue() == "hello");

  node = static_cast<StreamNode *>(node->getRight().get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);

  function = static_cast<FunctionNode *>(node->getLeft().get());
  EXPECT_TRUE(function->getNodeType() == AST::Function);
  EXPECT_TRUE(function->getName() == "Function2");
  properties = function->getProperties();
  property = properties.at(0).get();
  EXPECT_TRUE(property->getName() == "propReal");
  value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Real);
  EXPECT_TRUE(value->getRealValue() == 1.2);
  property = properties.at(1).get();
  EXPECT_TRUE(property->getName() == "propInt");
  value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 123);
  property = properties.at(2).get();
  EXPECT_TRUE(property->getName() == "propString");
  value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::String);
  EXPECT_TRUE(value->getStringValue() == "function");

  function = static_cast<FunctionNode *>(node->getRight().get());
  EXPECT_TRUE(function->getNodeType() == AST::Function);
  EXPECT_TRUE(function->getName() == "Function3");
  properties = function->getProperties();
  property = properties.at(0).get();
  EXPECT_TRUE(property->getName() == "propReal");
  value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Real);
  EXPECT_TRUE(value->getRealValue() == 1.3);
  property = properties.at(1).get();
  EXPECT_TRUE(property->getName() == "propInt");
  value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 1123);
  property = properties.at(2).get();
  EXPECT_TRUE(property->getName() == "propString");
  value = static_cast<ValueNode *>(property->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::String);
  EXPECT_TRUE(value->getStringValue() == "lines");
}

TEST(Basic, Stream) {
  ASTNode tree;
  tree =
      ASTFunctions::parseFile(TESTS_SOURCE_DIR "basic/04_basic_stream.stride");
  EXPECT_TRUE(tree != nullptr);
  std::vector<ASTNode> nodes = tree->getChildren();

  // Val1 >> Val2 ;
  StreamNode *node = static_cast<StreamNode *>(nodes.at(1).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_EQ(node->getLine(), 3);
  std::vector<ASTNode> streamParts = node->getChildren();
  EXPECT_TRUE(streamParts.size() == 2);
  AST *streamComp = streamParts.at(0).get();
  EXPECT_TRUE(streamComp->getNodeType() == AST::Block);
  EXPECT_EQ(streamComp->getLine(), 3);
  BlockNode *nameNode = static_cast<BlockNode *>(streamComp);
  EXPECT_TRUE(nameNode->getName() == "Val1");
  EXPECT_TRUE(nameNode->getLine() == 3);
  streamComp = streamParts.at(1).get();
  EXPECT_TRUE(streamComp->getNodeType() == AST::Block);
  EXPECT_TRUE(streamComp->getLine() == 3);
  nameNode = static_cast<BlockNode *>(streamComp);
  EXPECT_TRUE(nameNode->getName() == "Val2");
  EXPECT_TRUE(nameNode->getLine() == 3);

  // Func1() >> Func2() ;
  node = static_cast<StreamNode *>(nodes.at(2).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLine() == 4);
  streamParts = node->getChildren();
  EXPECT_TRUE(streamParts.size() == 2);
  streamComp = streamParts.at(0).get();
  EXPECT_TRUE(streamComp->getLine() == 4);
  EXPECT_TRUE(streamComp->getNodeType() == AST::Function);
  FunctionNode *functionNode = static_cast<FunctionNode *>(streamComp);
  EXPECT_TRUE(functionNode->getName() == "Func1");
  EXPECT_TRUE(functionNode->getLine() == 4);
  streamComp = streamParts.at(1).get();
  EXPECT_TRUE(streamComp->getNodeType() == AST::Function);
  EXPECT_TRUE(streamComp->getLine() == 4);
  functionNode = static_cast<FunctionNode *>(streamComp);
  EXPECT_TRUE(functionNode->getName() == "Func2");
  EXPECT_TRUE(functionNode->getLine() == 4);

  // Val1 >> Func1() >> Func2() >> Val2 ;
  node = static_cast<StreamNode *>(nodes.at(3).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLine() == 6);
  streamComp = node->getLeft().get();
  EXPECT_TRUE(streamComp->getNodeType() == AST::Block);
  EXPECT_TRUE(streamComp->getLine() == 6);
  nameNode = static_cast<BlockNode *>(streamComp);
  EXPECT_TRUE(nameNode->getName() == "Val1");
  EXPECT_TRUE(nameNode->getLine() == 6);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Stream);
  node = static_cast<StreamNode *>(node->getRight().get());
  streamComp = node->getLeft().get();
  EXPECT_TRUE(streamComp->getLine() == 6);
  EXPECT_TRUE(streamComp->getNodeType() == AST::Function);
  functionNode = static_cast<FunctionNode *>(streamComp);
  EXPECT_TRUE(functionNode->getName() == "Func1");
  EXPECT_TRUE(functionNode->getLine() == 6);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Stream);
  node = static_cast<StreamNode *>(node->getRight().get());
  EXPECT_TRUE(node->getLine() == 7);
  streamComp = node->getLeft().get();
  EXPECT_TRUE(streamComp->getNodeType() == AST::Function);
  EXPECT_TRUE(streamComp->getLine() == 7);
  functionNode = static_cast<FunctionNode *>(streamComp);
  EXPECT_TRUE(functionNode->getName() == "Func2");
  EXPECT_TRUE(functionNode->getLine() == 7);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Block);
  nameNode = static_cast<BlockNode *>(node->getRight().get());
  EXPECT_TRUE(nameNode->getName() == "Val2");
  EXPECT_TRUE(nameNode->getLine() == 8);

  //    Bundle1[1] >> Bundle2[2];
  node = static_cast<StreamNode *>(nodes.at(4).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLine() == 10);
  EXPECT_TRUE(node->getLeft()->getNodeType() == AST::Bundle);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Bundle);
  BundleNode *bundle = static_cast<BundleNode *>(node->getLeft().get());
  EXPECT_TRUE(bundle->getName() == "Bundle1");
  EXPECT_TRUE(bundle->getLine() == 10);
  ListNode *indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  ValueNode *value =
      static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getLine() == 10);
  EXPECT_TRUE(value->getIntValue() == 1);
  bundle = static_cast<BundleNode *>(node->getRight().get());
  EXPECT_TRUE(bundle->getName() == "Bundle2");
  EXPECT_TRUE(bundle->getLine() == 10);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getLine() == 10);
  EXPECT_TRUE(value->getIntValue() == 2);

  //    Val1 * 3 >> Bundle[2];
  node = static_cast<StreamNode *>(nodes.at(5).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLeft()->getNodeType() == AST::Expression);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Bundle);
  EXPECT_TRUE(node->getLine() == 11);
  ExpressionNode *expression =
      static_cast<ExpressionNode *>(node->getLeft().get());
  EXPECT_TRUE(expression->getExpressionType() == ExpressionNode::Multiply);
  EXPECT_TRUE(expression->getLeft()->getNodeType() == AST::Block);
  EXPECT_TRUE(expression->getLine() == 11);
  nameNode = static_cast<BlockNode *>(expression->getLeft().get());
  EXPECT_TRUE(nameNode->getName() == "Val1");
  EXPECT_TRUE(nameNode->getLine() == 11);
  EXPECT_TRUE(expression->getRight()->getNodeType() == AST::Int);
  EXPECT_TRUE(
      static_cast<ValueNode *>(expression->getRight().get())->getIntValue() ==
      3);
  bundle = static_cast<BundleNode *>(node->getRight().get());
  EXPECT_TRUE(bundle->getName() == "Bundle");
  EXPECT_TRUE(bundle->getLine() == 11);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getLine() == 11);
  EXPECT_TRUE(value->getIntValue() == 2);

  //    Bundle1[1] * 0.5 >> Bundle2[2];
  node = static_cast<StreamNode *>(nodes.at(6).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLeft()->getNodeType() == AST::Expression);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Bundle);
  EXPECT_TRUE(node->getLine() == 12);
  expression = static_cast<ExpressionNode *>(node->getLeft().get());
  EXPECT_TRUE(expression->getExpressionType() == ExpressionNode::Multiply);
  EXPECT_TRUE(expression->getLeft()->getNodeType() == AST::Bundle);
  EXPECT_TRUE(expression->getLine() == 12);
  bundle = static_cast<BundleNode *>(expression->getLeft().get());
  EXPECT_TRUE(bundle->getName() == "Bundle1");
  EXPECT_TRUE(bundle->getLine() == 12);
  EXPECT_TRUE(expression->getRight()->getNodeType() == AST::Real);
  EXPECT_TRUE(
      static_cast<ValueNode *>(expression->getRight().get())->getRealValue() ==
      0.5);
  bundle = static_cast<BundleNode *>(node->getRight().get());
  EXPECT_TRUE(bundle->getName() == "Bundle2");
  EXPECT_TRUE(bundle->getLine() == 12);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  value = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getLine() == 12);
  EXPECT_TRUE(value->getIntValue() == 2);

  //    BundleRange[1:2] >> BundleRange2[3:4];
  node = static_cast<StreamNode *>(nodes.at(7).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLeft()->getNodeType() == AST::Bundle);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Bundle);
  EXPECT_TRUE(node->getLine() == 14);
  bundle = static_cast<BundleNode *>(node->getLeft().get());
  EXPECT_TRUE(bundle->getName() == "BundleRange");
  EXPECT_EQ(bundle->getLine(), 14);
  ListNode *index = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(index->getNodeType() == AST::List);
  EXPECT_TRUE(index->getChildren().size() == 1);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  RangeNode *range =
      static_cast<RangeNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(range->getNodeType() == AST::Range);
  EXPECT_TRUE(range->getLine() == 14);
  value = static_cast<ValueNode *>(range->startIndex().get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 1);
  value = static_cast<ValueNode *>(range->endIndex().get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getLine() == 14);
  EXPECT_TRUE(value->getIntValue() == 2);

  bundle = static_cast<BundleNode *>(node->getRight().get());
  EXPECT_TRUE(bundle->getName() == "BundleRange2");
  EXPECT_TRUE(bundle->getLine() == 14);
  index = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(index->getNodeType() == AST::List);
  EXPECT_TRUE(index->getChildren().size() == 1);
  indexList = bundle->index().get();
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  range = static_cast<RangeNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(range->getNodeType() == AST::Range);
  EXPECT_TRUE(range->getLine() == 14);
  value = static_cast<ValueNode *>(range->startIndex().get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getIntValue() == 3);
  value = static_cast<ValueNode *>(range->endIndex().get());
  EXPECT_TRUE(value->getNodeType() == AST::Int);
  EXPECT_TRUE(value->getLine() == 14);
  EXPECT_TRUE(value->getIntValue() == 4);

  //    AudioIn[1] >> level(gain: 1.5) >> AudioOut[1];
  node = static_cast<StreamNode *>(nodes.at(8).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLeft()->getNodeType() == AST::Bundle);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLine() == 16);
  node = static_cast<StreamNode *>(node->getRight().get());
  EXPECT_TRUE(node->getLeft()->getNodeType() == AST::Function);
  functionNode = static_cast<FunctionNode *>(node->getLeft().get());
  EXPECT_TRUE(functionNode->getName() == "Level");
  std::vector<std::shared_ptr<PropertyNode>> properties =
      functionNode->getProperties();
  EXPECT_TRUE(properties.size() == 1);
  PropertyNode *prop = properties[0].get();
  EXPECT_TRUE(prop->getName() == "gain");
  value = static_cast<ValueNode *>(prop->getValue().get());
  EXPECT_TRUE(value->getNodeType() == AST::Real);
  EXPECT_TRUE(value->getRealValue() == 1.5);

  //    A[1:2,3,4] >> B[1,2,3:4] >> C[1,2:3,4] >> D[1,2,3,4];
  node = static_cast<StreamNode *>(nodes.at(9).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLeft()->getNodeType() == AST::Bundle);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::Stream);
  bundle = static_cast<BundleNode *>(node->getLeft().get());
  EXPECT_TRUE(bundle->getName() == "A");
  EXPECT_TRUE(bundle->getLine() == 18);
  ListNode *list = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(list->getNodeType() == AST::List);
  EXPECT_TRUE(list->getLine() == 18);

  node = static_cast<StreamNode *>(nodes.at(10).get());
  EXPECT_TRUE(node->getNodeType() == AST::Stream);
  EXPECT_TRUE(node->getLeft()->getNodeType() == AST::List);
  EXPECT_TRUE(node->getRight()->getNodeType() == AST::List);
  ListNode *l = static_cast<ListNode *>(node->getLeft().get());
  EXPECT_TRUE(l->size() == 2);
  std::vector<ASTNode> elements = l->getChildren();
  EXPECT_TRUE(elements.size() == 2);
  EXPECT_TRUE(elements.at(0)->getNodeType() == AST::Bundle);
  EXPECT_TRUE(elements.at(1)->getNodeType() == AST::Bundle);
  l = static_cast<ListNode *>(node->getRight().get());
  EXPECT_TRUE(l->size() == 2);
  elements = l->getChildren();
  EXPECT_TRUE(elements.size() == 2);
  EXPECT_TRUE(elements.at(0)->getNodeType() == AST::Bundle);
  EXPECT_TRUE(elements.at(1)->getNodeType() == AST::Bundle);

  EXPECT_TRUE(l->getLine() == 20);
}

TEST(Basic, Bundle) {
  ASTNode tree;
  tree =
      ASTFunctions::parseFile(TESTS_SOURCE_DIR "basic/03_basic_bundle.stride");
  EXPECT_TRUE(tree != nullptr);
  std::vector<ASTNode> nodes = tree->getChildren();
  EXPECT_TRUE(nodes.size() == 8);
  DeclarationNode *block = static_cast<DeclarationNode *>(nodes.at(1).get());
  EXPECT_TRUE(block->getNodeType() == AST::BundleDeclaration);
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 5);
  std::shared_ptr<BundleNode> bundle = block->getBundle();
  EXPECT_TRUE(bundle->getName() == "Integer");
  EXPECT_TRUE(bundle->getLine() == 5);
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  ListNode *indexList = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  ValueNode *valueNode =
      static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  EXPECT_TRUE(valueNode->getIntValue() == 3);
  EXPECT_TRUE(bundle->getLine() == 5);
  EXPECT_TRUE(block->getProperties().size() == 2);
  std::shared_ptr<PropertyNode> property = block->getProperties().at(0);
  EXPECT_TRUE(property->getName() == "value");
  AST *propertyValue = property->getValue().get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  ListNode *listnode = static_cast<ListNode *>(propertyValue);
  //    EXPECT_TRUE(listnode->getListType() == AST::Int);
  std::vector<ASTNode> listValues = listnode->getChildren();
  EXPECT_TRUE(listValues.size() == 3);
  EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::Int);
  EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::Expression);
  EXPECT_TRUE(listValues.at(2)->getNodeType() == AST::Int);
  property = block->getProperties().at(1);
  EXPECT_TRUE(property->getName() == "meta");

  // Next Block - Float list
  block = static_cast<DeclarationNode *>(nodes.at(2).get());
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 10);
  bundle = block->getBundle();
  EXPECT_TRUE(bundle->getName() == "Float");
  EXPECT_TRUE(bundle->getLine() == 10);
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  EXPECT_TRUE(valueNode->getIntValue() == 4);
  EXPECT_TRUE(block->getProperties().size() == 2);
  property = block->getProperties().at(0);
  EXPECT_TRUE(property->getName() == "value");
  propertyValue = property->getValue().get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  listnode = static_cast<ListNode *>(propertyValue);
  //    EXPECT_TRUE(listnode->getListType() == AST::Real);
  listValues = listnode->getChildren();
  EXPECT_TRUE(listValues.size() == 4);
  EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::Real);
  EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::Expression);
  EXPECT_TRUE(listValues.at(2)->getNodeType() == AST::Real);
  EXPECT_TRUE(listValues.at(3)->getNodeType() == AST::Real);
  EXPECT_TRUE(block->getChildren().at(1)->getNodeType() == AST::Property);
  property =
      std::static_pointer_cast<PropertyNode>(block->getProperties().at(1));
  EXPECT_TRUE(property->getName() == "meta");

  // Next Block - String list
  block = static_cast<DeclarationNode *>(nodes.at(3).get());
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 15);
  bundle = block->getBundle();
  EXPECT_TRUE(bundle->getName() == "String");
  EXPECT_TRUE(bundle->getLine() == 15);
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  EXPECT_TRUE(valueNode->getIntValue() == 5);
  EXPECT_TRUE(block->getProperties().size() == 2);
  property = block->getProperties().at(0);
  EXPECT_TRUE(property->getName() == "value");
  propertyValue = property->getValue().get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  listnode = static_cast<ListNode *>(propertyValue);
  EXPECT_TRUE(listnode->getListType() == AST::String);
  listValues = listnode->getChildren();
  EXPECT_TRUE(listValues.size() == 5);
  EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::String);
  EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::String);
  EXPECT_TRUE(listValues.at(2)->getNodeType() == AST::String);
  EXPECT_TRUE(listValues.at(3)->getNodeType() == AST::String);
  EXPECT_TRUE(listValues.at(4)->getNodeType() == AST::String);
  property = block->getProperties().at(1);
  EXPECT_TRUE(property->getName() == "meta");

  // Next Block - UVar list
  block = static_cast<DeclarationNode *>(nodes.at(4).get());
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 21);
  bundle = block->getBundle();
  EXPECT_TRUE(bundle->getName() == "UVar");
  EXPECT_TRUE(bundle->getLine() == 21);
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  EXPECT_TRUE(valueNode->getIntValue() == 7);
  EXPECT_TRUE(block->getProperties().size() == 2);
  property = block->getProperties().at(0);
  EXPECT_TRUE(property->getName() == "value");
  propertyValue = property->getValue().get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  listValues = propertyValue->getChildren();
  EXPECT_TRUE(listValues.size() == 7);
  EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::Block);
  EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::Block);
  EXPECT_TRUE(listValues.at(2)->getNodeType() == AST::Block);
  EXPECT_TRUE(listValues.at(3)->getNodeType() == AST::Block);
  EXPECT_TRUE(listValues.at(4)->getNodeType() == AST::Block);
  EXPECT_TRUE(listValues.at(5)->getNodeType() == AST::Block);
  EXPECT_TRUE(listValues.at(6)->getNodeType() == AST::Block);
  property = block->getProperties().at(1);
  EXPECT_TRUE(property->getName() == "meta");

  // Next Block - ArrayList list
  block = static_cast<DeclarationNode *>(nodes.at(5).get());
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 26);
  bundle = block->getBundle();
  EXPECT_TRUE(bundle->getName() == "ArrayList");
  EXPECT_TRUE(bundle->getLine() == 26);
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  EXPECT_TRUE(valueNode->getIntValue() == 2);
  EXPECT_TRUE(block->getProperties().size() == 2);
  property = block->getProperties().at(0);
  EXPECT_TRUE(property->getName() == "value");
  propertyValue = property->getValue().get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  listValues = propertyValue->getChildren();
  EXPECT_TRUE(listValues.size() == 2);
  EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::Bundle);
  EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::Bundle);
  property = block->getProperties().at(1);
  EXPECT_TRUE(property->getName() == "meta");

  // Next Block - BlockList list
  block = static_cast<DeclarationNode *>(nodes.at(6).get());
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 31);
  bundle = block->getBundle();
  EXPECT_TRUE(bundle->getName() == "BlockList");
  EXPECT_TRUE(bundle->getLine() == 31);
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  EXPECT_TRUE(valueNode->getIntValue() == 3);
  EXPECT_TRUE(block->getProperties().size() == 2);
  property = block->getProperties().at(0);
  EXPECT_TRUE(property->getName() == "value");
  propertyValue = property->getValue().get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  listValues = propertyValue->getChildren();
  EXPECT_TRUE(listValues.size() == 3);
  EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::Declaration);
  EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::Declaration);
  property =
      std::static_pointer_cast<PropertyNode>(block->getProperties().at(1));
  EXPECT_TRUE(property->getName() == "meta");

  // Next Block - BlockBundleList list
  block = static_cast<DeclarationNode *>(nodes.at(7).get());
  EXPECT_TRUE(block->getObjectType() == "constant");
  EXPECT_TRUE(block->getLine() == 36);
  bundle = block->getBundle();
  EXPECT_TRUE(bundle->getName() == "BlockBundleList");
  EXPECT_TRUE(bundle->getLine() == 36);
  EXPECT_TRUE(bundle->getChildren().size() == 1);
  indexList = static_cast<ListNode *>(bundle->index().get());
  EXPECT_TRUE(indexList->getNodeType() == AST::List);
  EXPECT_TRUE(indexList->size() == 1);
  valueNode = static_cast<ValueNode *>(indexList->getChildren().at(0).get());
  EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  EXPECT_TRUE(valueNode->getIntValue() == 3);
  EXPECT_TRUE(block->getProperties().size() == 2);
  property = block->getProperties().at(0);
  EXPECT_TRUE(property->getName() == "value");
  propertyValue = property->getValue().get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  listValues = propertyValue->getChildren();
  EXPECT_TRUE(listValues.size() == 3);
  EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::BundleDeclaration);
  EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::BundleDeclaration);
  EXPECT_TRUE(listValues.at(2)->getNodeType() == AST::BundleDeclaration);
  property = block->getProperties().at(1);
  EXPECT_TRUE(property->getName() == "meta");

  //    // Next Block - IntegerList list
  //    block = static_cast<DeclarationNode *>(nodes.at(7));
  //    EXPECT_TRUE(block->getObjectType() == "constant");
  //    EXPECT_TRUE(block->getLine() == 46);
  //    bundle = block->getBundle();
  //    EXPECT_TRUE(bundle->getName() == "IntegerList");
  //    EXPECT_TRUE(bundle->getLine() == 46);
  //    EXPECT_TRUE(bundle->getChildren().size() == 1);
  //    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
  //    EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  //    EXPECT_TRUE(valueNode->getIntValue() == 3);
  //    EXPECT_TRUE(block->getProperties().size() == 2);
  //    property = block->getProperties().at(0);
  //    EXPECT_TRUE(property->getName() == "value");
  //    propertyValue = property->getValue();
  //    EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  //    listValues = propertyValue->getChildren();
  //    EXPECT_TRUE(listValues.size() == 3);
  //    EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::List);
  //    EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::List);
  //    EXPECT_TRUE(listValues.at(2)->getNodeType() == AST::List);
  //    property = block->getProperties().at(1);
  //    EXPECT_TRUE(property->getName() == "meta");

  //    // Next Block - IntegerList list
  //    block = static_cast<DeclarationNode *>(nodes.at(8));
  //    EXPECT_TRUE(block->getObjectType() == "constant");
  //    EXPECT_TRUE(block->getLine() == 52);
  //    bundle = block->getBundle();
  //    EXPECT_TRUE(bundle->getName() == "IntegerList");
  //    EXPECT_TRUE(bundle->getLine() == 53);
  //    EXPECT_TRUE(bundle->getChildren().size() == 1);
  //    valueNode = static_cast<ValueNode *>(bundle->getChildren().at(0));
  //    EXPECT_TRUE(valueNode->getNodeType() == AST::Int);
  //    EXPECT_TRUE(valueNode->getIntValue() == 3);
  //    EXPECT_TRUE(block->getProperties().size() == 2);
  //    property = block->getProperties().at(0);
  //    EXPECT_TRUE(property->getName() == "value");
  //    propertyValue = property->getValue();
  //    EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  //    listValues = propertyValue->getChildren();
  //    EXPECT_TRUE(listValues.size() == 3);
  //    EXPECT_TRUE(listValues.at(0)->getNodeType() == AST::List);
  //    EXPECT_TRUE(listValues.at(1)->getNodeType() == AST::List);
  //    EXPECT_TRUE(listValues.at(2)->getNodeType() == AST::List);
  //    property = block->getProperties().at(1);
  //    EXPECT_TRUE(property->getName() == "meta");
}

TEST(Basic, Blocks) {
  ASTNode tree;
  tree =
      ASTFunctions::parseFile(TESTS_SOURCE_DIR "basic/02_basic_blocks.stride");
  EXPECT_TRUE(tree != nullptr);
  std::vector<ASTNode> nodes = tree->getChildren();
  EXPECT_TRUE(nodes.size() == 5);
  AST *node = nodes.at(0).get();
  EXPECT_TRUE(node->getNodeType() == AST::Declaration);
  std::vector<std::shared_ptr<PropertyNode>> properties =
      static_cast<DeclarationNode *>(node)->getProperties();
  EXPECT_TRUE(properties.size() == 2);
  std::shared_ptr<PropertyNode> property = properties.at(0);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "rate");
  AST *propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::Block);
  EXPECT_TRUE(static_cast<BlockNode *>(propertyValue)->getName() ==
              "AudioRate");
  property = properties.at(1);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "meta");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::String);
  EXPECT_TRUE(static_cast<ValueNode *>(propertyValue)->getStringValue() ==
              "Guitar input.");

  node = nodes.at(1).get();
  EXPECT_TRUE(node->getNodeType() == AST::Declaration);
  properties = static_cast<DeclarationNode *>(node)->getProperties();
  EXPECT_TRUE(properties.size() == 2);
  property = properties.at(0);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "value");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::Int);
  EXPECT_TRUE(static_cast<ValueNode *>(propertyValue)->getIntValue() == 5);
  property = properties.at(1);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "meta");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::String);
  EXPECT_TRUE(static_cast<ValueNode *>(propertyValue)->getStringValue() ==
              "Integer Value.");

  // No properties
  node = nodes.at(2).get();
  EXPECT_TRUE(node->getNodeType() == AST::Declaration);
  properties = static_cast<DeclarationNode *>(node)->getProperties();
  EXPECT_TRUE(properties.size() == 0);

  // Property is an object
  node = nodes.at(3).get();
  EXPECT_TRUE(node->getNodeType() == AST::Declaration);
  properties = static_cast<DeclarationNode *>(node)->getProperties();
  EXPECT_TRUE(properties.size() == 2);
  property = properties.at(0);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "value");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::Declaration);
  DeclarationNode *object = static_cast<DeclarationNode *>(propertyValue);
  EXPECT_TRUE(object->getName() == "");
  EXPECT_TRUE(object->getObjectType() == "");
  std::vector<std::shared_ptr<PropertyNode>> objProperties =
      static_cast<DeclarationNode *>(object)->getProperties();
  EXPECT_TRUE(objProperties.size() == 2);
  property = objProperties.at(0);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "prop1");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::Int);
  EXPECT_TRUE(static_cast<ValueNode *>(propertyValue)->getIntValue() == 5);
  property = objProperties.at(1);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "prop2");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::String);
  EXPECT_TRUE(static_cast<ValueNode *>(propertyValue)->getStringValue() ==
              "hello");
  property = properties.at(1);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "meta");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::String);
  EXPECT_TRUE(static_cast<ValueNode *>(propertyValue)->getStringValue() ==
              "Block as Property");

  node = nodes.at(4).get();
  EXPECT_TRUE(node->getNodeType() == AST::Declaration);
  properties = static_cast<DeclarationNode *>(node)->getProperties();
  EXPECT_TRUE(properties.size() == 2);
  property = properties.at(0);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "process");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::List);
  ListNode *listnode = static_cast<ListNode *>(propertyValue);
  EXPECT_TRUE(listnode->getNodeType() == AST::List);
  EXPECT_TRUE(listnode->getChildren().size() == 1);
  StreamNode *streamNode =
      static_cast<StreamNode *>(listnode->getChildren().at(0).get());
  EXPECT_TRUE(streamNode->getChildren().size() == 2);
  EXPECT_TRUE(streamNode->getChildren().at(0)->getNodeType() == AST::Function);
  EXPECT_TRUE(streamNode->getChildren().at(1)->getNodeType() == AST::Bundle);
  property = properties.at(1);
  EXPECT_TRUE(property != nullptr && property->getChildren().size() == 1);
  EXPECT_TRUE(property->getName() == "meta");
  propertyValue = property->getChildren().at(0).get();
  EXPECT_TRUE(propertyValue->getNodeType() == AST::String);
  EXPECT_TRUE(static_cast<ValueNode *>(propertyValue)->getStringValue() ==
              "Stream property");
}

TEST(Basic, Header) {
  ASTNode tree;
  tree = ASTFunctions::parseFile(TESTS_SOURCE_DIR "basic/01_header.stride");

  EXPECT_TRUE(tree != nullptr);
  std::vector<ASTNode> nodes = tree->getChildren();
  SystemNode *node = static_cast<SystemNode *>(nodes.at(0).get());
  EXPECT_TRUE(node->getNodeType() == AST::Platform);
  EXPECT_TRUE(node->platformName() == "PufferFish");
  EXPECT_TRUE(node->majorVersion() == 1);
  EXPECT_TRUE(node->minorVersion() == 1);
  EXPECT_TRUE(node->getChildren().size() == 0);
  EXPECT_EQ(node->getLine(), 1);

  node = static_cast<SystemNode *>(nodes.at(1).get());
  EXPECT_TRUE(node->getNodeType() == AST::Platform);
  EXPECT_TRUE(node->platformName() == "Gamma");
  EXPECT_TRUE(node->majorVersion() == -1);
  EXPECT_TRUE(node->minorVersion() == -1);
  EXPECT_TRUE(node->getChildren().size() == 0);
  EXPECT_TRUE(node->getLine() == 3);

  ImportNode *importnode = static_cast<ImportNode *>(nodes.at(2).get());
  EXPECT_TRUE(importnode->getNodeType() == AST::Import);
  EXPECT_TRUE(importnode->importName() == "File");
  EXPECT_TRUE(importnode->importAlias() == "");
  EXPECT_TRUE(importnode->getLine() == 5);

  importnode = static_cast<ImportNode *>(nodes.at(3).get());
  EXPECT_TRUE(importnode->getNodeType() == AST::Import);
  EXPECT_TRUE(importnode->importName() == "File");
  EXPECT_TRUE(importnode->importAlias() == "F");
  EXPECT_TRUE(importnode->getLine() == 6);
}
