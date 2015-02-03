#ifndef FUNCTIONNODE_H
#define FUNCTIONNODE_H

#include <string>

#include "ast.h"

class FunctionNode : public AST
{
public:
    typedef enum {
        BuiltIn,
        UserDefined
    } FunctionType;

    FunctionNode(string name, AST *propertiesList, FunctionType type);
    ~FunctionNode();

    string getName() const { return m_name; }

private:
    string m_name;
    FunctionType m_type;
};

#endif // FUNCTIONNODE_H
