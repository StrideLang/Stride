#ifndef RANGENODE_H
#define RANGENODE_H


#include "ast.h"

class RangeNode : public AST
{
public:
  RangeNode(AST *start, AST *end, int line);

  AST *startIndex() const;
  AST *endIndex() const;
  AST *deepCopy();

private:
  AST *m_start;
  AST *m_end;
};

#endif // RANGENODE_H
