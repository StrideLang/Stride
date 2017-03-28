#ifndef DECLARATIONNODE_H
#define DECLARATIONNODE_H

#include <string>

#include "ast.h"
#include "bundlenode.h"
#include "propertynode.h"

class DeclarationNode : public AST
{
public:
    DeclarationNode(string name, string objectType, AST *propertiesList, const char *filename, int line);
    DeclarationNode(BundleNode *bundle, string objectType, AST *propertiesList, const char *filename, int line);
    ~DeclarationNode();

    string getName() const;
    BundleNode *getBundle() const;
    vector<PropertyNode *> getProperties() const;
    bool addProperty(PropertyNode *newProperty);
    AST *getPropertyValue(string propertyName);
    void setPropertyValue(string propertyName, AST *value);
    void replacePropertyValue(string propertyName, AST *newValue);

    AST *getDomain();
    void setDomainString(string domain);

    string getObjectType() const;
    AST *deepCopy();

private:
    string m_name;
    string m_objectType;
    vector<PropertyNode *> m_properties;
};

#endif // DECLARATIONNODE_H
