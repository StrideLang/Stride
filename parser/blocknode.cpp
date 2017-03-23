#include <cassert>

#include "blocknode.h"
#include "scopenode.h"

BlockNode::BlockNode(string name, const char *filename, int line) :
    AST(AST::Block, filename, line)
{
    m_name = name;
}

BlockNode::BlockNode(string name, AST *scope, const char *filename, int line) :
    AST(AST::Block, filename, line)
{
    m_name = name;
    resolveScope(scope);
}

BlockNode::~BlockNode()
{

}

void BlockNode::resolveScope(AST *scope)
{
    if (scope) {
        for (unsigned int i = 0; i < scope->getChildren().size(); i++) {
            assert(scope->getChildren().at(i)->getNodeType() == AST::Scope);
            m_scope.push_back((static_cast<ScopeNode *>(scope->getChildren().at(i)))->getName());
        }
    }
}

AST *BlockNode::deepCopy()
{
    BlockNode *newNode = new BlockNode(m_name, m_filename.data(), m_line);
    newNode->setRate(m_rate);
    for (unsigned int i = 0; i < this->getScopeLevels(); i++) {
        newNode->addScope(this->getScopeAt(i));
    }
    return newNode;
}
