#include "ast.h"

AST::AST()
{
    m_token = NONE;
}

AST::AST(Token token)
{
    m_token = token;
}

AST::AST(int tokenType)
{
    m_token = (Token) tokenType;
}

AST::~AST()
{

}

void AST::addChild(AST t) {
    m_children.push_back(t);
}

