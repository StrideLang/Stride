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

    FunctionNode(string name, AST *propertiesList, FunctionType type, const char *filename, int line, string namespace_ = "");
    ~FunctionNode();

    virtual void addChild(AST *t);
    virtual void setChildren(vector<AST *> &newChildren);
    virtual void deleteChildren();

    string getName() const { return m_name; }
    string getNamespace() const { return m_namespace; }
    vector<PropertyNode *> getProperties() const;

    void addProperty(PropertyNode *newProperty);
    AST *getPropertyValue(string propertyName);

    AST *getDomain();
    void setDomain(string domain);

    AST *deepCopy();

private:
    string m_name;
    string m_namespace;
    FunctionType m_type;
    vector<PropertyNode *> m_properties;
};

#endif // FUNCTIONNODE_H
