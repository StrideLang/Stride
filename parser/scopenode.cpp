#include "scopenode.h"

ScopeNode::ScopeNode(std::string name, const char *filename, int line) :
    AST(AST::Scope, filename, line)
{
    m_name = name;
}

ScopeNode::~ScopeNode()
{

}
