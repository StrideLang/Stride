#include "scopenode.h"

ScopeNode::ScopeNode(string name, const char *filename, int line) :
    AST(AST::Scope, filename, line)
{
    m_name = name;
}

ScopeNode::~ScopeNode()
{

}
