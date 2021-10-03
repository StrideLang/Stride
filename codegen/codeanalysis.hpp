#ifndef CODEANALYSIS_HPP
#define CODEANALYSIS_HPP

#include "strideparser.h"

class CodeAnalysis {
public:
  // Domain queries
  static std::shared_ptr<DeclarationNode>
  findDomainDeclaration(std::string domainName, std::string framework,
                        ASTNode tree);

  static std::shared_ptr<DeclarationNode>
  findDomainDeclaration(std::string domainId, ASTNode tree);

  static ASTNode getNodeDomain(ASTNode node, ScopeStack scopeStack,
                               ASTNode tree);

  static std::string getNodeDomainName(ASTNode node, ScopeStack scopeStack,
                                       ASTNode tree);

  static std::string getDomainIdentifier(ASTNode domain, ScopeStack scopeStack,
                                         ASTNode tree);

  // Framework queries
  static std::string getFrameworkForDomain(std::string domainName,
                                           ASTNode tree);
};

#endif // CODEANALYSIS_HPP
