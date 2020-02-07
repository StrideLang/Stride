#ifndef CODEENTITIES_HPP
#define CODEENTITIES_HPP

#include <algorithm>
#include <map>
#include <string>

#include "../codegen/codevalidator.h"
#include "../parser/ast.h"
#include "../parser/declarationnode.h"
#include "../parser/functionnode.h"
#include "../parser/portpropertynode.h"

typedef enum {
  ACCESS_NONE = 0,
  ACCESS_SDR = 1 << 1,    // Single Domain Read
  ACCESS_SDW = 1 << 2,    // Single Domain Write
  ACCESS_MDR = 1 << 3,    // Multi Domain Read
  ACCESS_MDW = 1 << 4,    // Multi Domain Write
  ACCESS_SDRst = 1 << 5,  // Single domain Reset
  ACCESS_MDRst = 1 << 6   // Multi domain Reset
} SignalAccess;

enum class CodeEntityType { Declaration, Instance, Reset };

class CodeEntity {
 public:
  std::string name;
  CodeEntityType entityType;
  std::vector<std::shared_ptr<CodeEntity>> dependents;
  std::vector<std::string> includes;

  virtual std::string fullName() { return name; }
};

struct Declaration : public CodeEntity {
  Declaration() { entityType = CodeEntityType::Declaration; }
  std::string type;
  std::string code;
  std::string templateParams;
  std::string parent;
};

struct Instance : public CodeEntity {
  Instance(std::string name_, std::string prefix, int size, std::string type,
           std::vector<std::string> defaultValue, ASTNode instanceNode,
           SignalAccess access)
      : prefix(prefix),
        size(size),
        type(type),
        defaultValue(defaultValue),
        instanceNode(instanceNode),
        access(access) {
    entityType = CodeEntityType::Instance;
    name = name_;
  }

  Instance(const Instance &inst)
      : prefix(inst.prefix),
        size(inst.size),
        type(inst.type),
        defaultValue(inst.defaultValue),
        instanceNode(inst.instanceNode) {
    entityType = CodeEntityType::Instance;
    name = inst.name;
    dependents = inst.dependents;
  }

  std::string prefix;
  std::string parent;
  int size{0};
  std::string type;
  std::vector<std::string> defaultValue;
  ASTNode instanceNode;
  std::vector<std::string> constructorArgs;
  std::vector<std::string> templateArgs;
  SignalAccess access{ACCESS_NONE};

  std::string fullName() override { return prefix + name; }
};

struct Reset : public CodeEntity {
  Reset(std::shared_ptr<DeclarationNode> decl) {
    entityType = CodeEntityType::Reset;
    resetBlockDecl = decl;
  }
  std::shared_ptr<DeclarationNode> resetBlockDecl;
};

typedef struct {
  std::shared_ptr<DeclarationNode> internalDecl;
  std::vector<std::string> parentList;
  ASTNode externalConnection;
  std::string blockForPort;  // If this block is a block port, provide
                             // the port declaration name
  SignalAccess access;
} DeclarationMap;

typedef struct {
  std::shared_ptr<Instance> functionCall;
  std::shared_ptr<Declaration> declaration;
  std::vector<std::string> parentInstanceNames;
  std::map<std::string, std::vector<DeclarationMap>> blockInMap;
  std::map<std::string, std::vector<DeclarationMap>> blockOutMap;
  std::map<std::string, std::string> domainMap;
} FunctionCall;

// Code that applies to a single domain
class DomainCode {
 public:
  std::vector<std::shared_ptr<CodeEntity>> scopeEntities;
  //    std::vector<Instance> scopeInstances;

  std::string headerCode;

  std::string initCode;
  //    std::vector<std::string> scopeDeclarations;
  std::string processingCode;
  std::vector<std::string> postProcessingCode;
  std::string cleanupCode;

