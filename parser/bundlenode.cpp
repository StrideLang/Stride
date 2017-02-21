#include <cassert>

#include "bundlenode.h"
#include "scopenode.h"

BundleNode::BundleNode(string name, ListNode *indexList, const char *filename, int line) :
    AST(AST::Bundle, filename, line)
{
    addChild(indexList);
    m_name = name;
}

BundleNode::BundleNode(string name, AST *scope, ListNode *indexList, const char *filename, int line) :
    AST(AST::Bundle, filename, line)
{
    addChild(indexList);
    m_name = name;
    resolveScope(scope);
}

BundleNode::~BundleNode()
{

}

string BundleNode::getName() const
{
    return m_name;
}

ListNode *BundleNode::index() const
{
    return static_cast<ListNode *>(m_children.at(0));
}

void BundleNode::resolveScope(AST *scope)
{
    if (scope)
    {
        for (unsigned int i = 0; i < scope->getChildren().size(); i++) {
            assert(scope->getChildren().at(i)->getNodeType() == AST::Scope);
            m_scope.push_back((static_cast<ScopeNode *>(scope->getChildren().at(i)))->getName());
        }
    }
}

AST *BundleNode::deepCopy()
{
    assert(getNodeType() == AST::Bundle);
    if(getNodeType() == AST::Bundle) {
        AST *newBundle = new BundleNode(m_name, static_cast<ListNode *>(index()->deepCopy()), m_filename.data(), m_line);
        newBundle->setRate(m_rate);
        for (unsigned int i = 0; i < this->getScopeLevels(); i++) {
            newBundle->addScope(this->getScopeAt(i));
        }
        return newBundle;
    }
    assert(0 == 1);
    return 0;
}
