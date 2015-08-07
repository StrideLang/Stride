#ifndef IMPORTNODE_H
#define IMPORTNODE_H

#include <string>

#include "ast.h"

class ImportNode : public AST
{
public:
  ImportNode(string name, int line, string alias = string());

  string importName() const;
  void setImportName(const string &importName);

  string importAlias() const;
  void setImportAlias(const string &importAlias);

private:
  string m_importName;
  string m_importAlias;
};

#endif // IMPORTNODE_H
