#include "fornode.h"

ForNode::ForNode(const char *filename, int line) :
  AST(AST::For, filename, line)
{

}

AST *ForNode::deepCopy()
{
  AST* newNode = new ForNode(m_filename.data(), getLine());
  vector<AST *> children = getChildren();
  for (unsigned int i = 0; i < children.size(); i++) {
    newNode->addChild(children.at(i)->deepCopy());
  }
  return newNode;
}

