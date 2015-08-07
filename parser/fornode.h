#ifndef FORNODE_H
#define FORNODE_H

#include "ast.h"

class ForNode : public AST
{
public:
  ForNode(int line);
};

#endif // FORNODE_H
