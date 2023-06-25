#ifndef CODEENTITIES_HPP
#define CODEENTITIES_HPP

#include <algorithm>
#include <map>
#include <string>

//#include "../codegen/codevalidator.h"
#include "stride/parser/ast.h"
#include "stride/parser/declarationnode.h"
//#include "stride/parser/functionnode.h"
#include "stride/parser/portpropertynode.h"

typedef enum {
  ACCESS_NONE = 0,
  ACCESS_SDR = 1 << 1,   // Single Domain Read
  ACCESS_SDW = 1 << 2,   // Single Domain Write
  ACCESS_MDR = 1 << 3,   // Multi Domain Read
  ACCESS_MDW = 1 << 4,   // Multi Domain Write
  ACCESS_SDRst = 1 << 5, // Single domain Reset
  ACCESS_MDRst = 1 << 6, // Multi domain Reset

  ACCESS_CROSSES_FRAMEWORK = 1 << 16
} DomainAccess;

struct SignalAccess {
  DomainAccess access = ACCESS_NONE;
  std::vector<std::string> readDomains;
  std::vector<std::string> readFrameworks;
  std::vector<std::string> writeDomains;
  std::vector<std::string> writeFrameworks;
};

enum class CodeEntityType { Declaration, Instance, Reset };

class CodeEntity {
public:
  std::string name;
  CodeEntityType entityType;
  std::vector<std::shared_ptr<CodeEntity>> dependents;
  std::vector<std::string> requiredIncludes;

  virtual std::string fullName() { return name; }
};

struct Declaration : public CodeEntity {
  Declaration() { entityType = CodeEntityType::Declaration; }
  std::string type;
  std::string code;
  std::string templateParams;
  std::string parent;
  std::vector<std::shared_ptr<Declaration>> dependsOn;
};

struct Instance : public CodeEntity {
  Instance(std::string name_, std::string prefix, int size, std::string type,
           std::vector<std::string> defaultValue, ASTNode instanceNode,
           SignalAccess access = SignalAccess())
      : prefix(prefix), size(size), type(type), defaultValue(defaultValue),
        instanceNode(instanceNode), access(access) {
    entityType = CodeEntityType::Instance;
    name = name_;
  }

  Instance(const Instance &inst)
      : prefix(inst.prefix), parent(inst.parent), size(inst.size),
        type(inst.type), defaultValue(inst.defaultValue),
        instanceNode(inst.instanceNode) {
    entityType = CodeEntityType::Instance;
    name = inst.name;
    dependents = inst.dependents;
    constructorArgs = inst.constructorArgs;
    templateArgs = inst.templateArgs;
    access = inst.access;
  }

  std::string prefix;
  std::string parent;
  int size{0};
  std::string type;
  std::vector<std::string> defaultValue;
  ASTNode instanceNode;
  std::vector<std::string> constructorArgs;
  std::vector<std::string> templateArgs;
  SignalAccess access;

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
  ASTNode externalConnection;
  std::vector<std::string> parentList;
  SignalAccess access;
  std::shared_ptr<PortPropertyNode> parentScopeProperty;
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
  std::vector<std::shared_ptr<DeclarationNode>> globalReferences;

  // Holds the code generated token to pass between code generation passes
  std::vector<std::string> currentOutTokens;

  // Code
  std::string initCode;
  std::vector<std::string> includeFiles;
  std::string processingCode;
  std::vector<std::string> postProcessingCode;
  std::string cleanupCode;

  // Compilation directives
  std::vector<std::string> linkTargets;
  std::vector<std::string> linkDirs;
  std::vector<std::string> includeDirs;

  std::vector<DeclarationMap> inMap;
  std::vector<DeclarationMap> outMap;

  // Maps the inner module signals to outer signal declarations for code
  // generated in this domain
  std::vector<FunctionCall> moduleCalls;

  std::map<std::shared_ptr<PortPropertyNode>, std::string> portPropertiesMap;

  void append(DomainCode newCode) {
    scopeEntities.insert(scopeEntities.end(), newCode.scopeEntities.begin(),
                         newCode.scopeEntities.end());
    initCode += newCode.initCode;
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
      for (auto entry : portPropertiesMap) { // Check if property already there.
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
    currentOutTokens.insert(
        currentOutTokens.end(), newCode.currentOutTokens.begin(),
        newCode.currentOutTokens
            .end()); // Out tokens are appended. The expectation is that they
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
    initCode = initCode + newCode.initCode; // Init code is not prepended
    processingCode +=
        newCode.processingCode; // Processing code is not prepended

    for (auto postCode : newCode.postProcessingCode) {
      if (std::find(postProcessingCode.begin(), postProcessingCode.end(),
                    postCode) == postProcessingCode.end()) {
        postProcessingCode.push_back(postCode);
      }
    }
    cleanupCode =
        cleanupCode + newCode.cleanupCode; // Cleanup code is not prepended
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
      for (auto entry : portPropertiesMap) { // Check if property already there.
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

#endif // CODEENTITIES_HPP
