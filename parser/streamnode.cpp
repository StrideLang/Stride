
#include <cassert>

#include "streamnode.h"

StreamNode::StreamNode(AST *left, AST *right, int line) :
    AST(AST::Stream, line)
{
//    assert(left); assert(right);
    addChild(left);
    addChild(right);
}

StreamNode::~StreamNode()
{

}

AST *StreamNode::deepCopy()
{
    AST * stream = new StreamNode(m_children.at(0)->deepCopy(), m_children.at(1)->deepCopy(), m_line);
    stream->setRate(m_rate);
    return stream;
}

