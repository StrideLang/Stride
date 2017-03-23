#include <cassert>

#include "importnode.h"
#include "scopenode.h"

ImportNode::ImportNode(string name, AST *scope, const char *filename, int line, string alias) :
    AST(AST::Import, filename, line)
{
    m_importName = name;
    m_importAlias = alias;
    resolveScope(scope);
}

ImportNode::ImportNode(string name, const char *filename, int line, string alias) :
    AST(AST::Import, filename, line)
{
    m_importName = name;
    m_importAlias = alias;
}

string ImportNode::importName() const
{
    return m_importName;
}

void ImportNode::setImportName(const string &importName)
{
    m_importName = importName;
}
string ImportNode::importAlias() const
{
    return m_importAlias;
}

void ImportNode::setImportAlias(const string &importAlias)
{
    m_importAlias = importAlias;
}

void ImportNode::resolveScope(AST *scope)
{
    if (scope) {
        for (unsigned int i = 0; i < scope->getChildren().size(); i++) {
            assert(scope->getChildren().at(i)->getNodeType() == AST::Scope);
            m_scope.push_back((static_cast<ScopeNode *>(scope->getChildren().at(i)))->getName());
        }
    }
}

AST *ImportNode::deepCopy()
{
    AST* newImportNode = new ImportNode(m_importName, m_filename.data(), getLine(), m_importAlias);
    for (unsigned int i = 0; i < this->getScopeLevels(); i++) {
        newImportNode->addScope(this->getScopeAt(i));
    }
    return newImportNode;
}



