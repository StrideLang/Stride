#include "gtest/gtest.h"

#include "stride/codegen/astfunctions.hpp"
#include "stride/codegen/astquery.hpp"
#include "stride/codegen/stridelibrary.hpp"

TEST(Library, ImportRaw) {
  StrideLibrary library;

  // Add current path to import directories
  library.initializeLibrary("", {TESTS_SOURCE_DIR "library"});
  auto nodes = library.loadImport("Include", "Test");

  EXPECT_EQ(nodes.size(), 1);
  EXPECT_EQ(nodes[0]->getNodeType(), AST::Declaration);
  auto decl = std::static_pointer_cast<DeclarationNode>(nodes[0]);
  EXPECT_EQ(decl->getObjectType(), "module");
  EXPECT_EQ(decl->getName(), "LibTest");

  auto members = library.getLibraryMembers();

  EXPECT_EQ(members.size(), 1);
  EXPECT_TRUE(members.find("Test") != members.end());
  EXPECT_EQ(members["Test"][0], nodes[0]);
}

TEST(Library, Import) {

  auto tree =
      AST::parseFile(TESTS_SOURCE_DIR "library/01_import.stride");

  ASTFunctions::preprocess(tree);

  auto decl = ASTQuery::findDeclarationByName("LibTest", {}, tree);

  EXPECT_NE(decl, nullptr);
  EXPECT_EQ(decl->getNodeType(), AST::Declaration);
}
