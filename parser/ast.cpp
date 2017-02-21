#include <cassert>

#include "ast.h"

extern AST *parse(const char* fileName);
extern std::vector<LangError> getErrors();

AST::AST()
{
    m_token = AST::None;
    m_line = -1;
    m_rate = -1;
    m_namespace = "";
}

AST::AST(Token token, const char *filename, int line)
{
    m_token = token;
    m_filename = filename;
    m_line = line;
    m_rate = -1;
    m_namespace = "";
}

AST::~AST()
{

}

void AST::addChild(AST *t) {
    m_children.push_back(t);
}

void AST::giveChildren(AST *p)
{
    for(size_t i = 0; i < m_children.size(); i++) {
        p->addChild(m_children.at(i));
    }
    m_children.clear();
}

void AST::setChildren(vector<AST *> &newChildren)
{
//    deleteChildren();
    m_children = newChildren;
}

void AST::deleteChildren()
{
    for(size_t i = 0; i < m_children.size(); i++) {
        m_children.at(i)->deleteChildren();
        delete m_children.at(i);
    }
    m_children.clear();
}

AST *AST::deepCopy()
{
    assert(0 == 1); // can't deep copy base AST
    return NULL;
}

AST *AST::parseFile(const char *fileName)
{
    return parse(fileName);
}

vector<LangError> AST::getParseErrors()
{
    return getErrors();
}

double AST::getRate() const
{
    return m_rate;
}

void AST::setRate(double rate)
{
    m_rate = rate;
}

string AST::getFilename() const
{
    return m_filename;
}

void AST::setFilename(const string &filename)
{
    m_filename = filename;
}

void AST::resolveScope(AST* scope)
{
    assert(0 == 1); // Each type should resolve its scope
}

void AST::addScope(string newScope)
{
    m_scope.push_back(newScope);
}

unsigned int AST::getScopeLevels()
{
    return m_scope.size();
}

string AST::getScopeAt(unsigned int scopeLevel)
{
    return m_scope.at(scopeLevel);
}
