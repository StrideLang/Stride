#ifndef CODEQUERY_HPP
#define CODEQUERY_HPP

#include "strideparser.h"

#include <string>

class CodeQuery {
public:
  // Resolve types
  static std::string resolveBundleDataType(BundleNode *bundle,
                                           ScopeStack scopeStack, ASTNode tree);
  static std::string resolveBlockDataType(BlockNode *name,
                                          ScopeStack scopeStack, ASTNode tree);
  static std::string resolveNodeOutDataType(ASTNode node, ScopeStack scopeStack,
                                            ASTNode tree);
  static std::string resolveListDataType(ListNode *listnode,
                                         ScopeStack scopeStack, ASTNode tree);
  static std::string resolveExpressionDataType(ExpressionNode *exprnode,
                                               ScopeStack scopeStack,
                                               ASTNode tree);
  static std::string resolveRangeDataType(RangeNode *rangenode,
                                          ScopeStack scopeStack, ASTNode tree);
  static std::string resolvePortPropertyDataType(PortPropertyNode *portproperty,
                                                 ScopeStack scopeStack,
                                                 ASTNode tree);
};

#endif // CODEQUERY_HPP
