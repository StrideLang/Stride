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

#ifndef CODEGEN_H
#define CODEGEN_H

#include "stride/parser/strideparser.h"

class StrideSystem;

typedef std::vector<std::pair<ASTNode, std::vector<ASTNode>>> ScopeStack;

class CodeValidator {
public:
  typedef enum { NO_OPTIONS = 0x00, NO_RATE_VALIDATION = 0x01 } Options;

  CodeValidator(ASTNode tree = nullptr, Options options = NO_OPTIONS);
  ~CodeValidator();

  bool isValid();
  bool platformIsValid();

  std::vector<LangError> getErrors();
  std::vector<std::string> getPlatformErrors();

  // Static functions -----------------------------

  static std::vector<StreamNode *> getStreamsAtLine(ASTNode tree, int line);

private:
  void validateTree(ASTNode tree);
  void validate();

  void validatePlatform(ASTNode node, std::vector<LangError> &errors);

  void validateBundleIndeces(ASTNode node, std::vector<LangError> &errors,
                             ScopeStack scope);
  void validateBundleSizes(ASTNode node, std::vector<LangError> &errors,
                           ScopeStack scope);
  void validateSymbolUniqueness(ScopeStack scope,
                                std::vector<LangError> &errors);
  void validateStreamSizes(ASTNode tree, std::vector<LangError> &errors,
                           ScopeStack scope);
  void validateRates(ASTNode tree);
  void validateConstraints(ASTNode tree);

  void sortErrors();

  void validateStreamInputSize(StreamNode *stream,
                               std::vector<LangError> &errors,
                               ScopeStack scope);

  void validateNodeRate(ASTNode node, ASTNode tree);

  void validateConstraints(std::shared_ptr<StreamNode> stream,
                           ScopeStack scopeStack, ASTNode tree);
  void validateFunctionConstraints(std::shared_ptr<FunctionNode> function,
                                   ScopeStack scopeStack, ASTNode tree);

  void validateConstraintStream(std::shared_ptr<StreamNode> stream,
                                std::shared_ptr<FunctionNode> function,
                                std::shared_ptr<DeclarationNode> declaration,
                                ScopeStack scopeStack, ASTNode tree);

  std::vector<ASTNode>
  resolveConstraintNode(ASTNode node, std::vector<ASTNode> previous,
                        std::shared_ptr<FunctionNode> function,
                        std::shared_ptr<DeclarationNode> declaration,
                        ScopeStack scopeStack, ASTNode tree);

  int getBlockDataSize(std::shared_ptr<DeclarationNode> declaration,
                       ScopeStack scope,
                       std::vector<LangError> *errors = nullptr);

  std::shared_ptr<StrideSystem> m_system;
  ASTNode m_tree;
  std::vector<LangError> m_errors;
  Options m_options;
};

#endif // CODEGEN_H
