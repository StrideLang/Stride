
#include <cassert>

#include "streamnode.h"

StreamNode::StreamNode(AST *left, AST *right) :
    AST(AST::Stream)
{
//    assert(left); assert(right);
    addChild(left);
    addChild(right);
}

StreamNode::~StreamNode()
{

}

