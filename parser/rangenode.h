#ifndef RANGENODE_H
#define RANGENODE_H


#include "ast.h"

class RangeNode : public AST
{
public:
  RangeNode(AST *start, AST *end, const char *filename, int line);

  AST *startIndex() const;
  AST *endIndex() const;
  AST *deepCopy();

private:

};

#endif // RANGENODE_H
