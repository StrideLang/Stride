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

#ifndef STRIDELIBRARY_HPP
#define STRIDELIBRARY_HPP

#include <map>
#include <vector>

#include "stride/parser/declarationnode.h"
//#include "langerror.h"

struct LibraryTree {
  std::string importName;
  std::string importAs;
  std::vector<ASTNode> nodes;
  std::vector<std::string> namespaces;
};

class StrideLibrary {
public:
  StrideLibrary();
  ~StrideLibrary();

  void initializeLibrary(std::string strideRootPath,
                         std::vector<std::string> includePaths = {});

  std::shared_ptr<DeclarationNode> findTypeInLibrary(std::string typeName);

  bool isValidBlock(DeclarationNode *block);

  std::map<std::string, std::vector<ASTNode>> getLibraryMembers();

  std::vector<ASTNode> loadImport(std::string importName, std::string importAs);

private:
  bool isValidProperty(std::shared_ptr<PropertyNode> property,
                       DeclarationNode *type);
  std::vector<DeclarationNode *> getParentTypes(DeclarationNode *type);

  void readLibrary(std::string rootDir);

  std::vector<LibraryTree>
      m_libraryTrees; // List of root and imported library trees

  std::string m_libraryPath;

  int m_majorVersion;
  int m_minorVersion;
  std::vector<std::string> m_includePaths;
};

#endif // STRIDELIBRARY_HPP