  std::vector<std::string> linkTargets;
  std::vector<std::string> linkDirs;
  std::vector<std::string> includeFiles;
  std::vector<std::string> includeDirs;

  std::vector<std::shared_ptr<DeclarationNode>> globalReferences;

  // Holds the token to pass to the next member
  // FIXME remove currentOutTokens and rely on "tokens" compiler property
  std::vector<std::string> currentOutTokens;

  // This holds items to process in higher scopes. e.g. in preprocess or post
  // process in the domain or global declarations.
  //    std::vector<ASTNode> m_unprocessedItems;

  // Key is internal, value is external. If second is null, there is no external
  // connection.
  std::vector<DeclarationMap> inMap;
  std::vector<DeclarationMap> outMap;

  // Maps the inner module signals to outer signal declarations for code
  // generated in this domain
  std::vector<FunctionCall> moduleCalls;

  std::map<std::shared_ptr<PortPropertyNode>, std::string> portPropertiesMap;

  void append(DomainCode newCode) {
    scopeEntities.insert(scopeEntities.end(), newCode.scopeEntities.begin(),
                         newCode.scopeEntities.end());
    headerCode += newCode.headerCode;
    initCode += newCode.initCode;
    //        for (auto decl : newCode.scopeDeclarations) {
    //            if (std::find(scopeDeclarations.begin(),
    //            scopeDeclarations.end(), decl) == scopeDeclarations.end()) {
    //                scopeDeclarations.push_back(decl);
    //            }
    //        }
    processingCode += newCode.processingCode;

    for (auto postCode : newCode.postProcessingCode) {
      if (std::find(postProcessingCode.begin(), postProcessingCode.end(),
                    postCode) == postProcessingCode.end()) {
        postProcessingCode.push_back(postCode);
      }
    }
    cleanupCode += newCode.cleanupCode;
    linkTargets.insert(linkTargets.end(), newCode.linkTargets.begin(),
                       newCode.linkTargets.end());
    linkDirs.insert(linkDirs.end(), newCode.linkDirs.begin(),
                    newCode.linkDirs.end());
    includeFiles.insert(includeFiles.end(), newCode.includeFiles.begin(),
                        newCode.includeFiles.end());
    includeDirs.insert(includeDirs.end(), newCode.includeDirs.begin(),
                       newCode.includeDirs.end());

    for (auto newReference : newCode.globalReferences) {
      bool declarationRegistered = false;
      for (auto existingRef : globalReferences) {
        // FIXME check namespaces
        if (existingRef->getName() == newReference->getName()) {
          declarationRegistered = true;
          break;
        }
      }
      if (!declarationRegistered) {
        globalReferences.push_back(newReference);
      }
    }

    for (auto &newEntry : newCode.portPropertiesMap) {
      bool propertyRegistered = false;
      std::string domainId;
      for (auto entry :
           portPropertiesMap) {  // Check if property already there.
        std::shared_ptr<PortPropertyNode> portProp = entry.first;
        if (portProp->getName() == newEntry.first->getName() &&
            portProp->getPortName() == newEntry.first->getPortName()) {
          propertyRegistered = true;
          break;
        }
      }
      if (!propertyRegistered) {
        portPropertiesMap[newEntry.first] = newEntry.second;
      }
    }
    //        m_unprocessedItems.insert(m_unprocessedItems.end(),
    //        newCode.m_unprocessedItems.begin(),
    //        newCode.m_unprocessedItems.end());
    currentOutTokens.insert(
        currentOutTokens.end(), newCode.currentOutTokens.begin(),
        newCode.currentOutTokens
            .end());  // Out tokens are appended. The expectation is that they
                      // are consumed as they are used.

    for (auto call : newCode.moduleCalls) {
      bool found = false;
      for (auto existingCall : moduleCalls) {
        if (existingCall.functionCall->fullName() ==
            call.functionCall->fullName()) {
          found = true;
          break;
        }
      }
      if (!found) {
        moduleCalls.push_back(call);
      }
    }
  }

