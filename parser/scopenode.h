#ifndef SCOPENODE_H
#define SCOPENODE_H

#include <string>

#include "ast.h"

class ScopeNode : public AST
{
public:

    ScopeNode(string name, const char *filename, int line);

    ScopeNode(string name, string namespace_, const char *filename, int line);
    ~ScopeNode();

    string getName() const {return m_name;}

    AST *deepCopy();

private:
    std::string m_name;
    std::string m_scope;
};


#endif // SCOPENODE_H
