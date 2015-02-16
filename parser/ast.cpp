#include "ast.h"

AST::AST()
{
    m_token = AST::None;
    m_line = -1;
}

AST::AST(Token token, int line)
{
    m_token = token;
    m_line = line;
}

AST::~AST()
{
}

void AST::addChild(AST *t) {
    m_children.push_back(t);
}

void AST::giveChildren(AST *p)
{
    for(int i = 0; i < (int) m_children.size(); i++) {
        p->addChild(m_children.at(i));
    }
    m_children.clear();
}

void AST::deleteChildren()
{
    for(int i = 0; i < (int) m_children.size(); i++) {
        m_children.at(i)->deleteChildren();
        delete m_children.at(i);
    }
    m_children.clear();
}

