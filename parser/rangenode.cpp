#include <cassert>

#include "rangenode.h"

RangeNode::RangeNode(AST *start, AST *end, const char *filename, int line):
    AST(AST::Range, filename, line)
{
	addChild(start);
	addChild(end);
}

AST *RangeNode::startIndex() const
{
    return m_children.at(0);
}

AST *RangeNode::endIndex() const
{
    return m_children.at(1);
}

AST *RangeNode::deepCopy()
{
    AST *output = new RangeNode(startIndex()->deepCopy(), endIndex()->deepCopy(),
                                m_filename.data(), m_line);
    output->setRate(m_rate);
    return output;
}

