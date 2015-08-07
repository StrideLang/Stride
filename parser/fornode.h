#ifndef FORNODE_H
#define FORNODE_H

#include "ast.h"

class ForNode : public AST
{
public:
  ForNode(int line);

  AST *deepCopy();
};

#endif // FORNODE_H
