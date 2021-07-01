/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <algorithm>
#include <cassert>

#include "ast.h"
#include "blocknode.h"
#include "declarationnode.h"
#include "listnode.h"
#include "propertynode.h"
#include "valuenode.h"

using namespace std;

extern AST *parse(const char *fileName, const char *sourceFilename);
extern std::vector<LangError> getErrors();

AST::AST() {
  m_token = AST::None;
  m_line = -1;
}

AST::AST(Token token, const char *filename, int line, vector<string> scope) {
  m_token = token;
  m_filename.append(filename);
  m_line = line;
  m_scope = scope;
}

AST::~AST() {}

void AST::addChild(ASTNode t) { m_children.push_back(t); }

void AST::setChildren(vector<ASTNode> &newChildren) {
  //    deleteChildren();
  m_children = newChildren;
}

ASTNode AST::deepCopy() {
  ASTNode newNode =
      std::make_shared<AST>(AST::None, m_filename.data(), m_line, m_scope);
  for (unsigned int i = 0; i < m_children.size(); i++) {
    newNode->addChild(m_children.at(i)->deepCopy());
  }
  //    if (this->m_CompilerProperties) {
  //        newNode->m_CompilerProperties =
  //        std::static_pointer_cast<ListNode>(this->m_CompilerProperties->deepCopy());
  //    } else {
  //        newNode->m_CompilerProperties = nullptr;
  //    }
  return newNode;
}

ASTNode AST::parseFile(const char *fileName, const char *sourceFilename) {
  return std::shared_ptr<AST>(parse(fileName, sourceFilename));
}

vector<LangError> AST::getParseErrors() { return getErrors(); }

string AST::getFilename() const { return m_filename; }

void AST::setFilename(const string &filename) { m_filename = filename; }

void AST::resolveScope(ASTNode scope) {
  (void)scope;    // To remove warning
  assert(0 == 1); // Each type should resolve its scope
}

void AST::addScope(string newScope) { m_scope.push_back(newScope); }

void AST::setRootScope(string scopeName) {
  if (scopeName != "") {
    m_scope.insert(m_scope.begin(), scopeName);
  }
}

size_t AST::getScopeLevels() { return m_scope.size(); }

string AST::getScopeAt(unsigned int scopeLevel) {
  return m_scope.at(scopeLevel);
}

vector<string> AST::getNamespaceList() { return m_scope; }

void AST::setNamespaceList(vector<string> list) { m_scope = list; }

void AST::setCompilerProperty(string propertyName, ASTNode value) {
  if (!m_CompilerProperties) {
    m_CompilerProperties = make_shared<ListNode>(__FILE__, __LINE__);
  }
  for (auto child : m_CompilerProperties->getChildren()) {
    if (child->getNodeType() == AST::Property) {
      std::shared_ptr<PropertyNode> prop =
          static_pointer_cast<PropertyNode>(child);
      if (prop->getName() ==
          propertyName) { // if property already exists replace
        prop->replaceValue(value);
        return;
      }
    }
  }
  std::shared_ptr<PropertyNode> newProp =
      std::make_shared<PropertyNode>(propertyName, value, __FILE__, __LINE__);
  m_CompilerProperties->addChild(newProp);
}

ASTNode AST::getCompilerProperty(string propertyName) {
  if (!m_CompilerProperties) {
    return nullptr;
  }
  for (auto child : m_CompilerProperties->getChildren()) {
    if (child->getNodeType() == AST::Property) {
      std::shared_ptr<PropertyNode> prop =
          static_pointer_cast<PropertyNode>(child);
      if (prop->getName() ==
          propertyName) { // if property already exists replace
        return prop->getValue();
      }
    }
  }
  return nullptr;
}

void AST::appendToPropertyValue(string propertyName, ASTNode value) {
  if (!m_CompilerProperties) {
    m_CompilerProperties = make_shared<ListNode>(__FILE__, __LINE__);
  }
  for (auto child : m_CompilerProperties->getChildren()) {
    if (child->getNodeType() == AST::Property) {
      std::shared_ptr<PropertyNode> prop =
          static_pointer_cast<PropertyNode>(child);
      if (prop->getName() ==
          propertyName) { // if property already exists replace
        if (prop->getValue()->getNodeType() == AST::List) {
          auto list = std::static_pointer_cast<ListNode>(prop->getValue());
          list->addChild(value);
          return;
        } else {
          assert(0 == 1);
        }
      }
    }
  }
  auto newProperty = std::make_shared<PropertyNode>(
      propertyName, std::make_shared<ListNode>(value, __FILE__, __LINE__),
      __FILE__, __LINE__);
  m_CompilerProperties->addChild(newProperty);
}

string AST::toText(ASTNode node, int indentOffset, int indentSize) {
  std::string outText;
  std::string indentBase = "";

  for (auto i = 0; i < indentOffset; i++) {
    indentBase += " ";
  }
  if (node->getNodeType() == AST::Declaration) {
    auto decl = std::static_pointer_cast<DeclarationNode>(node);
    outText += indentBase;
    if (decl->getNamespaceList().size() > 0) {
      // FIXME namespace
    }
    outText += decl->getObjectType() + " " + decl->getName() + "{\n";
    for (auto prop : decl->getProperties()) {
      outText += AST::toText(prop, indentOffset + indentSize);
    }
    outText += indentBase + "}\n";
  } else if (node->getNodeType() == AST::Block) {
    auto block = std::static_pointer_cast<BlockNode>(node);
    if (block->getNamespaceList().size() > 0) {
      //      outText +=
      // FIXME namespace
    }
    outText += block->getName(); //+"\n";
  } else if (node->getNodeType() == AST::Property) {
    auto pp = std::static_pointer_cast<PropertyNode>(node);
    outText += indentBase + pp->getName() + ": ";
    outText += AST::toText(pp->getValue(), indentOffset + indentSize);
    outText += "\n";
  } else if (node->getNodeType() == AST::List) {
    auto list = std::static_pointer_cast<ListNode>(node);
    outText += "[ ";
    int currentColumn = indentOffset + indentSize;
    for (auto elem : list->getChildren()) {
      auto newText = AST::toText(elem, currentColumn);
      outText += newText + ", ";
      currentColumn += newText.size();
      if (currentColumn > 80 ||
          (std::find(newText.begin(), newText.end(), '\n') != newText.end())) {
        outText += "\n";
        currentColumn = indentOffset + indentSize;
      }
    }
    if (list->getChildren().size() > 0) {
      if (outText.back() == '\n') {
        outText.resize(outText.size() - 3);
      } else {
        outText.resize(outText.size() - 2);
      }
    }
    outText += " ]";

  } else if (node->getNodeType() == AST::Int) {
    outText +=
        std::to_string(static_pointer_cast<ValueNode>(node)->getIntValue());
  } else if (node->getNodeType() == AST::Real) {
    outText +=
        std::to_string(static_pointer_cast<ValueNode>(node)->getRealValue());
  } else if (node->getNodeType() == AST::String) {
    outText +=
        "\"" + static_pointer_cast<ValueNode>(node)->getStringValue() + "\"";
  } else if (node->getNodeType() == AST::Switch) {
    outText +=
        (std::static_pointer_cast<ValueNode>(node)->getSwitchValue() ? "on "
                                                                     : "off ");
  } else if (node->getNodeType() == AST::None) {
    // Root tree
    for (auto child : node->getChildren()) {
      outText += AST::toText(child);
    }
  }
  return outText;
}
