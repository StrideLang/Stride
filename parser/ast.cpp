#include "ast.h"

AST::AST()
{
    m_token = AST::None;
}

AST::AST(Token token)
{
    m_token = token;
}

AST::~AST()
{
    for(int i = 0; i < (int) m_children.size(); i++) {
        delete m_children.at(i);
    }
//    m_children.clear();
}

void AST::addChild(AST *t) {
    m_children.push_back(t);
}

void AST::pushParent(AST *p)
{
    for(int i = 0; i < (int) m_children.size(); i++) {
        p->addChild(m_children.at(i));
    }
    m_children.clear();
    addChild(p);
}

