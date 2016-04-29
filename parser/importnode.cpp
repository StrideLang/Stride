#include "importnode.h"

ImportNode::ImportNode(string name, const char *filename, int line, string alias) :
  AST(AST::Import, filename, line)
{
  m_importName = name;
  m_importAlias = alias;
}

string ImportNode::importName() const
{
  return m_importName;
}

void ImportNode::setImportName(const string &importName)
{
  m_importName = importName;
}
string ImportNode::importAlias() const
{
  return m_importAlias;
}

void ImportNode::setImportAlias(const string &importAlias)
{
  m_importAlias = importAlias;
}

AST *ImportNode::deepCopy()
{
  AST* newNode = new ImportNode(m_importName, m_filename.data(), getLine(), m_importAlias);
//  vector<AST *> children = getChildren();
//  newNode->setChildren(children);
  return newNode;
}



