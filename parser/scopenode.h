#ifndef SCOPENODE_H
#define SCOPENODE_H

#include <string>

#include "ast.h"

class ScopeNode : public AST
{
public:
    ScopeNode(string name, const char *filename, int line);
    ~ScopeNode();

    string getName() const {return m_name;}

private:
    string m_name;
};

#endif // SCOPENODE_H