  void prepend(DomainCode newCode) {
    scopeEntities.insert(scopeEntities.begin(), newCode.scopeEntities.begin(),
                         newCode.scopeEntities.end());
    headerCode = newCode.headerCode + headerCode;
    initCode = initCode + newCode.initCode;  // Init code is not prepended
    //        for (auto decl : newCode.scopeDeclarations) {
    //            if (std::find(scopeDeclarations.begin(),
    //            scopeDeclarations.end(), decl) == scopeDeclarations.end()) {
    //                scopeDeclarations.push_back(decl);
    //            }
    //        }
    processingCode +=
        newCode.processingCode;  // Processing code is not prepended

    for (auto postCode : newCode.postProcessingCode) {
      if (std::find(postProcessingCode.begin(), postProcessingCode.end(),
                    postCode) == postProcessingCode.end()) {
        postProcessingCode.push_back(postCode);
      }
    }
    cleanupCode =
        cleanupCode + newCode.cleanupCode;  // Cleanup code is not prepended
    linkTargets.insert(linkTargets.begin(), newCode.linkTargets.begin(),
                       newCode.linkTargets.end());
    linkDirs.insert(linkDirs.begin(), newCode.linkDirs.begin(),
                    newCode.linkDirs.end());
    includeFiles.insert(includeFiles.begin(), newCode.includeFiles.begin(),
                        newCode.includeFiles.end());
    includeDirs.insert(includeDirs.begin(), newCode.includeDirs.begin(),
                       newCode.includeDirs.end());
    for (auto newReference : newCode.globalReferences) {
      bool declarationRegistered = false;
      for (auto existingRef : globalReferences) {
        // FIXME check namespaces
        if (existingRef->getName() == newReference->getName()) {
          declarationRegistered = true;
          break;
        }
      }
      if (!declarationRegistered) {
        globalReferences.push_back(newReference);
      }
    }

    for (auto &newEntry : newCode.portPropertiesMap) {
      bool propertyRegistered = false;
      std::string domainId;
      for (auto entry :
           portPropertiesMap) {  // Check if property already there.
        std::shared_ptr<PortPropertyNode> portProp = entry.first;
        if (portProp->getName() == newEntry.first->getName() &&
            portProp->getPortName() == newEntry.first->getPortName()) {
          propertyRegistered = true;
          break;
        }
      }
      if (!propertyRegistered) {
        portPropertiesMap[newEntry.first] = newEntry.second;
      }
    }

    //        m_unprocessedItems.insert(m_unprocessedItems.begin(),
    //        newCode.m_unprocessedItems.begin(),
    //        newCode.m_unprocessedItems.end());
    currentOutTokens.insert(currentOutTokens.begin(),
                            newCode.currentOutTokens.begin(),
                            newCode.currentOutTokens.end());
    for (auto call : newCode.moduleCalls) {
      bool found = false;
      for (auto existingCall : moduleCalls) {
        if (existingCall.functionCall->fullName() ==
            call.functionCall->fullName()) {
          found = true;
          break;
        }
      }
      if (!found) {
        moduleCalls.insert(moduleCalls.begin(), call);
      }
    }
  }
};

class DomainCodeMap : public std::map<std::string, DomainCode> {
 public:
  void append(DomainCodeMap newMap) {
    for (auto domainCode : newMap) {
      append(domainCode.second, domainCode.first);
    }
  }

  void append(DomainCode code, std::string domainId) {
    if (this->find(domainId) == this->end()) {
      (*this)[domainId] = DomainCode();
    }
    (*this)[domainId].append(code);
  }

  void prepend(DomainCode code, std::string domainId) {
    if (this->find(domainId) == this->end()) {
      (*this)[domainId] = DomainCode();
    }
    (*this)[domainId].prepend(code);
  }
};

#endif  // CODEENTITIES_HPP
