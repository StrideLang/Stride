#ifndef FUNCTIONNODE_H
#define FUNCTIONNODE_H

#include <string>

#include "ast.h"
#include "propertynode.h"

class FunctionNode : public AST
{
public:
    typedef enum {
        BuiltIn,
        UserDefined
    } FunctionType;

    FunctionNode(string name, AST *propertiesList, FunctionType type, const char *filename, int line);
    FunctionNode(string name, AST *scope, AST *propertiesList, FunctionType type, const char *filename, int line);
    ~FunctionNode();

    void addChild(AST *t);
    void setChildren(vector<AST *> &newChildren);
    void deleteChildren();

    string getName() const { return m_name; }
    vector<PropertyNode *> getProperties() const;

    void addProperty(PropertyNode *newProperty);
    AST *getPropertyValue(string propertyName);

    AST *getDomain();
    void setDomain(string domain);

    void resolveScope(AST* scope);

    AST *deepCopy();

private:
    string m_name;
    FunctionType m_type;
    vector<PropertyNode *> m_properties;
};

#endif // FUNCTIONNODE_H
