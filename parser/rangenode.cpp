#include <cassert>

#include "rangenode.h"

RangeNode::RangeNode(AST *start, AST *end, int line):
    AST(AST::Range, line)
{
    m_start = start;
    m_end = end;
}

AST *RangeNode::startIndex() const
{
    return m_start;
}

AST *RangeNode::endIndex() const
{
    return m_end;
}

AST *RangeNode::deepCopy()
{
    AST *output = new RangeNode(m_start->deepCopy(), m_end->deepCopy(), m_line);
    output->setRate(m_rate);
    return output;
}

