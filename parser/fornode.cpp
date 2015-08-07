#include "fornode.h"

ForNode::ForNode(int line) :
  AST(AST::For, line)
{

}

AST *ForNode::deepCopy()
{
  AST* newNode = new ForNode(getLine());
  vector<AST *> children = getChildren();
  for (unsigned int i = 0; i < children.size(); i++) {
    newNode->addChild(children.at(i)->deepCopy());
  }
  return newNode;
}

