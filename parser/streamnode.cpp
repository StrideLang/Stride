#include <cassert>

#include "streamnode.h"

StreamNode::StreamNode(AST *left, AST *right, const char *filename, int line) :
    AST(AST::Stream, filename, line)
{
//    assert(left); assert(right);
    assert(left->getNodeType() != AST:: Stream); // This is not allowed
    addChild(left);
    addChild(right);
}

StreamNode::~StreamNode()
{

}

void StreamNode::setLeft(AST *newLeft)
{
    AST *oldLeft = m_children.at(0);
    oldLeft->deleteChildren();
    delete oldLeft;
    m_children.at(0) = newLeft;
}

void StreamNode::setRight(AST *newRight)
{
    AST *oldRight = m_children.at(1);
    oldRight->deleteChildren();
    delete oldRight;
    m_children.at(1) = newRight;
}

AST *StreamNode::deepCopy()
{
    AST * stream = new StreamNode(m_children.at(0)->deepCopy(), m_children.at(1)->deepCopy(), m_filename.data(), m_line);
    stream->setRate(m_rate);
    return stream;
}

