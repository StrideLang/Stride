
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

